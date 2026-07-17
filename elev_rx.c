/***************************************************************************//**
 * @file elev_rx.c
 * @brief USART0 VCOM init + character RX ISR (ping-pong buffers)
 ******************************************************************************/
#include "elev_rx.h"
#include "elev_tx.h"
#include "elev_gpio.h"

#include "em_usart.h"
#include "em_device.h"

#include "sl_gpio.h"
#include "sl_clock_manager.h"
#include "sl_board_control.h"
#include "sl_core.h"

#include "FreeRTOS.h"
#include "task.h"

typedef struct {
  char buf[ELEV_CMD_BUF_LEN];
  uint16_t len;
  volatile bool complete;
} elev_cmd_slot_t;

static elev_cmd_slot_t s_slot[2];
static volatile uint8_t s_fill;      /* ISR writes here */
static volatile uint8_t s_ready;     /* index with complete==true for parser */
static volatile bool s_overflow;
static volatile bool s_saw_cr;
static TaskHandle_t s_parser;

void elev_uart_hw_init(void)
{
  (void)sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  (void)sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);

  /* Enable VCOM path to board controller */
  (void)sl_board_enable_vcom();

  sl_gpio_t tx = { .port = SL_GPIO_PORT_A, .pin = 8 };
  sl_gpio_t rx = { .port = SL_GPIO_PORT_A, .pin = 9 };
  sl_gpio_set_pin_mode(&tx, SL_GPIO_MODE_PUSH_PULL, 1);
  sl_gpio_set_pin_mode(&rx, SL_GPIO_MODE_INPUT_PULL, 1);

  GPIO->USARTROUTE[0].ROUTEEN = 0;
  GPIO->USARTROUTE[0].TXROUTE = (SL_GPIO_PORT_A << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                | (8 << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (SL_GPIO_PORT_A << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                | (9 << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN | GPIO_USART_ROUTEEN_RXPEN;

  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
  init.enable = usartEnable;
  init.baudrate = 115200;
  USART_InitAsync(USART0, &init);
}

void elev_rx_init(TaskHandle_t parser_task)
{
  s_parser = parser_task;
  s_fill = 0;
  s_ready = 0;
  s_overflow = false;
  s_saw_cr = false;
  s_slot[0].len = 0;
  s_slot[0].complete = false;
  s_slot[1].len = 0;
  s_slot[1].complete = false;
}

void elev_rx_enable(void)
{
  USART_IntClear(USART0, USART_IF_RXDATAV);
  USART_IntEnable(USART0, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  NVIC_EnableIRQ(USART0_RX_IRQn);
}

uint16_t elev_rx_take_command(char *out, uint16_t out_max, bool *overflow)
{
  uint16_t n = 0;
  CORE_DECLARE_IRQ_STATE;

  if (overflow != NULL) {
    *overflow = false;
  }

  CORE_ENTER_ATOMIC();
  if (s_overflow) {
    if (overflow != NULL) {
      *overflow = true;
    }
    s_overflow = false;
  }

  uint8_t idx = s_ready;
  if (!s_slot[idx].complete) {
    /* Try the other slot if ready advanced differently */
    idx = (uint8_t)(1U - s_fill);
    if (!s_slot[idx].complete) {
      CORE_EXIT_ATOMIC();
      return 0;
    }
  }

  n = s_slot[idx].len;
  if (n >= out_max) {
    n = (uint16_t)(out_max - 1U);
  }
  for (uint16_t i = 0; i < n; i++) {
    out[i] = s_slot[idx].buf[i];
  }
  out[n] = '\0';
  s_slot[idx].len = 0;
  s_slot[idx].complete = false;
  CORE_EXIT_ATOMIC();
  return n;
}

void USART0_RX_IRQHandler(void)
{
  BaseType_t woken = pdFALSE;

  elev_gpio_rx_isr_pulse();

  if ((USART_IntGet(USART0) & USART_IF_RXDATAV) == 0U) {
    return;
  }

  uint8_t c = (uint8_t)USART_RxDataGet(USART0);
  USART_IntClear(USART0, USART_IF_RXDATAV);

  /* \r\n → single EOL */
  if (c == (uint8_t)'\n' && s_saw_cr) {
    s_saw_cr = false;
    return;
  }
  s_saw_cr = (c == (uint8_t)'\r');

  if (c == (uint8_t)'\r' || c == (uint8_t)'\n') {
    elev_cmd_slot_t *slot = &s_slot[s_fill];
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    slot->complete = true;
    s_ready = s_fill;
    s_fill = (uint8_t)(1U - s_fill);
    s_slot[s_fill].len = 0;
    s_slot[s_fill].complete = false;
    CORE_EXIT_ATOMIC();

    elev_gpio_cmd_complete_pulse();
    /* Echo CRLF for terminal */
    (void)elev_tx_enqueue_echo_from_isr((uint8_t)'\r', &woken);
    (void)elev_tx_enqueue_echo_from_isr((uint8_t)'\n', &woken);

    if (s_parser != NULL) {
      vTaskNotifyGiveFromISR(s_parser, &woken);
    }
    portYIELD_FROM_ISR(woken);
    return;
  }

  /* Echo printable / backspace optional: echo all non-EOL */
  if (c >= 0x20U && c <= 0x7EU) {
    (void)elev_tx_enqueue_echo_from_isr(c, &woken);
  }

  elev_cmd_slot_t *slot = &s_slot[s_fill];
  if (slot->complete) {
    /* Parser still owns other buffer and this one somehow complete — overflow */
    s_overflow = true;
    portYIELD_FROM_ISR(woken);
    return;
  }
  if (slot->len >= (ELEV_CMD_BUF_LEN - 1U)) {
    s_overflow = true;
    portYIELD_FROM_ISR(woken);
    return;
  }
  slot->buf[slot->len++] = (char)c;
  portYIELD_FROM_ISR(woken);
}
