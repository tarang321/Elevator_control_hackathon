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

#define ELEV_CMD_BUF_LEN            64U
#define ELEV_TX_MSG_MAX             160U
#define ELEV_REQ_QUEUE_LEN          8U
#define ELEV_WAIT_QUEUE_LEN         8U
#define ELEV_TX_QUEUE_LEN           24U

/*******************************************************************************
 ********************************   TYPES   ************************************
 ******************************************************************************/

typedef enum {
  ELEV_REQ_ENTER = 0,
  ELEV_REQ_EXIT,
  ELEV_REQ_STATUS,
  ELEV_REQ_HELP
} elev_req_type_t;

typedef struct {
  elev_req_type_t type;
  uint16_t count;                 /**< Meaningful for ENTER / EXIT */
} elev_req_t;

typedef struct {
  uint16_t len;
  uint8_t data[ELEV_TX_MSG_MAX];
} elev_tx_msg_t;

#ifdef __cplusplus
}
#endif

#endif /* ELEV_TYPES_H */
