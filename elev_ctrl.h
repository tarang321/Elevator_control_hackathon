/***************************************************************************//**
 * @file elev_ctrl.h
 * @brief Elevator application logic task
 ******************************************************************************/
#ifndef ELEV_CTRL_H
#define ELEV_CTRL_H

#include "FreeRTOS.h"
#include "queue.h"

void elev_ctrl_init(QueueHandle_t req_queue);
void elev_ctrl_start_task(void);

#endif /* ELEV_CTRL_H */
