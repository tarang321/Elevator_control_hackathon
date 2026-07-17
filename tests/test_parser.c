/***************************************************************************//**
 * @file test_parser.c
 * @brief Host-side unit tests for elev_parse_util (not part of firmware image)
 *
 * Build (from repo root, with a host gcc):
 *   gcc -I elevator -o tests/test_parser.exe tests/test_parser.c elevator/elev_parse_util.c
 *   ./tests/test_parser.exe
 ******************************************************************************/
#include "elev_parse_util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_fail;

static void expect_true(const char *name, bool ok)
{
  if (!ok) {
    printf("FAIL: %s\n", name);
    g_fail++;
  } else {
    printf("PASS: %s\n", name);
  }
}

static void test_enter_ok(void)
{
  char line[] = "enter 3";
  elev_req_t req;
  char err[96];
  err[0] = '\0';
  expect_true("enter 3 queues", elev_parse_line(line, &req, err, sizeof(err)));
  expect_true("enter type", req.type == ELEV_REQ_ENTER && req.count == 3);
}

static void test_enter_zero(void)
{
  char line[] = "enter 0";
  elev_req_t req;
  char err[96];
  err[0] = '\0';
  expect_true("enter 0 rejects", !elev_parse_line(line, &req, err, sizeof(err)));
  expect_true("enter 0 err", strstr(err, "greater than zero") != NULL);
}

static void test_exit_missing(void)
{
  char line[] = "exit";
  elev_req_t req;
  char err[96];
  err[0] = '\0';
  expect_true("exit missing rejects", !elev_parse_line(line, &req, err, sizeof(err)));
}

static void test_capacity(void)
{
  char line[] = "capacity 10";
  elev_req_t req;
  char err[96];
  err[0] = '\0';
  expect_true("capacity 10", elev_parse_line(line, &req, err, sizeof(err)));
  expect_true("capacity type", req.type == ELEV_REQ_CAPACITY && req.count == 10);
}

static void test_stats_help(void)
{
  char line1[] = "stats";
  char line2[] = "help";
  elev_req_t req;
  char err[96];
  err[0] = '\0';
  expect_true("stats", elev_parse_line(line1, &req, err, sizeof(err))
              && req.type == ELEV_REQ_STATS);
  expect_true("help", elev_parse_line(line2, &req, err, sizeof(err))
              && req.type == ELEV_REQ_HELP);
}

static void test_unknown(void)
{
  char line[] = "unknown 5";
  elev_req_t req;
  char err[96];
  err[0] = '\0';
  expect_true("unknown rejects", !elev_parse_line(line, &req, err, sizeof(err)));
}

int main(void)
{
  g_fail = 0;
  test_enter_ok();
  test_enter_zero();
  test_exit_missing();
  test_capacity();
  test_stats_help();
  test_unknown();
  if (g_fail != 0) {
    printf("%d failure(s)\n", g_fail);
    return 1;
  }
  printf("All parser tests passed.\n");
  return 0;
}
