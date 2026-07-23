/***************************************************************************//**
 * @file elev_types.h
 * @brief Shared types and limits for Elevator Capacity Controller
 *******************************************************************************
 * Slot: elevator/ — shared constants used by RX, parser, control, TX.
 ******************************************************************************/
#ifndef ELEV_TYPES_H
#define ELEV_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef ELEV_MAX_CAPACITY
#define ELEV_MAX_CAPACITY           5U
#endif

/** Absolute upper bound for runtime capacity <count>. */
#define ELEV_ABS_MAX_CAPACITY       32U

#define ELEV_CMD_BUF_LEN            64U
#define ELEV_TX_MSG_MAX             160U
/** Power-of-2 UART TX byte ring (one slot left empty). */
#define ELEV_TX_RING_SIZE           512U
#define ELEV_REQ_QUEUE_LEN          8U
#define ELEV_WAIT_QUEUE_LEN         8U

/** Deferred wait entry timeout (ms). */
#ifndef ELEV_WAIT_TIMEOUT_MS
#define ELEV_WAIT_TIMEOUT_MS        15000U
#endif

/** App tick for wait timeout, button debounce, LED blink (ms). */
#ifndef ELEV_APP_TICK_MS
#define ELEV_APP_TICK_MS            50U
#endif

/*******************************************************************************
 ********************************   TYPES   ************************************
 ******************************************************************************/

typedef enum {
  ELEV_REQ_ENTER = 0,
  ELEV_REQ_EXIT,
  ELEV_REQ_STATUS,
  ELEV_REQ_HELP,
  ELEV_REQ_CAPACITY,
  ELEV_REQ_STATS,
  ELEV_REQ_TICK
} elev_req_type_t;

typedef struct {
  elev_req_type_t type;
  uint16_t count;                 /**< ENTER / EXIT / CAPACITY */
} elev_req_t;

typedef struct {
  uint16_t count;
  uint32_t deadline_ticks;        /**< FreeRTOS tick deadline */
} elev_wait_item_t;

#ifdef __cplusplus
}
#endif

#endif /* ELEV_TYPES_H */
