/***************************************************************************//**
 * @file elev_tx.c
 * @brief TX task: serializes echo + responses over USART0 LDMA
 ******************************************************************************/
#include "elev_tx.h"
#include "elev_gpio.h"

#include <string.h>

#include "em_usart.h"
#include "em_device.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sl_dma_manager.h"
#include "sl_dma_channel.h"
#include "sl_device_dma.h"
#include "sl_status.h"

#define ELEV_TX_TASK_STACK   256
#define ELEV_TX_TASK_PRIO    (tskIDLE_PRIORITY + 3)

static QueueHandle_t s_tx_q;
static TaskHandle_t s_tx_task;
static sl_dma_channel_handle_t s_dma_tx;
static uint8_t s_dma_ch;
static volatile bool s_dma_busy;
static elev_tx_msg_t s_inflight;

static void dma_cb(sl_dma_channel_handle_t *handle,
                   void *user_data,
                   bool error,
                   bool aborted)
{
  BaseType_t woken = pdFALSE;
  (void)handle;
  (void)user_data;
  (void)error;
  (void)aborted;

  elev_gpio_dma_active_set(false);
  elev_gpio_dma_complete_pulse();
  s_dma_busy = false;

  if (s_tx_task != NULL) {
    vTaskNotifyGiveFromISR(s_tx_task, &woken);
    portYIELD_FROM_ISR(woken);
  }
}

static void start_dma(const elev_tx_msg_t *msg)
{
  s_inflight = *msg;
  s_dma_busy = true;
  elev_gpio_dma_active_set(true);

  sl_status_t st = sl_dma_channel_submit_transfer_m2p(
    &s_dma_tx,
    (void *)s_inflight.data,
    (void *)&USART0->TXDATA,
    (size_t)s_inflight.len,
    SL_DMA_CTRL_SIZE_BYTE,
    NULL);

  if (st != SL_STATUS_OK) {
    elev_gpio_dma_active_set(false);
    s_dma_busy = false;
  }
}

static void tx_task(void *arg)
{
  (void)arg;
  elev_tx_msg_t msg;

  for (;;) {
    if (xQueueReceive(s_tx_q, &msg, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    if (msg.len == 0U || msg.len > ELEV_TX_MSG_MAX) {
      continue;
    }

    start_dma(&msg);

    /* Wait for DMA done (or immediate fail cleared busy). */
    while (s_dma_busy) {
      (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
  }
}

void elev_tx_init(void)
{
  s_tx_q = xQueueCreate(ELEV_TX_QUEUE_LEN, sizeof(elev_tx_msg_t));
  configASSERT(s_tx_q != NULL);

  sl_status_t st = sl_dma_manager_allocate_channel(NULL, &s_dma_ch);
  configASSERT(st == SL_STATUS_OK);

  st = sl_dma_channel_init(&s_dma_tx, NULL, s_dma_ch, dma_cb, NULL);
  configASSERT(st == SL_STATUS_OK);

  st = sl_dma_channel_set_peripheral_signal(&s_dma_tx, SL_DMA_SIGNAL_USART0_TXBL);
  configASSERT(st == SL_STATUS_OK);

  s_dma_busy = false;
}

void elev_tx_start_task(void)
{
  BaseType_t ok = xTaskCreate(tx_task,
                              "elev_tx",
                              ELEV_TX_TASK_STACK,
                              NULL,
                              ELEV_TX_TASK_PRIO,
                              &s_tx_task);
  configASSERT(ok == pdPASS);
}

bool elev_tx_enqueue(const uint8_t *data, uint16_t len)
{
  elev_tx_msg_t msg;

  if (data == NULL || len == 0U || len > ELEV_TX_MSG_MAX) {
    return false;
  }
  msg.len = len;
  memcpy(msg.data, data, len);
  return xQueueSend(s_tx_q, &msg, pdMS_TO_TICKS(50)) == pdTRUE;
}

bool elev_tx_print(const char *s)
{
  if (s == NULL) {
    return false;
  }
  size_t n = strlen(s);
  if (n > ELEV_TX_MSG_MAX) {
    n = ELEV_TX_MSG_MAX;
  }
  return elev_tx_enqueue((const uint8_t *)s, (uint16_t)n);
}

bool elev_tx_enqueue_echo_from_isr(uint8_t c, BaseType_t *woken)
{
  elev_tx_msg_t msg;
  msg.len = 1U;
  msg.data[0] = c;
  return xQueueSendFromISR(s_tx_q, &msg, woken) == pdTRUE;
}
