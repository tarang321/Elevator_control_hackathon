/***************************************************************************//**
 * @file elev_parse_util.c
 * @brief Pure command-line parse helpers (host-testable)
 ******************************************************************************/
#include "elev_parse_util.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void elev_parse_skip_ws(char **p)
{
  while ((**p != '\0') && isspace((unsigned char)**p)) {
    (*p)++;
  }
}

bool elev_parse_u16(char **p, uint16_t *out)
{
  char *end = NULL;
  long v;

  elev_parse_skip_ws(p);
  if (**p == '\0') {
    return false;
  }
  if ((**p == '-') || (**p == '+')) {
    if (**p == '-') {
      return false;
    }
    (*p)++;
  }
  if (!isdigit((unsigned char)**p)) {
    return false;
  }
  v = strtol(*p, &end, 10);
  if ((end == *p) || (v <= 0) || (v > 65535)) {
    return false;
  }
  *p = end;
  elev_parse_skip_ws(p);
  if (**p != '\0') {
    return false;
  }
  *out = (uint16_t)v;
  return true;
}

bool elev_parse_line(char *line, elev_req_t *req, char *err, uint16_t err_len)
{
  char *p;

  if ((line == NULL) || (req == NULL)) {
    return false;
  }

  p = line;
  elev_parse_skip_ws(&p);
  if (*p == '\0') {
    return false;
  }

  memset(req, 0, sizeof(*req));

  if ((strncmp(p, "enter", 5) == 0)
      && ((p[5] == '\0') || isspace((unsigned char)p[5]))) {
    p += 5;
    elev_parse_skip_ws(&p);
    if (*p == '\0') {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: missing argument. Usage: enter <count>\r\n");
      }
      return false;
    }
    if (!elev_parse_u16(&p, &req->count)) {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: passenger count must be greater than zero.\r\n");
      }
      return false;
    }
    req->type = ELEV_REQ_ENTER;
    return true;
  }

  if ((strncmp(p, "exit", 4) == 0)
      && ((p[4] == '\0') || isspace((unsigned char)p[4]))) {
    p += 4;
    elev_parse_skip_ws(&p);
    if (*p == '\0') {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: missing argument. Usage: exit <count>\r\n");
      }
      return false;
    }
    if (!elev_parse_u16(&p, &req->count)) {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: passenger count must be greater than zero.\r\n");
      }
      return false;
    }
    req->type = ELEV_REQ_EXIT;
    return true;
  }

  if ((strncmp(p, "capacity", 8) == 0)
      && ((p[8] == '\0') || isspace((unsigned char)p[8]))) {
    p += 8;
    elev_parse_skip_ws(&p);
    if (*p == '\0') {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: missing argument. Usage: capacity <count>\r\n");
      }
      return false;
    }
    if (!elev_parse_u16(&p, &req->count)) {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: capacity must be greater than zero.\r\n");
      }
      return false;
    }
    if (req->count > ELEV_ABS_MAX_CAPACITY) {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: capacity exceeds absolute maximum (%u).\r\n",
                       (unsigned)ELEV_ABS_MAX_CAPACITY);
      }
      return false;
    }
    req->type = ELEV_REQ_CAPACITY;
    return true;
  }

  if ((strncmp(p, "status", 6) == 0)
      && ((p[6] == '\0') || isspace((unsigned char)p[6]))) {
    p += 6;
    elev_parse_skip_ws(&p);
    if (*p != '\0') {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: unknown command. Type 'help' for supported commands.\r\n");
      }
      return false;
    }
    req->type = ELEV_REQ_STATUS;
    return true;
  }

  if ((strncmp(p, "stats", 5) == 0)
      && ((p[5] == '\0') || isspace((unsigned char)p[5]))) {
    p += 5;
    elev_parse_skip_ws(&p);
    if (*p != '\0') {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: unknown command. Type 'help' for supported commands.\r\n");
      }
      return false;
    }
    req->type = ELEV_REQ_STATS;
    return true;
  }

  if ((strncmp(p, "help", 4) == 0)
      && ((p[4] == '\0') || isspace((unsigned char)p[4]))) {
    p += 4;
    elev_parse_skip_ws(&p);
    if (*p != '\0') {
      if ((err != NULL) && (err_len > 0U)) {
        (void)snprintf(err, err_len,
                       "Error: unknown command. Type 'help' for supported commands.\r\n");
      }
      return false;
    }
    req->type = ELEV_REQ_HELP;
    return true;
  }

  if ((err != NULL) && (err_len > 0U)) {
    (void)snprintf(err, err_len,
                   "Error: unknown command. Type 'help' for supported commands.\r\n");
  }
  return false;
}
