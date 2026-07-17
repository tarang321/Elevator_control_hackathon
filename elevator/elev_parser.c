/***************************************************************************//**
 * @file elev_parser.c
 * @brief Parse UART commands → reqQ (or TX error)
 ******************************************************************************/
#include "elev_parser.h"
#include "elev_parse_util.h"
#include "elev_rx.h"
#include "elev_tx.h"
#include "elev_gpio.h"
#include "elev_types.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define ELEV_PARSER_STACK  256
#define ELEV_PARSER_PRIO   (tskIDLE_PRIORITY + 2)

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static QueueHandle_t s_req_q;
static TaskHandle_t s_parser_task;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void parser_task(void *arg);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void parser_task(void *arg)
{
  char line[ELEV_CMD_BUF_LEN];
  char err[96];
  (void)arg;

  for (;;) {
    bool overflow = false;
    uint16_t n;
    elev_req_t req;

    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    elev_gpio_parser_set(true);

    n = elev_rx_take_command(line, sizeof(line), &overflow);
    if (overflow) {
      elev_tx_print("Error: command too long.\r\n");
      elev_gpio_parser_set(false);
      continue;
    }
    if (n == 0U) {
      elev_gpio_parser_set(false);
      continue;
    }

    err[0] = '\0';
    if (!elev_parse_line(line, &req, err, (uint16_t)sizeof(err))) {
      if (err[0] != '\0') {
        elev_tx_print(err);
      }
      elev_gpio_parser_set(false);
      continue;
    }

    if (xQueueSend(s_req_q, &req, pdMS_TO_TICKS(50)) != pdTRUE) {
      elev_tx_print("Error: request queue full.\r\n");
    }

    elev_gpio_parser_set(false);
  }
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void elev_parser_init(QueueHandle_t req_queue)
{
  s_req_q = req_queue;
}

void elev_parser_start_task(void)
{
  BaseType_t ok = xTaskCreate(parser_task,
                              "elev_parser",
                              ELEV_PARSER_STACK,
                              NULL,
                              ELEV_PARSER_PRIO,
                              &s_parser_task);
  configASSERT(ok == pdPASS);
}

TaskHandle_t elev_parser_task_handle(void)
{
  return s_parser_task;
}
