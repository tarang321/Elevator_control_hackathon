/***************************************************************************//**
 * @file elev_gpio.h
 * @brief Logic-analyzer GPIO instrumentation + onboard LED/button API
 *******************************************************************************
 * Slot: elevator/ — PDF §11 signals + BRD4187C LED0/LED1 + BTN0/BTN1.
 ******************************************************************************/
#ifndef ELEV_GPIO_H
#define ELEV_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ELEV_BTN_NONE = 0,
  ELEV_BTN0,                 /**< Onboard BTN0 — exit 1 */
  ELEV_BTN1                  /**< Onboard BTN1 — enter 1 */
} elev_btn_id_t;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void elev_gpio_init(void);

/* --- Mandatory instrumentation (PC00–PC07, PD02) --- */
void elev_gpio_rx_isr_pulse(void);
void elev_gpio_cmd_complete_pulse(void);
void elev_gpio_parser_set(bool active);
void elev_gpio_elevator_set(bool active);
void elev_gpio_entry_blocked_set(bool blocked);
void elev_gpio_sem_give_pulse(void);
void elev_gpio_sem_take_pulse(void);
void elev_gpio_dma_active_set(bool active);
void elev_gpio_dma_complete_pulse(void);

/* --- Onboard LEDs: LED0 (PB02) available, LED1 (PB04) full/blocked --- */
void elev_gpio_led_update(bool available, bool full, bool blocked);
void elev_gpio_led_blink_tick(void);

/* --- Onboard buttons: BTN0 (PB01), BTN1 (PB03), active-low --- */
void elev_gpio_btn_init(void);

/**
 * @brief Debounced press edge; call from app tick (~50 ms).
 * @return Which button was pressed, or ELEV_BTN_NONE.
 */
elev_btn_id_t elev_gpio_btn_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_GPIO_H */
