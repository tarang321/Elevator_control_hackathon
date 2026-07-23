/***************************************************************************//**
 * @file elev_ctrl.c
 * @brief Capacity logic: sem_capacity, mtx_state, waitQ (partial board)
 *******************************************************************************
 * NEVER block on semaphore take for ENTER (deadlock trap). Remainder → waitQ.
 * Optional: wait timeout, capacity resize, stats, LED refresh.
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

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define ELEV_CTRL_STACK  384
#define ELEV_CTRL_PRIO   (tskIDLE_PRIORITY + 2)

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static QueueHandle_t s_req_q;
static QueueHandle_t s_wait_q;
static SemaphoreHandle_t s_sem_capacity;
static SemaphoreHandle_t s_mtx_state;
static uint16_t s_occupancy;
static uint16_t s_waiting_total;
static uint16_t s_capacity;

static uint32_t s_stat_entered;
static uint32_t s_stat_exited;
static uint32_t s_stat_deferred;
static uint32_t s_stat_timeouts;
static uint16_t s_stat_max_occ;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static uint16_t free_seats(void);
static void update_blocked_gpio(void);
static void refresh_leds(void);
static void claim_seats(uint16_t board);
static void board_passengers(uint16_t n);
static void drain_wait_queue(void);
static void expire_wait_queue(void);
static void handle_exit(uint16_t n);
static void handle_status(void);
static void handle_help(void);
static void handle_stats(void);
static void handle_capacity(uint16_t new_cap);
static void handle_tick(void);
static void ctrl_task(void *arg);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static uint16_t free_seats(void)
{
  return (uint16_t)uxSemaphoreGetCount(s_sem_capacity);
}

static void update_blocked_gpio(void)
{
  elev_gpio_entry_blocked_set(s_waiting_total > 0U);
}

static void refresh_leds(void)
{
  uint16_t free = free_seats();
  bool available = (free > 0U);
  bool full = (s_occupancy >= s_capacity);
  bool blocked = (s_waiting_total > 0U);
  elev_gpio_led_update(available, full, blocked);
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
  s_stat_entered += board;
  if (s_occupancy > s_stat_max_occ) {
    s_stat_max_occ = s_occupancy;
  }
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
             "Entry allowed: %u passenger(s) entered.\r\n",
             (unsigned)board);
    elev_tx_print(msg);
  }

  if (remain > 0U) {
    elev_wait_item_t item;
    item.count = remain;
    item.deadline_ticks = xTaskGetTickCount()
                          + pdMS_TO_TICKS(ELEV_WAIT_TIMEOUT_MS);
    if (xQueueSend(s_wait_q, &item, 0) != pdTRUE) {
      elev_tx_print("Error: wait queue full.\r\n");
    } else {
      s_waiting_total = (uint16_t)(s_waiting_total + remain);
      s_stat_deferred++;
      update_blocked_gpio();
      snprintf(msg, sizeof(msg),
               "Entry deferred: %u passenger(s) waiting (insufficient capacity).\r\n",
               (unsigned)remain);
      elev_tx_print(msg);
    }
  }
  refresh_leds();
  handle_status();
}

static void drain_wait_queue(void)
{
  char msg[ELEV_TX_MSG_MAX];

  while ((s_waiting_total > 0U) && (free_seats() > 0U)) {
    elev_wait_item_t head;
    uint16_t free;
    uint16_t board;
    uint16_t left;

    if (xQueueReceive(s_wait_q, &head, 0) != pdTRUE) {
      break;
    }
    if (head.count == 0U) {
      continue;
    }

    free = free_seats();
    board = (head.count < free) ? head.count : free;
    left = (uint16_t)(head.count - board);

    if (board > 0U) {
      claim_seats(board);
      s_waiting_total = (uint16_t)(s_waiting_total - board);
      snprintf(msg, sizeof(msg),
               "Deferred entry admitted: %u passenger(s) entered.\r\n"
               "Current occupancy: %u/%u\r\n",
               (unsigned)board,
               (unsigned)s_occupancy,
               (unsigned)s_capacity);
      elev_tx_print(msg);
    }

    if (left > 0U) {
      elev_wait_item_t rest = head;
      rest.count = left;
      if (xQueueSendToFront(s_wait_q, &rest, 0) != pdTRUE) {
        s_waiting_total = (uint16_t)(s_waiting_total + left);
        elev_tx_print("Error: wait queue full.\r\n");
        break;
      }
    }
  }
  update_blocked_gpio();
  refresh_leds();
}

static void expire_wait_queue(void)
{
  char msg[ELEV_TX_MSG_MAX];
  TickType_t now = xTaskGetTickCount();
  UBaseType_t n;
  UBaseType_t i;
  elev_wait_item_t item;

  n = uxQueueMessagesWaiting(s_wait_q);
  for (i = 0; i < n; i++) {
    if (xQueueReceive(s_wait_q, &item, 0) != pdTRUE) {
      break;
    }
    if ((int32_t)(now - item.deadline_ticks) >= 0) {
      if (item.count <= s_waiting_total) {
        s_waiting_total = (uint16_t)(s_waiting_total - item.count);
      } else {
        s_waiting_total = 0;
      }
      s_stat_timeouts++;
      snprintf(msg, sizeof(msg),
               "Entry timed out: %u deferred passenger(s) dropped.\r\n",
               (unsigned)item.count);
      elev_tx_print(msg);
    } else {
      (void)xQueueSend(s_wait_q, &item, 0);
    }
  }
  update_blocked_gpio();
  refresh_leds();
}

static void handle_exit(uint16_t n)
{
  char msg[ELEV_TX_MSG_MAX];
  uint16_t occ;

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  occ = s_occupancy;
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
  s_stat_exited += n;
  xSemaphoreGive(s_mtx_state);

  snprintf(msg, sizeof(msg),
           "Exit processed: %u passenger(s) exited.\r\n",
           (unsigned)n);
  elev_tx_print(msg);

  drain_wait_queue();
  handle_status();
}

static void handle_status(void)
{
  char msg[ELEV_TX_MSG_MAX];
  uint16_t occ;
  uint16_t free;

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  occ = s_occupancy;
  xSemaphoreGive(s_mtx_state);
  free = free_seats();

  snprintf(msg, sizeof(msg),
           "Current occupancy: %u/%u\r\n"
           "Available capacity: %u\r\n"
           "Waiting: %u\r\n",
           (unsigned)occ, (unsigned)s_capacity,
           (unsigned)free, (unsigned)s_waiting_total);
  elev_tx_print(msg);
}

static void handle_help(void)
{
  elev_tx_print(
    "Commands:\r\n"
    "  enter <count>     - request passengers to enter\r\n"
    "  exit <count>      - request passengers to exit\r\n"
    "  capacity <count>  - set max capacity (1..32)\r\n"
    "  status            - show occupancy and waiting\r\n"
    "  stats             - show runtime statistics\r\n"
    "  help              - show this help\r\n");
}

static void handle_stats(void)
{
  char msg[ELEV_TX_MSG_MAX];

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  snprintf(msg, sizeof(msg),
           "Statistics:\r\n"
           "  Total entered: %lu\r\n"
           "  Total exited: %lu\r\n"
           "  Deferred entry events: %lu\r\n"
           "  Deferred timeouts: %lu\r\n"
           "  Max occupancy: %u\r\n",
           (unsigned long)s_stat_entered,
           (unsigned long)s_stat_exited,
           (unsigned long)s_stat_deferred,
           (unsigned long)s_stat_timeouts,
           (unsigned)s_stat_max_occ);
  xSemaphoreGive(s_mtx_state);
  elev_tx_print(msg);
}

static void handle_capacity(uint16_t new_cap)
{
  char msg[ELEV_TX_MSG_MAX];
  uint16_t occ;
  uint16_t needed;
  uint16_t free_new;
  SemaphoreHandle_t neu;

  if ((new_cap == 0U) || (new_cap > ELEV_ABS_MAX_CAPACITY)) {
    elev_tx_print("Error: capacity out of range.\r\n");
    return;
  }

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  occ = s_occupancy;
  needed = (uint16_t)(occ + s_waiting_total);
  xSemaphoreGive(s_mtx_state);

  if (new_cap < needed) {
    snprintf(msg, sizeof(msg),
             "Error: cannot set capacity %u; occupancy+waiting is %u.\r\n",
             (unsigned)new_cap, (unsigned)needed);
    elev_tx_print(msg);
    return;
  }

  free_new = (uint16_t)(new_cap - occ);
  neu = xSemaphoreCreateCounting(new_cap, free_new);
  if (neu == NULL) {
    elev_tx_print("Error: capacity change failed.\r\n");
    return;
  }

  xSemaphoreTake(s_mtx_state, portMAX_DELAY);
  vSemaphoreDelete(s_sem_capacity);
  s_sem_capacity = neu;
  s_capacity = new_cap;
  xSemaphoreGive(s_mtx_state);

  snprintf(msg, sizeof(msg),
           "Capacity set to %u.\r\n",
           (unsigned)new_cap);
  elev_tx_print(msg);
  refresh_leds();
  handle_status();
}

static void handle_tick(void)
{
  expire_wait_queue();
  elev_gpio_led_blink_tick();
}

static void ctrl_task(void *arg)
{
  elev_req_t req;
  (void)arg;

  for (;;) {
    /* Timed wait drives wait-timeout expiry + LED blink without flooding reqQ. */
    if (xQueueReceive(s_req_q, &req, pdMS_TO_TICKS(ELEV_APP_TICK_MS)) != pdTRUE) {
      handle_tick();
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
      case ELEV_REQ_CAPACITY:
        handle_capacity(req.count);
        break;
      case ELEV_REQ_STATS:
        handle_stats();
        break;
      case ELEV_REQ_TICK:
        handle_tick();
        break;
      default:
        break;
    }
    elev_gpio_elevator_set(false);
    handle_tick();
  }
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void elev_ctrl_init(QueueHandle_t req_queue)
{
  s_req_q = req_queue;
  s_occupancy = 0;
  s_waiting_total = 0;
  s_capacity = ELEV_MAX_CAPACITY;
  s_stat_entered = 0;
  s_stat_exited = 0;
  s_stat_deferred = 0;
  s_stat_timeouts = 0;
  s_stat_max_occ = 0;

  s_wait_q = xQueueCreate(ELEV_WAIT_QUEUE_LEN, sizeof(elev_wait_item_t));
  configASSERT(s_wait_q != NULL);

  s_sem_capacity = xSemaphoreCreateCounting(ELEV_MAX_CAPACITY, ELEV_MAX_CAPACITY);
  configASSERT(s_sem_capacity != NULL);

  s_mtx_state = xSemaphoreCreateMutex();
  configASSERT(s_mtx_state != NULL);

  refresh_leds();
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

uint16_t elev_ctrl_get_capacity(void)
{
  return s_capacity;
}
