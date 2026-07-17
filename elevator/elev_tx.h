/***************************************************************************//**
 * @file elev_tx.h
 * @brief UART TX service — sole LDMA owner (echo + responses)
 *******************************************************************************
 * Slot: elevator/ — PDF §6 / §7 serialization.
 ******************************************************************************/
#ifndef ELEV_TX_H
#define ELEV_TX_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "elev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void elev_tx_init(void);
void elev_tx_start_task(void);

bool elev_tx_enqueue(const uint8_t *data, uint16_t len);
bool elev_tx_print(const char *s);
bool elev_tx_enqueue_echo_from_isr(uint8_t c, BaseType_t *woken);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_TX_H */
