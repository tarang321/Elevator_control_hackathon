/***************************************************************************//**
 * @file elev_ctrl.c
 * @brief Elevator capacity: partial board + FIFO wait remainder
 ******************************************************************************/
#include "elev_ctrl.h"
#include "elev_tx.h"
#include "elev_gpio.h"
#include "elev_types.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define ELEV_CTRL_STACK  320
#define ELEV_CTRL_PRIO   (tskIDLE_PRIORITY + 2)

static QueueHandle_t s_req_q;
static QueueHandle_t s_wait_q;
static SemaphoreHandle_t s_sem_capacity;
static SemaphoreHandle_t s_mtx_state;
static uint16_t s_occupancy;

static uint16_t free_seats(void)
{
  return (uint16_t)uxSemaphoreGetCount(s_sem_capacity);
}

static uint16_t s_waiting_total;

static void update_blocked_gpio(void)
{
  elev_gpio_entry_blocked_set(s_waiting_total > 0U);
}

static void claim_seats(uint16_t board)
{
  for (uint16_t i = 0; i < board; i++) {
    BaseType_t ok = xSemaphoreTake(s_sem_capacity, 0);
    configASSERT(ok == pdTRUE);
    elev_gpio_sem_take_pulse();
  }
  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  s_occupancy = (uint16_t)(s_occupancy + board);
  xSemaphoreGive(s_mtx_state);
}

static void board_passengers(uint16_t n)
{
  char msg[ELEV_TX_MSG_MAX];
  uint16_t free = free_seats();
  uint16_t board = (n < free) ? n : free;
  uint16_t remain = (uint16_t)(n - board);

  if (board > 0U) {
    claim_seats(board);
    snprintf(msg, sizeof(msg),
             "Entry allowed: %u passenger(s) entered.\r\n"
             "Current occupancy: %u/%u\r\n",
             (unsigned)board,
             (unsigned)s_occupancy,
             (unsigned)ELEV_MAX_CAPACITY);
    elev_tx_print(msg);
  }

  if (remain > 0U) {
    if (xQueueSend(s_wait_q, &remain, 0) != pdTRUE) {
      elev_tx_print("Error: wait queue full.\r\n");
    } else {
      s_waiting_total = (uint16_t)(s_waiting_total + remain);
      update_blocked_gpio();
      snprintf(msg, sizeof(msg),
               "Entry deferred: %u passenger(s) waiting (insufficient capacity).\r\n",
               (unsigned)remain);
      elev_tx_print(msg);
    }
  }
}

static void drain_wait_queue(void)
{
  char msg[ELEV_TX_MSG_MAX];

  while (s_waiting_total > 0U && free_seats() > 0U) {
    uint16_t head = 0;
    if (xQueueReceive(s_wait_q, &head, 0) != pdTRUE) {
      break;
    }
    if (head == 0U) {
      continue;
    }

    uint16_t free = free_seats();
    uint16_t board = (head < free) ? head : free;
    uint16_t left = (uint16_t)(head - board);

    if (board > 0U) {
      claim_seats(board);
      s_waiting_total = (uint16_t)(s_waiting_total - board);
      snprintf(msg, sizeof(msg),
               "Deferred entry admitted: %u passenger(s) entered.\r\n"
               "Current occupancy: %u/%u\r\n",
               (unsigned)board,
               (unsigned)s_occupancy,
               (unsigned)ELEV_MAX_CAPACITY);
      elev_tx_print(msg);
    }

    if (left > 0U) {
      /* Push remainder back to front: use send-to-front */
      if (xQueueSendToFront(s_wait_q, &left, 0) != pdTRUE) {
        /* Should not happen — restore count */
        s_waiting_total = (uint16_t)(s_waiting_total + left);
        elev_tx_print("Error: wait queue full.\r\n");
        break;
      }
    }
  }
  update_blocked_gpio();
}

static void handle_exit(uint16_t n)
{
  char msg[ELEV_TX_MSG_MAX];

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  uint16_t occ = s_occupancy;
  xSemaphoreGive(s_mtx_state);

  if (n > occ) {
    snprintf(msg, sizeof(msg),
             "Error: cannot exit %u passenger(s); current occupancy is %u.\r\n",
             (unsigned)n, (unsigned)occ);
    elev_tx_print(msg);
    return;
  }

  for (uint16_t i = 0; i < n; i++) {
    (void)xSemaphoreGive(s_sem_capacity);
    elev_gpio_sem_give_pulse();
  }

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  s_occupancy = (uint16_t)(s_occupancy - n);
  occ = s_occupancy;
  xSemaphoreGive(s_mtx_state);

  snprintf(msg, sizeof(msg),
           "Exit processed: %u passenger(s) exited.\r\n"
           "Current occupancy: %u/%u\r\n",
           (unsigned)n, (unsigned)occ, (unsigned)ELEV_MAX_CAPACITY);
  elev_tx_print(msg);

  drain_wait_queue();
}

static void handle_status(void)
{
  char msg[ELEV_TX_MSG_MAX];
  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  uint16_t occ = s_occupancy;
  xSemaphoreGive(s_mtx_state);
  uint16_t free = free_seats();

  snprintf(msg, sizeof(msg),
           "Current occupancy: %u/%u\r\n"
           "Available capacity: %u\r\n"
           "Waiting: %u\r\n",
           (unsigned)occ, (unsigned)ELEV_MAX_CAPACITY,
           (unsigned)free, (unsigned)s_waiting_total);
  elev_tx_print(msg);
}

static void handle_help(void)
{
  elev_tx_print(
    "Commands:\r\n"
    "  enter <count>  - request passengers to enter\r\n"
    "  exit <count>   - request passengers to exit\r\n"
    "  status         - show occupancy and waiting\r\n"
    "  help           - show this help\r\n");
}

static void ctrl_task(void *arg)
{
  elev_req_t req;
  (void)arg;

  for (;;) {
    if (xQueueReceive(s_req_q, &req, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    elev_gpio_elevator_set(true);
    switch (req.type) {
      case ELEV_REQ_ENTER:
        board_passengers(req.count);
        break;
      case ELEV_REQ_EXIT:
        handle_exit(req.count);
        break;
      case ELEV_REQ_STATUS:
        handle_status();
        break;
      case ELEV_REQ_HELP:
        handle_help();
        break;
      default:
        break;
    }
    elev_gpio_elevator_set(false);
  }
}

void elev_ctrl_init(QueueHandle_t req_queue)
{
  s_req_q = req_queue;
  s_occupancy = 0;
  s_waiting_total = 0;

  s_wait_q = xQueueCreate(ELEV_WAIT_QUEUE_LEN, sizeof(uint16_t));
  configASSERT(s_wait_q != NULL);

  s_sem_capacity = xSemaphoreCreateCounting(ELEV_MAX_CAPACITY, ELEV_MAX_CAPACITY);
  configASSERT(s_sem_capacity != NULL);

  s_mtx_state = xSemaphoreCreateMutex();
  configASSERT(s_mtx_state != NULL);
}

void elev_ctrl_start_task(void)
{
  BaseType_t ok = xTaskCreate(ctrl_task,
                              "elev_ctrl",
                              ELEV_CTRL_STACK,
                              NULL,
                              ELEV_CTRL_PRIO,
                              NULL);
  configASSERT(ok == pdPASS);
}
