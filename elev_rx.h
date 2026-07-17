/***************************************************************************//**
 * @file elev_rx.h
 * @brief UART RX ISR + ping-pong command buffers
 ******************************************************************************/
#ifndef ELEV_RX_H
#define ELEV_RX_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "elev_types.h"

void elev_uart_hw_init(void);
void elev_rx_init(TaskHandle_t parser_task);
void elev_rx_enable(void);

/** Parser: copy completed line; returns length, 0 if none. */
uint16_t elev_rx_take_command(char *out, uint16_t out_max, bool *overflow);

#endif /* ELEV_RX_H */
