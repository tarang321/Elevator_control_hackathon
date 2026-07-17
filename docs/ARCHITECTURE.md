# Elevator Capacity Controller — Architecture Document

**Target:** EFR32xG24B / FreeRTOS (Simplicity SDK)  
**Policy:** Partial board + FIFO wait remainder (never reject a valid enter for capacity)

## Components

| Component | Context | Role |
|-----------|---------|------|
| UART RX ISR | IRQ | Char collect, EOL, echo enqueue, notify parser; backspace + up-arrow recall |
| `parser_task` | Task | Validate → `reqQ` or TX error |
| `elevator_task` | Task | Occupancy, `sem_capacity`, `waitQ`, partial board, wait timeout, stats |
| `tx_task` | Task | Sole LDMA owner; TX ring |
| Soft timers | Timer svc | Periodic status (10 s); BTN0/BTN1 poll (50 ms) |

## FreeRTOS primitives

| Primitive | Symbol | Purpose |
|-----------|--------|---------|
| Task notification | ISR → parser | Command complete |
| Queue | `reqQ` | Parser / button / status timer → elevator |
| Queue | `waitQ` | Remainder counts + deadline ticks |
| Queue | `tx_q` | Echo + responses |
| Counting semaphore | `sem_capacity` | Free seats (init = MAX; resizable) |
| Mutex | `mtx_state` | Occupancy + stats |
| Software timers | status / btn | Periodic status; button debounce poll |

## Capacity algorithm

```
board = min(n, free)
take board seats (non-blocking)
if remain > 0: waitQ.push({remain, now + timeout})
on exit: give seats, then drain waitQ with free seats
on tick: drop waitQ items past deadline
```

## GPIO map — instrumentation (mandatory, BRD4187C)

| Pin | Event | Type |
|-----|-------|------|
| PC00 | RX ISR | Pulse |
| PC01 | Command complete | Pulse |
| PC02 | Parser active | Level |
| PC03 | Elevator active | Level |
| PC04 | Entry blocked (waitQ) | Level |
| PC05 | Sem give | Pulse |
| PC06 | Sem take | Pulse |
| PC07 | LDMA TX active | Level |
| PD02 | LDMA TX complete | Pulse |

## GPIO map — onboard UI (BRD4187C)

| Pin | Role | Type |
|-----|------|------|
| PB02 | LED0 — capacity available | Level (active-high) |
| PB04 | LED1 — full (solid) / deferred (blink ~250 ms) | Level (active-high) |
| PB01 | BTN0 — `exit 1` | Input (active-low) |
| PB03 | BTN1 — `enter 1` | Input (active-low) |

LED rules: LED0 ON when free seats > 0; LED1 solid when full and not waiting; LED1 blink when deferred wait non-empty.

## Console commands

| Command | Action |
|---------|--------|
| `enter <n>` | Board up to free seats; remainder deferred |
| `exit <n>` | Exit passengers; drain waitQ |
| `capacity <n>` | Resize max (1..32); reject if &lt; occupancy+waiting |
| `status` | Occupancy / free / waiting |
| `stats` | Totals entered/exited, deferred, timeouts, max occupancy |
| `help` | Command list |

Also: backspace/DEL edits the line; up-arrow recalls last command.

## VCOM

USART0: PA08 TX, PA09 RX; enable PB00.

## Project slots

| Path | Contents |
|------|----------|
| `elevator/` | All hand-written elevator modules |
| `tests/` | Host-side parser unit test (not linked into firmware) |
| `docs/` | Architecture / design review notes |
| `config/` | Editable SLC configs |
| `autogen/` | Studio-generated — never hand-edit |
| root `app.c` | Thin shim → `elev_app_init()` |
