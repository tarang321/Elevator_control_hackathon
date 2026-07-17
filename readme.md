# Elevator Capacity Controller

UART console elevator capacity demo on EFR32xG24 (BRD4187C) with FreeRTOS.

## Layout (easy to navigate)

```text
Elevator_capacity_controller/
  main.c, app.c, app.h     # Studio / sl_main hooks (root)
  elevator/                # Application modules (hand-written)
    elev_app.*             # Bring-up
    elev_rx.*              # UART RX ISR + ping-pong
    elev_parser.*          # Parser task
    elev_ctrl.*            # Capacity + wait queue
    elev_tx.*              # LDMA TX task
    elev_gpio.*            # Analyzer GPIOs
    elev_types.h           # Shared types
  docs/ARCHITECTURE.md     # Design document
  config/                  # Hand-edit configs (pins, FreeRTOS, VCOM)
  autogen/                 # Generated only — do not edit
```

## Features

- Character-by-character UART RX interrupt (VCOM USART0)
- Parser / elevator / TX FreeRTOS tasks
- Counting semaphore = free seats; mutex = occupancy
- Partial boarding + FIFO wait queue
- LDMA UART TX for echo and responses
- GPIO instrumentation for logic analyzer

## Setup

1. Build and flash with Simplicity Studio (or cmake_gcc).
2. Open kit VCOM at **115200 8N1**.
3. Commands: `enter <n>`, `exit <n>`, `status`, `help`.
