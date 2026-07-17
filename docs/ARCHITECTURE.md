# Elevator Capacity Controller — Architecture Document

**Target:** EFR32xG24B / FreeRTOS (Simplicity SDK)  
**Policy:** Partial board + FIFO wait remainder (never reject a valid enter for capacity)

## Components

| Component | Context | Role |
|-----------|---------|------|
| UART RX ISR | IRQ | Char collect, EOL, echo enqueue, notify parser |
| `parser_task` | Task | Validate → `reqQ` or TX error |
| `elevator_task` | Task | Occupancy, `sem_capacity`, `waitQ`, partial board |
| `tx_task` | Task | Sole LDMA owner; TX ring |

## FreeRTOS primitives

| Primitive | Symbol | Purpose |
|-----------|--------|---------|
| Task notification | ISR → parser | Command complete |
| Queue | `reqQ` | Parser → elevator |
| Queue | `waitQ` | Remainder counts |
| Queue | `tx_q` | Echo + responses |
| Counting semaphore | `sem_capacity` | Free seats (init = MAX) |
| Mutex | `mtx_state` | Occupancy |

## Capacity algorithm

```
board = min(n, free)
take board seats (non-blocking)
if remain > 0: waitQ.push(remain)
on exit: give seats, then drain waitQ with free seats
```

## GPIO map (BRD4187C placeholders)

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

## VCOM

USART0: PA08 TX, PA09 RX; enable PB00.

## Project slots

| Path | Contents |
|------|----------|
| `elevator/` | All hand-written elevator modules |
| `docs/` | Architecture / design review notes |
| `config/` | Editable SLC configs |
| `autogen/` | Studio-generated — never hand-edit |
| root `app.c` | Thin shim → `elev_app_init()` |
