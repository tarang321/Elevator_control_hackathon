/***************************************************************************//**
 * @file elev_parser.c
 * @brief Parse enter/exit/status/help → reqQ (or TX error)
 ******************************************************************************/
#include "elev_parser.h"
#include "elev_rx.h"
#include "elev_tx.h"
#include "elev_gpio.h"
#include "elev_types.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

static void skip_ws(char **p);
static bool parse_u16(char **p, uint16_t *out);
static void send_err(const char *msg);
static void parser_task(void *arg);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void skip_ws(char **p)
{
  while ((**p != '\0') && isspace((unsigned char)**p)) {
    (*p)++;
  }
}

static bool parse_u16(char **p, uint16_t *out)
{
  char *end = NULL;
  long v;

  skip_ws(p);
  if (**p == '\0') {
    return false;
  }
  if ((**p == '-') || (**p == '+')) {
    if (**p == '-') {
      return false;
    }
    (*p)++;
  }
  if (!isdigit((unsigned char)**p)) {
    return false;
  }
  v = strtol(*p, &end, 10);
  if ((end == *p) || (v <= 0) || (v > 65535)) {
    return false;
  }
  *p = end;
  skip_ws(p);
  if (**p != '\0') {
    return false;
  }
  *out = (uint16_t)v;
  return true;
}

static void send_err(const char *msg)
{
  elev_tx_print(msg);
}

static void parser_task(void *arg)
{
  char line[ELEV_CMD_BUF_LEN];
  (void)arg;

  for (;;) {
    bool overflow = false;
    uint16_t n;
    char *p;
    elev_req_t req;

    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    elev_gpio_parser_set(true);

    n = elev_rx_take_command(line, sizeof(line), &overflow);
    if (overflow) {
      send_err("Error: command too long.\r\n");
      elev_gpio_parser_set(false);
      continue;
    }
    if (n == 0U) {
      elev_gpio_parser_set(false);
      continue;
    }

    p = line;
    skip_ws(&p);
    if (*p == '\0') {
      elev_gpio_parser_set(false);
      continue;
    }

    memset(&req, 0, sizeof(req));

    if ((strncmp(p, "enter", 5) == 0)
        && ((p[5] == '\0') || isspace((unsigned char)p[5]))) {
      p += 5;
      skip_ws(&p);
      if (*p == '\0') {
        send_err("Error: missing argument. Usage: enter <count>\r\n");
      } else if (!parse_u16(&p, &req.count)) {
        send_err("Error: passenger count must be greater than zero.\r\n");
      } else {
        req.type = ELEV_REQ_ENTER;
        if (xQueueSend(s_req_q, &req, pdMS_TO_TICKS(50)) != pdTRUE) {
          send_err("Error: request queue full.\r\n");
        }
      }
    } else if ((strncmp(p, "exit", 4) == 0)
               && ((p[4] == '\0') || isspace((unsigned char)p[4]))) {
      p += 4;
      skip_ws(&p);
      if (*p == '\0') {
        send_err("Error: missing argument. Usage: exit <count>\r\n");
      } else if (!parse_u16(&p, &req.count)) {
        send_err("Error: passenger count must be greater than zero.\r\n");
      } else {
        req.type = ELEV_REQ_EXIT;
        if (xQueueSend(s_req_q, &req, pdMS_TO_TICKS(50)) != pdTRUE) {
          send_err("Error: request queue full.\r\n");
        }
      }
    } else if ((strncmp(p, "status", 6) == 0)
               && ((p[6] == '\0') || isspace((unsigned char)p[6]))) {
      p += 6;
      skip_ws(&p);
      if (*p != '\0') {
        send_err("Error: unknown command. Type 'help' for supported commands.\r\n");
      } else {
        req.type = ELEV_REQ_STATUS;
        if (xQueueSend(s_req_q, &req, pdMS_TO_TICKS(50)) != pdTRUE) {
          send_err("Error: request queue full.\r\n");
        }
      }
    } else if ((strncmp(p, "help", 4) == 0)
               && ((p[4] == '\0') || isspace((unsigned char)p[4]))) {
      p += 4;
      skip_ws(&p);
      if (*p != '\0') {
        send_err("Error: unknown command. Type 'help' for supported commands.\r\n");
      } else {
        req.type = ELEV_REQ_HELP;
        if (xQueueSend(s_req_q, &req, pdMS_TO_TICKS(50)) != pdTRUE) {
          send_err("Error: request queue full.\r\n");
        }
      }
    } else {
      send_err("Error: unknown command. Type 'help' for supported commands.\r\n");
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
