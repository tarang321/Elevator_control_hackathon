/***************************************************************************//**
 * @file elev_parse_util.h
 * @brief Pure command-line parse helpers (host-testable)
 ******************************************************************************/
#ifndef ELEV_PARSE_UTIL_H
#define ELEV_PARSE_UTIL_H

#include <stdint.h>
#include <stdbool.h>

#include "elev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void elev_parse_skip_ws(char **p);
bool elev_parse_u16(char **p, uint16_t *out);

/**
 * @brief Classify a command line into a request (no queue I/O).
 * @param line  NUL-terminated command (mutated by tokenization).
 * @param req   Filled on success / known command shape.
 * @param err   Optional buffer for error text (may be NULL).
 * @param err_len Size of err.
 * @return true if req should be queued; false if error (err filled) or empty.
 */
bool elev_parse_line(char *line, elev_req_t *req, char *err, uint16_t err_len);

#ifdef __cplusplus
}
#endif

#endif /* ELEV_PARSE_UTIL_H */
