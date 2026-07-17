/***************************************************************************//**
 * @file elev_app.c
 * @brief Create queues/tasks and enable UART RX
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

void elev_app_init(void)
{
  char banner[96];

  elev_gpio_init();
  elev_uart_hw_init();

  QueueHandle_t req_q = xQueueCreate(ELEV_REQ_QUEUE_LEN, sizeof(elev_req_t));
  configASSERT(req_q != NULL);

  elev_tx_init();
  elev_ctrl_init(req_q);
  elev_parser_init(req_q);

  elev_parser_start_task();
  elev_rx_init(elev_parser_task_handle());
  elev_ctrl_start_task();
  elev_tx_start_task();

  snprintf(banner, sizeof(banner),
           "\r\nElevator Capacity Controller Demo\r\n"
           "Max capacity: %u\r\n"
           "Type 'help' for commands.\r\n> ",
           (unsigned)ELEV_MAX_CAPACITY);
  elev_tx_print(banner);

  elev_rx_enable();
}
