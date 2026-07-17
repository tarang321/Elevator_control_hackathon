# Elevator Capacity Controller

UART console elevator capacity demo on EFR32xG24 (BRD4187C) with FreeRTOS.

## Features

- Character-by-character UART RX interrupt (VCOM USART0)
- Parser / elevator / TX FreeRTOS tasks
- Counting semaphore = free seats; mutex = occupancy
- Partial boarding + FIFO wait queue (no capacity reject)
- LDMA UART TX for echo and responses
- GPIO instrumentation for logic analyzer

See [ARCHITECTURE.md](ARCHITECTURE.md) for algorithms and GPIO map.

## Setup

1. Build and flash with Simplicity Studio.
2. Open the kit VCOM serial port at **115200 8N1**.
3. Commands: `enter <n>`, `exit <n>`, `status`, `help`.

## Example

```
Max capacity: 5
> enter 3
> enter 5
  (boards 2, waits 3)
> exit 1
  (admits 1 from wait queue)
```
