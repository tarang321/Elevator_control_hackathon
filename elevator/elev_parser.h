/***************************************************************************//**
 * @file elev_parser.h
 * @brief UART command parser FreeRTOS task
 *******************************************************************************
 * Slot: elevator/ — PDF §8. Validates input; does not touch occupancy.
 ******************************************************************************/
#ifndef ELEV_PARSER_H
#define ELEV_PARSER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void elev_parser_init(QueueHandle_t req_queue);
void elev_parser_start_task(void);
TaskHandle_t elev_parser_task_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_PARSER_H */
