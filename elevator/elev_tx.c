/***************************************************************************//**
 * @file elev_tx.c
 * @brief UART TX byte ring drained by LDMA (no FreeRTOS TX task)
 *******************************************************************************
 * Callers write the ring. If DMA is idle, kick M2P. DMA-done advances head and
 * starts the next contiguous span (handles wrap in a second transfer).
 ******************************************************************************/
#include "elev_tx.h"
#include "elev_gpio.h"

#include <string.h>

#include "em_usart.h"
#include "em_device.h"
#include "em_core.h"

#include "FreeRTOS.h"

#include "sl_dma_manager.h"
#include "sl_dma_channel.h"
#include "sl_device_dma.h"
#include "sl_status.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#if (ELEV_TX_RING_SIZE & (ELEV_TX_RING_SIZE - 1U)) != 0U
#error ELEV_TX_RING_SIZE must be a power of 2
#endif

#define ELEV_TX_RING_MASK   (ELEV_TX_RING_SIZE - 1U)

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static uint8_t s_ring[ELEV_TX_RING_SIZE];
static volatile uint16_t s_head;
static volatile uint16_t s_tail;
static sl_dma_channel_handle_t s_dma_tx;
static uint8_t s_dma_ch;
static volatile bool s_dma_busy;
static volatile uint16_t s_dma_len;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static uint16_t ring_used(void);
static uint16_t ring_free(void);
static bool ring_write(const uint8_t *data, uint16_t len);
static void try_start_dma(void);
static void dma_cb(sl_dma_channel_handle_t *handle,
                   void *user_data,
                   bool error,
                   bool aborted);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static uint16_t ring_used(void)
{
  return (uint16_t)((s_tail - s_head) & ELEV_TX_RING_MASK);
}

static uint16_t ring_free(void)
{
  /* Leave one slot empty so head==tail means empty. */
  return (uint16_t)((ELEV_TX_RING_SIZE - 1U) - ring_used());
}

static bool ring_write(const uint8_t *data, uint16_t len)
{
  uint16_t i;

  if ((data == NULL) || (len == 0U)) {
    return false;
  }
  if (len > ring_free()) {
    return false;
  }

  for (i = 0U; i < len; i++) {
    s_ring[s_tail] = data[i];
    s_tail = (uint16_t)((s_tail + 1U) & ELEV_TX_RING_MASK);
  }
  return true;
}

static void try_start_dma(void)
{
  uint16_t used;
  uint16_t head;
  uint16_t contig;
  sl_status_t st;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  if (s_dma_busy) {
    CORE_EXIT_ATOMIC();
    return;
  }

  used = ring_used();
  if (used == 0U) {
    CORE_EXIT_ATOMIC();
    return;
  }

  head = s_head;
  contig = (uint16_t)(ELEV_TX_RING_SIZE - head);
  if (contig > used) {
    contig = used;
  }

  s_dma_len = contig;
  s_dma_busy = true;
  CORE_EXIT_ATOMIC();

  elev_gpio_dma_active_set(true);

  st = sl_dma_channel_submit_transfer_m2p(
    &s_dma_tx,
    (void *)&s_ring[head],
    (void *)&USART0->TXDATA,
    (size_t)contig,
    SL_DMA_CTRL_SIZE_BYTE,
    NULL);

  if (st != SL_STATUS_OK) {
    elev_gpio_dma_active_set(false);
    CORE_ENTER_ATOMIC();
    s_dma_busy = false;
    s_dma_len = 0U;
    CORE_EXIT_ATOMIC();
  }
}

static void dma_cb(sl_dma_channel_handle_t *handle,
                   void *user_data,
                   bool error,
                   bool aborted)
{
  (void)handle;
  (void)user_data;
  (void)error;
  (void)aborted;

  elev_gpio_dma_active_set(false);
  elev_gpio_dma_complete_pulse();

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  s_head = (uint16_t)((s_head + s_dma_len) & ELEV_TX_RING_MASK);
  s_dma_len = 0U;
  s_dma_busy = false;
  CORE_EXIT_ATOMIC();

  try_start_dma();
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void elev_tx_init(void)
{
  sl_status_t st;

  s_head = 0U;
  s_tail = 0U;
  s_dma_busy = false;
  s_dma_len = 0U;

  st = sl_dma_manager_allocate_channel(NULL, &s_dma_ch);
  configASSERT(st == SL_STATUS_OK);

  st = sl_dma_channel_init(&s_dma_tx, NULL, s_dma_ch, dma_cb, NULL);
  configASSERT(st == SL_STATUS_OK);

  st = sl_dma_channel_set_peripheral_signal(&s_dma_tx, SL_DMA_SIGNAL_USART0_TXBL);
  configASSERT(st == SL_STATUS_OK);
}

bool elev_tx_enqueue(const uint8_t *data, uint16_t len)
{
  bool ok;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  ok = ring_write(data, len);
  CORE_EXIT_ATOMIC();

  if (ok) {
    try_start_dma();
  }
  return ok;
}

bool elev_tx_print(const char *s)
{
  size_t n;

  if (s == NULL) {
    return false;
  }
  n = strlen(s);
  if (n > ELEV_TX_MSG_MAX) {
    n = ELEV_TX_MSG_MAX;
  }
  return elev_tx_enqueue((const uint8_t *)s, (uint16_t)n);
}

bool elev_tx_enqueue_echo_from_isr(uint8_t c, BaseType_t *woken)
{
  bool ok;

  if (woken != NULL) {
    *woken = pdFALSE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  ok = ring_write(&c, 1U);
  CORE_EXIT_ATOMIC();

  if (ok) {
    try_start_dma();
  }
  return ok;
}
