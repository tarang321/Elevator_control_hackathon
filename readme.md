# Elevator Capacity Controller

UART console elevator capacity demo on EFR32xG24 (BRD4187C) with FreeRTOS.

## Layout (easy to navigate)

```text
Elevator_capacity_controller/
  main.c, app.c, app.h     # Studio / sl_main hooks (root)
  elevator/                # Application modules (hand-written)
    elev_app.*             # Bring-up (tasks, btn timer, banner)
    elev_rx.*              # UART RX ISR + ping-pong cmd slots
    elev_parser.*          # Parser task → reqQ
    elev_ctrl.*            # Capacity + wait queue + status-on-update
    elev_tx.*              # TX byte ring + LDMA (no TX task)
    elev_gpio.*            # Analyzer GPIOs + LEDs/buttons
    elev_types.h           # Shared types / limits
  docs/ARCHITECTURE.md     # Design + flow graphs
  config/                  # Hand-edit configs (pins, FreeRTOS, VCOM)
  autogen/                 # Generated only — do not edit
```

## Features

- Character-by-character UART RX interrupt (VCOM USART0)
- Parser + elevator FreeRTOS tasks (no dedicated TX task)
- TX byte ring (512 B) drained by LDMA; DMA-done kick deferred via timer pend
- Counting semaphore = free seats; mutex = occupancy
- Partial boarding + FIFO wait queue
- Status printed **once** after button/serial enter/exit/capacity (or on `status`) — no 10 s spam
- GPIO instrumentation for logic analyzer

## Runtime flow (summary)

```text
BTN / UART cmd ──► reqQ ──► elev_ctrl ──► event print + status once
                                              │
USART RX ISR ──echo──► TX ring ──LDMA──► VCOM
                 ▲
                 └── elev_ctrl / parser errors also write here
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for mermaid diagrams, GPIO maps, and primitives.

## Setup

1. Build and flash with Simplicity Studio (or `cmake_gcc`).
2. Open kit VCOM at **115200 8N1**.
3. Commands: `enter <n>`, `exit <n>`, `capacity <n>`, `status`, `stats`, `help`.
4. BTN0 = exit 1, BTN1 = enter 1.
