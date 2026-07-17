/***************************************************************************//**
 * @file elev_gpio.c
 * @brief GPIO instrumentation (short set/clear pulses and levels)
 ******************************************************************************/
#include "elev_gpio.h"
#include "sl_gpio.h"
#include "sl_clock_manager.h"

/* Placeholder map — document in ARCHITECTURE.md */
static const sl_gpio_t pin_rx_isr     = { .port = SL_GPIO_PORT_C, .pin = 0 };
static const sl_gpio_t pin_cmd_done   = { .port = SL_GPIO_PORT_C, .pin = 1 };
static const sl_gpio_t pin_parser     = { .port = SL_GPIO_PORT_C, .pin = 2 };
static const sl_gpio_t pin_elevator   = { .port = SL_GPIO_PORT_C, .pin = 3 };
static const sl_gpio_t pin_blocked    = { .port = SL_GPIO_PORT_C, .pin = 4 };
static const sl_gpio_t pin_sem_give   = { .port = SL_GPIO_PORT_C, .pin = 5 };
static const sl_gpio_t pin_sem_take   = { .port = SL_GPIO_PORT_C, .pin = 6 };
static const sl_gpio_t pin_dma_active = { .port = SL_GPIO_PORT_C, .pin = 7 };
static const sl_gpio_t pin_dma_done   = { .port = SL_GPIO_PORT_D, .pin = 2 };

static void cfg_out(const sl_gpio_t *g)
{
  sl_gpio_set_pin_mode(g, SL_GPIO_MODE_PUSH_PULL, 0);
}

static void pulse(const sl_gpio_t *g)
{
  sl_gpio_set_pin(g);
  sl_gpio_clear_pin(g);
}

void elev_gpio_init(void)
{
  (void)sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  cfg_out(&pin_rx_isr);
  cfg_out(&pin_cmd_done);
  cfg_out(&pin_parser);
  cfg_out(&pin_elevator);
  cfg_out(&pin_blocked);
  cfg_out(&pin_sem_give);
  cfg_out(&pin_sem_take);
  cfg_out(&pin_dma_active);
  cfg_out(&pin_dma_done);
}

void elev_gpio_rx_isr_pulse(void)
{
  pulse(&pin_rx_isr);
}

void elev_gpio_cmd_complete_pulse(void)
{
  pulse(&pin_cmd_done);
}

void elev_gpio_parser_set(bool active)
{
  if (active) {
    sl_gpio_set_pin(&pin_parser);
  } else {
    sl_gpio_clear_pin(&pin_parser);
  }
}

void elev_gpio_elevator_set(bool active)
{
  if (active) {
    sl_gpio_set_pin(&pin_elevator);
  } else {
    sl_gpio_clear_pin(&pin_elevator);
  }
}

void elev_gpio_entry_blocked_set(bool blocked)
{
  if (blocked) {
    sl_gpio_set_pin(&pin_blocked);
  } else {
    sl_gpio_clear_pin(&pin_blocked);
  }
}

void elev_gpio_sem_give_pulse(void)
{
  pulse(&pin_sem_give);
}

void elev_gpio_sem_take_pulse(void)
{
  pulse(&pin_sem_take);
}

void elev_gpio_dma_active_set(bool active)
{
  if (active) {
    sl_gpio_set_pin(&pin_dma_active);
  } else {
    sl_gpio_clear_pin(&pin_dma_active);
  }
}

void elev_gpio_dma_complete_pulse(void)
{
  pulse(&pin_dma_done);
}
