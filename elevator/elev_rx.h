/***************************************************************************//**
 * @file elev_rx.h
 * @brief UART RX ISR path + ping-pong command buffers
 *******************************************************************************
 * Slot: elevator/ — PDF §5. ISR must not parse or run elevator logic.
 ******************************************************************************/
#ifndef ELEV_RX_H
#define ELEV_RX_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "elev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void elev_uart_hw_init(void);
void elev_rx_init(TaskHandle_t parser_task);
void elev_rx_enable(void);

/**
 * @brief Copy a completed command line for the parser task.
 * @return Length copied (0 if none ready).
 */
uint16_t elev_rx_take_command(char *out, uint16_t out_max, bool *overflow);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_RX_H */
