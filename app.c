/***************************************************************************//**
 * @file
 * @brief Top-level application hooks (Simplicity Studio / sl_main)
 ******************************************************************************/
#include "elev_app.h"

/***************************************************************************//**
 * @brief Initialize application — starts elevator FreeRTOS modules.
 ******************************************************************************/
void app_init(void)
{
  elev_app_init();
}
