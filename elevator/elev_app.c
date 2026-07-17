/***************************************************************************//**
 * @file elev_app.c
 * @brief Create primitives/tasks, timers, banner, enable UART RX IRQ
 *******************************************************************************
 * Init order matches architecture doc. Optional: status timer, button → exit 1.
 ******************************************************************************/
#include "elev_app.h"
#include "elev_gpio.h"
#include "elev_rx.h"
#include "elev_tx.h"
#include "elev_parser.h"
#include "elev_ctrl.h"
#include "elev_types.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static QueueHandle_t s_req_q;
static TimerHandle_t s_btn_tmr;
static TimerHandle_t s_status_tmr;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void btn_timer_cb(TimerHandle_t tmr);
static void status_timer_cb(TimerHandle_t tmr);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void btn_timer_cb(TimerHandle_t tmr)
{
  elev_req_t req;
  elev_btn_id_t btn;
  (void)tmr;

  btn = elev_gpio_btn_poll();
  if (btn == ELEV_BTN0) {
    req.type = ELEV_REQ_EXIT;
    req.count = 1;
    (void)xQueueSend(s_req_q, &req, 0);
  } else if (btn == ELEV_BTN1) {
    req.type = ELEV_REQ_ENTER;
    req.count = 1;
    (void)xQueueSend(s_req_q, &req, 0);
  }
}

static void status_timer_cb(TimerHandle_t tmr)
{
  elev_req_t req;
  (void)tmr;

  req.type = ELEV_REQ_STATUS;
  req.count = 0;
  (void)xQueueSend(s_req_q, &req, 0);
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void elev_app_init(void)
{
  char banner[160];

  elev_gpio_init();
  elev_uart_hw_init();

  s_req_q = xQueueCreate(ELEV_REQ_QUEUE_LEN, sizeof(elev_req_t));
  configASSERT(s_req_q != NULL);

  elev_tx_init();
  elev_ctrl_init(s_req_q);
  elev_parser_init(s_req_q);

  elev_parser_start_task();
  elev_rx_init(elev_parser_task_handle());
  elev_ctrl_start_task();
  elev_tx_start_task();

  s_btn_tmr = xTimerCreate("elev_btn",
                           pdMS_TO_TICKS(ELEV_APP_TICK_MS),
                           pdTRUE,
                           NULL,
                           btn_timer_cb);
  configASSERT(s_btn_tmr != NULL);
  (void)xTimerStart(s_btn_tmr, 0);

  s_status_tmr = xTimerCreate("elev_stat",
                              pdMS_TO_TICKS(ELEV_STATUS_PERIOD_MS),
                              pdTRUE,
                              NULL,
                              status_timer_cb);
  configASSERT(s_status_tmr != NULL);
  (void)xTimerStart(s_status_tmr, 0);

  snprintf(banner, sizeof(banner),
           "\r\nElevator Capacity Controller Demo\r\n"
           "Max capacity: %u\r\n"
           "BTN0=exit 1  BTN1=enter 1  LED0=free  LED1=full/blocked\r\n"
           "Type 'help' for commands.\r\n> ",
           (unsigned)elev_ctrl_get_capacity());
  elev_tx_print(banner);

  elev_rx_enable();
}
