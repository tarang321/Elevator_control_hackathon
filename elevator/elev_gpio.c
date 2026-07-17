/***************************************************************************//**
 * @file elev_gpio.c
 * @brief GPIO instrumentation + BRD4187C onboard LEDs / buttons
 *******************************************************************************
 * Instrumentation (unchanged):
 *   PC00–PC07, PD02 — logic-analyzer signals
 *
 * Onboard UI (UG526 / BRD4187C):
 *   PB02 LED0  — capacity available (active-high)
 *   PB04 LED1  — full solid / deferred blink (active-high)
 *   PB01 BTN0  — exit 1 (active-low, RC debounced on board)
 *   PB03 BTN1  — enter 1 (active-low)
 ******************************************************************************/
#include "elev_gpio.h"

#include "sl_gpio.h"
#include "sl_clock_manager.h"
#include "sl_simple_led_instances.h"
#include "sl_led.h"

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static const sl_gpio_t pin_rx_isr     = { .port = SL_GPIO_PORT_C, .pin = 0 };
static const sl_gpio_t pin_cmd_done   = { .port = SL_GPIO_PORT_C, .pin = 1 };
static const sl_gpio_t pin_parser     = { .port = SL_GPIO_PORT_C, .pin = 2 };
static const sl_gpio_t pin_elevator   = { .port = SL_GPIO_PORT_C, .pin = 3 };
static const sl_gpio_t pin_blocked    = { .port = SL_GPIO_PORT_C, .pin = 4 };
static const sl_gpio_t pin_sem_give   = { .port = SL_GPIO_PORT_C, .pin = 5 };
static const sl_gpio_t pin_sem_take   = { .port = SL_GPIO_PORT_C, .pin = 6 };
static const sl_gpio_t pin_dma_active = { .port = SL_GPIO_PORT_C, .pin = 7 };
static const sl_gpio_t pin_dma_done   = { .port = SL_GPIO_PORT_D, .pin = 2 };

/* Onboard LED1 (LED0 uses sl_led_led0 on PB02). */
static const sl_gpio_t pin_led1       = { .port = SL_GPIO_PORT_B, .pin = 4 };
static const sl_gpio_t pin_btn0       = { .port = SL_GPIO_PORT_B, .pin = 1 };
static const sl_gpio_t pin_btn1       = { .port = SL_GPIO_PORT_B, .pin = 3 };

static bool s_led_available;
static bool s_led_full;
static bool s_led_blocked;
static uint8_t s_blink_div;
static bool s_led1_phase;

typedef struct {
  bool stable_down;
  uint8_t debounce;
  bool armed;
} btn_state_t;

static btn_state_t s_btn0;
static btn_state_t s_btn1;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void cfg_out(const sl_gpio_t *g);
static void pulse(const sl_gpio_t *g);
static void apply_leds(void);
static bool btn_read_down(const sl_gpio_t *pin);
static elev_btn_id_t btn_poll_one(btn_state_t *st,
                                  const sl_gpio_t *pin,
                                  elev_btn_id_t id);

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void cfg_out(const sl_gpio_t *g)
{
  sl_gpio_set_pin_mode(g, SL_GPIO_MODE_PUSH_PULL, 0);
}

static void pulse(const sl_gpio_t *g)
{
  sl_gpio_set_pin(g);
  sl_gpio_clear_pin(g);
}

static void apply_leds(void)
{
  if (s_led_available) {
    sl_led_turn_on(&sl_led_led0);
  } else {
    sl_led_turn_off(&sl_led_led0);
  }

  if (s_led_blocked) {
    if (s_led1_phase) {
      sl_gpio_set_pin(&pin_led1);
    } else {
      sl_gpio_clear_pin(&pin_led1);
    }
  } else if (s_led_full) {
    sl_gpio_set_pin(&pin_led1);
  } else {
    sl_gpio_clear_pin(&pin_led1);
  }
}

static bool btn_read_down(const sl_gpio_t *pin)
{
  bool pin_high = true;
  (void)sl_gpio_get_pin_input(pin, &pin_high);
  return !pin_high;
}

static elev_btn_id_t btn_poll_one(btn_state_t *st,
                                  const sl_gpio_t *pin,
                                  elev_btn_id_t id)
{
  bool down = btn_read_down(pin);

  if (down == st->stable_down) {
    st->debounce = 0;
    return ELEV_BTN_NONE;
  }

  if (++st->debounce < 1U) {
    return ELEV_BTN_NONE;
  }

  st->stable_down = down;
  st->debounce = 0;
  if (down && st->armed) {
    st->armed = false;
    return id;
  }
  if (!down) {
    st->armed = true;
  }
  return ELEV_BTN_NONE;
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

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

  cfg_out(&pin_led1);
  sl_led_init(&sl_led_led0);
  s_led_available = true;
  s_led_full = false;
  s_led_blocked = false;
  s_blink_div = 0;
  s_led1_phase = true;
  apply_leds();

  elev_gpio_btn_init();
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

void elev_gpio_led_update(bool available, bool full, bool blocked)
{
  s_led_available = available;
  s_led_full = full;
  s_led_blocked = blocked;
  if (!blocked) {
    s_led1_phase = true;
    s_blink_div = 0;
  }
  apply_leds();
}

void elev_gpio_led_blink_tick(void)
{
  if (!s_led_blocked) {
    return;
  }
  if (++s_blink_div < 5U) {
    return;
  }
  s_blink_div = 0;
  s_led1_phase = !s_led1_phase;
  apply_leds();
}

void elev_gpio_btn_init(void)
{
  sl_gpio_set_pin_mode(&pin_btn0, SL_GPIO_MODE_INPUT_PULL, 1);
  sl_gpio_set_pin_mode(&pin_btn1, SL_GPIO_MODE_INPUT_PULL, 1);
  s_btn0.stable_down = false;
  s_btn0.debounce = 0;
  s_btn0.armed = true;
  s_btn1.stable_down = false;
  s_btn1.debounce = 0;
  s_btn1.armed = true;
}

elev_btn_id_t elev_gpio_btn_poll(void)
{
  elev_btn_id_t hit;

  hit = btn_poll_one(&s_btn0, &pin_btn0, ELEV_BTN0);
  if (hit != ELEV_BTN_NONE) {
    return hit;
  }
  return btn_poll_one(&s_btn1, &pin_btn1, ELEV_BTN1);
}
