/***************************************************************************//**
 * @file elev_ctrl.h
 * @brief Elevator capacity / wait-queue FreeRTOS task
 *******************************************************************************
 * Slot: elevator/ — PDF §9 / §10. Partial board + FIFO remainder.
 ******************************************************************************/
#ifndef ELEV_CTRL_H
#define ELEV_CTRL_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void elev_ctrl_init(QueueHandle_t req_queue);
void elev_ctrl_start_task(void);

/** Current configured max capacity (runtime, after capacity command). */
uint16_t elev_ctrl_get_capacity(void);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_CTRL_H */
