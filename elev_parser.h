/***************************************************************************//**
 * @file elev_parser.h
 * @brief UART command parser task
 ******************************************************************************/
#ifndef ELEV_PARSER_H
#define ELEV_PARSER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void elev_parser_init(QueueHandle_t req_queue);
void elev_parser_start_task(void);
TaskHandle_t elev_parser_task_handle(void);

#endif /* ELEV_PARSER_H */
