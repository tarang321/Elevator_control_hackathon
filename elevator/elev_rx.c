/***************************************************************************//**
 * @file elev_rx.c
 * @brief USART0 VCOM bring-up + character-by-character RX ISR
 *******************************************************************************
 * VCOM: PA08 TX / PA09 RX, 115200 8N1. Ping-pong buffers protect ISR vs parser.
 * Optional: backspace, last-command recall (up-arrow ESC [ A).
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

/*******************************************************************************
 ********************************   TYPES   ************************************
 ******************************************************************************/

typedef struct {
  char buf[ELEV_CMD_BUF_LEN];
  uint16_t len;
  volatile bool complete;
} elev_cmd_slot_t;

typedef enum {
  ESC_IDLE = 0,
  ESC_GOT_ESC,
  ESC_GOT_BRACKET
} esc_state_t;

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static elev_cmd_slot_t s_slot[2];
static volatile uint8_t s_fill;       /**< Slot ISR appends into */
static volatile uint8_t s_ready;      /**< Slot marked complete for parser */
static volatile bool s_overflow;
static volatile bool s_saw_cr;
static TaskHandle_t s_parser;

static char s_last[ELEV_CMD_BUF_LEN];
static uint16_t s_last_len;
static esc_state_t s_esc;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void save_last_from_slot(const elev_cmd_slot_t *slot);
static void echo_bs(BaseType_t *woken);
static void restore_last_cmd(BaseType_t *woken);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void save_last_from_slot(const elev_cmd_slot_t *slot)
{
  uint16_t n = slot->len;
  if (n >= ELEV_CMD_BUF_LEN) {
    n = (uint16_t)(ELEV_CMD_BUF_LEN - 1U);
  }
  for (uint16_t i = 0; i < n; i++) {
    s_last[i] = slot->buf[i];
  }
  s_last[n] = '\0';
  s_last_len = n;
}

static void echo_bs(BaseType_t *woken)
{
  (void)elev_tx_enqueue_echo_from_isr((uint8_t)'\b', woken);
  (void)elev_tx_enqueue_echo_from_isr((uint8_t)' ', woken);
  (void)elev_tx_enqueue_echo_from_isr((uint8_t)'\b', woken);
}

static void restore_last_cmd(BaseType_t *woken)
{
  elev_cmd_slot_t *slot = &s_slot[s_fill];
  uint16_t i;

  if (s_last_len == 0U) {
    return;
  }
  if (slot->complete) {
    return;
  }

  while (slot->len > 0U) {
    slot->len--;
    echo_bs(woken);
  }

  for (i = 0; i < s_last_len; i++) {
    slot->buf[i] = s_last[i];
    (void)elev_tx_enqueue_echo_from_isr((uint8_t)s_last[i], woken);
  }
  slot->len = s_last_len;
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void elev_uart_hw_init(void)
{
  sl_gpio_t tx = { .port = SL_GPIO_PORT_A, .pin = 8 };
  sl_gpio_t rx = { .port = SL_GPIO_PORT_A, .pin = 9 };
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;

  (void)sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  (void)sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);
  (void)sl_board_enable_vcom();

  sl_gpio_set_pin_mode(&tx, SL_GPIO_MODE_PUSH_PULL, 1);
  sl_gpio_set_pin_mode(&rx, SL_GPIO_MODE_INPUT_PULL, 1);

  GPIO->USARTROUTE[0].ROUTEEN = 0;
  GPIO->USARTROUTE[0].TXROUTE = (SL_GPIO_PORT_A << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                | (8 << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (SL_GPIO_PORT_A << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                | (9 << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN | GPIO_USART_ROUTEEN_RXPEN;

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
  s_esc = ESC_IDLE;
  s_last_len = 0;
  s_last[0] = '\0';
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
  uint8_t idx;
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

  idx = s_ready;
  if (!s_slot[idx].complete) {
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

/***************************************************************************//**
 * @brief USART0 RX IRQ — collect char, echo-enqueue, EOL → notify parser.
 ******************************************************************************/
void USART0_RX_IRQHandler(void)
{
  BaseType_t woken = pdFALSE;
  elev_cmd_slot_t *slot;
  uint8_t c;

  elev_gpio_rx_isr_pulse();

  if ((USART_IntGet(USART0) & USART_IF_RXDATAV) == 0U) {
    return;
  }

  c = (uint8_t)USART_RxDataGet(USART0);
  USART_IntClear(USART0, USART_IF_RXDATAV);

  /* ESC [ A — up-arrow recall last command */
  if (s_esc == ESC_GOT_ESC) {
    s_esc = (c == (uint8_t)'[') ? ESC_GOT_BRACKET : ESC_IDLE;
    portYIELD_FROM_ISR(woken);
    return;
  }
  if (s_esc == ESC_GOT_BRACKET) {
    s_esc = ESC_IDLE;
    if (c == (uint8_t)'A') {
      restore_last_cmd(&woken);
    }
    portYIELD_FROM_ISR(woken);
    return;
  }
  if (c == 0x1BU) {
    s_esc = ESC_GOT_ESC;
    portYIELD_FROM_ISR(woken);
    return;
  }
  s_esc = ESC_IDLE;

  /* Treat \r\n as a single end-of-line. */
  if ((c == (uint8_t)'\n') && s_saw_cr) {
    s_saw_cr = false;
    return;
  }
  s_saw_cr = (c == (uint8_t)'\r');

  if ((c == (uint8_t)'\r') || (c == (uint8_t)'\n')) {
    CORE_DECLARE_IRQ_STATE;

    slot = &s_slot[s_fill];
    save_last_from_slot(slot);

    CORE_ENTER_ATOMIC();
    slot->complete = true;
    s_ready = s_fill;
    s_fill = (uint8_t)(1U - s_fill);
    s_slot[s_fill].len = 0;
    s_slot[s_fill].complete = false;
    CORE_EXIT_ATOMIC();

    elev_gpio_cmd_complete_pulse();
    (void)elev_tx_enqueue_echo_from_isr((uint8_t)'\r', &woken);
    (void)elev_tx_enqueue_echo_from_isr((uint8_t)'\n', &woken);

    if (s_parser != NULL) {
      vTaskNotifyGiveFromISR(s_parser, &woken);
    }
    portYIELD_FROM_ISR(woken);
    return;
  }

  /* Backspace / DEL */
  if ((c == (uint8_t)'\b') || (c == 0x7FU)) {
    slot = &s_slot[s_fill];
    if (!slot->complete && (slot->len > 0U)) {
      slot->len--;
      echo_bs(&woken);
    }
    portYIELD_FROM_ISR(woken);
    return;
  }

  if ((c >= 0x20U) && (c <= 0x7EU)) {
    (void)elev_tx_enqueue_echo_from_isr(c, &woken);
  } else {
    portYIELD_FROM_ISR(woken);
    return;
  }

  slot = &s_slot[s_fill];
  if (slot->complete) {
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
