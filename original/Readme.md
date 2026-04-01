Now I have everything I need for a complete analysis. Let me write it up.

Here is the complete reverse engineering analysis.

---

## `HEAD_FIT.PRG` — Reverse Engineering Analysis

**Author:** H. Diebeck  
**Published:** *INPUT 64* magazine, H. Heise Verlag, Hannover  
**Purpose:** Commodore Datassette VC-1530 tape head azimuth alignment tool

---

### What it does

The program measures the period of the tape signal arriving from the cassette port and plots a real-time dot histogram on the C64's hi-resolution bitmap display. When head azimuth is wrong, crosstalk and frequency response degradation cause the measured period to jitter, producing a wide or displaced trace. The user turns the head azimuth screw while watching the display, aiming for the narrowest, most centered pattern — indicating the best mechanical alignment. The on-screen instructions read exactly:

```
* HEAD FIT *   FOR DATASSETTE VC-1530

1. PUT IN AND RUN THE TAPE
2. DRIVE THE SCREW DEEP
3. DRIVE IT BACK UNTIL THE APPEARING LINES ARE CLEAR
4. F1 – LEAD LINES FOR NORMAL MODE
   F3 – LEAD LINES FOR TURBO MODE
5. PRESS RETURN WHEN ENOUGH
```

---

### Overall architecture

The file is 1786 bytes, load address `$0801`. It has three distinct layers:

**Layer 1 — BASIC stub** (`$0801–$0875`)  
Two BASIC lines. Line 0 is a REM with the title and copyright. Line 1 is:
```basic
SYS(PEEK(44)*256+PEEK(43)+121)
```
`PEEK(43)/PEEK(44)` = TXTTAB = `$0801` (start of BASIC). Adding 121 = `$79` gives the machine code entry point at **`$087A`**.

**Layer 2 — Bootstrap relocator** (`$087A–$089C`)  
This is the small stub entered by `SYS`. It copies 6 pages (1536 bytes) from `$0901` (just after the BASIC header area) to `$8000`, then does `JMP $8000`. After this, the real program runs from a fixed, known address.

```asm
LDX $2C  ; hi byte of TXTTAB = $08
INX       ; +1 → $09
STX $FC   ; source hi = $09 (source = $0901)
LDA $2B   ; lo byte of TXTTAB = $01
STA $FB   ; source lo = $01
LDA #$00 / STA $FD  ; dest lo = $00
LDA #$80 / STA $FE  ; dest hi = $80  (dest = $8000)
LDY #$00
LDX #$06             ; 6 pages
[loop] LDA ($FB),Y / STA ($FD),Y / DEY / BNE / INC $FC / INC $FE / DEX / BNE
JMP $8000
```

**Layer 3 — Main code** (runs at `$8000–$85FF`)  
All logic: hardware init, raster IRQ handler, CIA1 timer measurement, bitmap plotter, dynamic code generator, display mode switching, text output.

---

### Memory map (post-relocation)

| Range | Contents |
|---|---|
| `$0801–$0875` | BASIC stub (unchanged) |
| `$0877–$0900` | Pre-relocation data area (padding, bit tables) |
| `$4800–$48FF` | Dynamically patched IRQ trampoline (`CLI` + `JMP $8xxx`) |
| `$4900–$490F` | Saved system register workspace |
| `$4910–$4911` | Saved IRQ vector backup |
| `$5000–$5FFF` | Dynamically generated bitmap driver — Normal mode |
| `$5800–$5FFF` | Dynamically generated bitmap driver — Turbo mode |
| `$5C00–$5FE7` | VIC color RAM (hires mode, in VIC bank 2) |
| `$6000–$7F3F` | Hires bitmap 320×200 = 8000 bytes |
| `$7400` | Out-of-range indicator byte |
| `$7538` | Status display cell |
| `$8000–$85FF` | Relocated main code |

The VIC-II bank is switched to **bank 2 (`$4000–$7FFF`)** via `CIA2 port A & $FE` (clears bit 0 of `$DD00`). Within that bank, VIC sees the bitmap at its `$2000` offset = CPU `$6000`.

---

### Data tables packed into `$8000`

The first page of relocated code doubles as a compact data area placed before the first `JMP`:

```
$8000:  4C 4C 80     JMP $804C         ; skip the tables
$8003:  72 81        word $8172         ; IRQ handler vector
$8005:  C0 60 30     pixel masks for    ; 11000000
$8006:  18 0C 06 03  pixel cols 0–7    ; 01100000 … 00000001
$800D:  BD 9D F8 7D F9 7D   6 opcode bytes — bitmap ops (normal mode)
$8013:  AD 8D BF 7C F8 7D   6 opcode bytes — bitmap ops (turbo mode)
$8019:  EA EA EA     3× NOP            ; IRQ self-mod patch A
$801C:  4A 66 C3     LSR A / ROR $C3   ; IRQ self-mod patch B
```

The **pixel masks** represent 2-pixel-wide spans across a hires byte:
```
col 0: 11000000  col 1: 01100000  col 2: 00110000  …  col 7: 00000001
```

---

### Subroutines

**`$804C` — Main initialization**  
`SEI`, clear decimal mode, save stack pointer to `$CD`, call save-state (`$81EB`). Disables both CIA interrupt controllers (`$DC0D/$DD0D = $7F`). Loads CIA1 timer A latch with `$8000` (32768 φ2 cycles ≈ 32 ms). Clears bit 5 of `$01` (cassette sense bit — no actual memory map change). Sets border/background colors (dark gray scheme). Calls display init (`$8274`) and IRQ trampoline setup (`$821D`).

---

**`$81EB` / `$8207` — Save / Restore system state**  
`$81EB` saves 16 bytes from `$C0–$CF` (VIC register mirrors and working registers) to `$4900`, and saves the C64 IRQ vector at `$0314/$0315` to `$4910/$4911`, then patches the vector to point at `$8172` (the custom handler). `$8207` reverses this on exit.

---

**`$821D` — Patch IRQ trampoline**  
Writes three literal bytes into the `$4800` page:
- `$58` (`CLI`) → `$4801`  
- `$D0` (`BNE` opcode = high byte of trampoline JMP target) → `$48FE`  
- `$80` → `$48FF`  

This creates a mini stub at `$4801–$48FF`: `CLI / BNE $80D0` which chains into the measurement IRQ. The address patched into `$48FE/$48FF` changes dynamically when the program switches display modes.

---

**`$8248` / `$8261` — Hires mode on/off**  
`$8248` enables bitmap mode: `$D018 = $18` (VIC character generator → bitmap base `$6000`), `$D011 = $3B` (bitmap + 25 rows), `$DD00 & $FE` (select VIC bank 2). `$8261` restores text mode by reversing those writes.

---

**`$8274` / `$82AF` — Dynamic code generator**  
This is the most unusual design element. Rather than writing a fixed bitmap blitter, the program **generates 6502 machine code at runtime** into pages `$5000–$5FFF` (normal mode) and `$5800–$5FFF` (turbo mode).

`$8274` loads one of the two 6-byte opcode tables from `$800D` or `$8013` into `$C5–$CA`, then iterates over every 2-byte bitmap column slot (X = `$50` to `$5F`, step 2 for normal; `$51` to `$60` for turbo) calling `$82AF`.

`$82AF` writes a complete 6502 mini-routine at each slot using indirect-indexed stores (`STA ($C1),Y`). The generated code pattern is approximately:
```asm
LDX #<column_index>      ; A2 nn
LDA $7DF8,X              ; BD F8 7D   ← from $C5-$C7 opcode bytes
...                      ; 30 pairs of: OR pixel, store back
RTS                      ; 60
```
Each generated routine handles one horizontal scan column. They self-modify in a loop using a running address calculation and the SEC/SBC borrow-chain pattern to decrement through 240 pixel rows. The final `JSR $5000` or `JSR $5800` from the main event loop runs whatever routine was generated for the current column.

---

**`$8165` — Raster IRQ dispatcher**  
The raster IRQ fires at line `$20` (line 32). On entry it acknowledges the interrupt (`STA $D019`), then reads the state variable `$CE` and branches:

| `$CE` value | State | Action |
|---|---|---|
| 0 | Measure | Arm CIA1 timer, enable CIA timer interrupt, scan joystick |
| 1 | Display/hold | Freeze display, move raster to line `$FF` |
| 2 | Reset | Clear measurement, re-arm for next cycle |

---

**`$8177` — CIA1 timer measurement (the core)**  
This is how the tape period is actually measured. CIA1 timer A (`$DC04/$DC05`) was pre-loaded with `$8000` and set to **count CNT transitions** (`$DC0E = $99`: one-shot, CNT clock source). The cassette READ signal on the C64 is connected to the CIA1 CNT pin, so each signal edge decrements the timer.

When the IRQ fires, the code reads the remaining count and subtracts:

```asm
SEC
LDA #$19 / SBC $DC04 → STA $C3    ; C3 = elapsed low byte
LDA #$80 / SBC $DC05              ; A  = elapsed high byte
```
This gives a 16-bit elapsed period, `$8019 − timer_value`, in units of one C64 clock cycle (≈ 1 µs).

For reference, a standard C64 tape bit cell at 1200 baud ≈ 833 µs = ~833 counts. At turbo speeds (~3000–6000 Hz), counts fall to ~170–330.

---

**`$81A6` — Period → pixel mapper (with self-modifying code)**  
The 16-bit period result is divided by 4 using two `LSR A / ROR $C3` pairs, scaling it to an 8-bit pixel coordinate:

```asm
CLI
[3 patched bytes at $81A7]   ; either 3× NOP or LSR A / ROR $C3
LSR A
ROR $C3
LDX $C3          ; X = period/4 (8-bit)
```

The **3 bytes at `$81A7` are self-modified** by the code at `$80D4/$80E1`, which copies one of two instruction sequences from the tables at `$8019` or `$801C`:

- Patch A (`EA EA EA` = 3× NOP): used in hold/display mode — no extra division
- Patch B (`4A 66 C3` = `LSR A / ROR $C3`): used in measurement mode — divide by 8 instead of 4, for higher-frequency (turbo) tapes

After dividing, range is checked: if hi-byte A > 1 (or A=1 and X ≥ `$38`), the signal is out of range and a marker byte is written to `$7538`. Otherwise, the pixel is plotted:

```asm
ADC #$74         ; C4 = page ($74xx = within hires bitmap)
STA $C4
TXA / AND #$F8 / STA $C3   ; C3 = byte row (aligned to 8)
TXA / AND #$07 / TAX        ; X  = bit column 0–7
LDA $8005,X                 ; load pixel mask (e.g. $18 for col 3)
ORA ($C3),Y                 ; OR into bitmap row
STA ($C3),Y                 ; write back
```

Each measured period produces exactly one pixel set in the hires bitmap. Over time the display builds up a histogram showing the distribution of tape signal periods. Good azimuth → narrow vertical band. Poor azimuth → wide smear.

---

**`$8021–$804B` — IRQ warm-start prolog (system protection trick)**  
On entry the IRQ saves registers, then does a stack-relative return address patch:
```asm
TSX
LDA $0105,X / CLC / ADC #$02 / STA $0105,X
; optionally: INC $0106,X if carry
```
This increments the IRQ return address by 2, effectively skipping the `SEI + JMP ($A002)` instruction pair that begins the standard C64 IRQ entry. This keeps the program resident and immune to accidental BASIC interpreter re-entry.

If `RESTORE` is pressed (NMI), the code checks a flag at `$C483`. If it matches `$78`, it performs a clean shutdown (`JSR $8207` to restore system state, then `JMP $0862` to fall back into the BASIC line). Otherwise it takes the warm-start vector via `JMP ($A002)`.

---

**`$8316` — Frequency range lookup table**  
7 two-byte entries pointing to sub-tables in `$8322–$833E`. These are indexed by counting leading zeros in `$C0` (the current best-estimate period accumulator) using a 6-bit barrel-roll loop:

```asm
LDA $C0 / LDX #$06
:loop  DEX / ROL A / BCS loop   ; count shifts until MSB appears
TXA / ASL / TAX                 ; X = leading-zero count × 2
LDA $8316,X / STA $C1           ; load address of sub-table
```
This is a fast integer `floor(log2($C0))` used to choose which horizontal region of the bitmap to update for the current frequency range.

---

### Design patterns summary

**Self-modifying code** is used in two places: the 3-byte patch at `$81A7` switches between two division depths depending on tape speed mode; the byte at `$80CA` is patched during display driver generation to embed a bitmap page address directly into a load instruction operand.

**Dynamic code generation** is the most distinctive feature. The hires bitmap driver is not written as fixed code but constructed at init from opcode byte tables, adapting to normal-speed vs. turbo-speed tape addressing. The generated routines live in pages `$5000` and `$5800` and are executed by `JSR`.

**IRQ vector hijacking with full save/restore** is cleanly done: the kernal's `$0314/$0315` vector is saved to `$4910/$4911`, replaced with the custom handler, and restored on clean exit. The trampoline at `$4800` mediates between the old and new handlers.

**Three-state IRQ state machine** (variable `$CE` = 0/1/2) drives the entire real-time loop without any busy-wait in the main thread. The "main thread" after init is simply an infinite `JMP $8136` or `JMP $813C` — all work happens in the IRQ.

**CIA1 timer as period meter**: using the CNT clock source to count tape signal edges rather than φ2 cycles gives direct period measurement tied to the tape signal itself, without needing a hardware comparator or edge-detection interrupt.
