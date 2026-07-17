/***************************************************************************//**
 * @file elev_gpio.h
 * @brief Logic-analyzer GPIO instrumentation API
 *******************************************************************************
 * Slot: elevator/ — PDF §11 signals (pulse / level).
 ******************************************************************************/
#ifndef ELEV_GPIO_H
#define ELEV_GPIO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void elev_gpio_init(void);

void elev_gpio_rx_isr_pulse(void);
void elev_gpio_cmd_complete_pulse(void);
void elev_gpio_parser_set(bool active);
void elev_gpio_elevator_set(bool active);
void elev_gpio_entry_blocked_set(bool blocked);
void elev_gpio_sem_give_pulse(void);
void elev_gpio_sem_take_pulse(void);
void elev_gpio_dma_active_set(bool active);
void elev_gpio_dma_complete_pulse(void);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_GPIO_H */
