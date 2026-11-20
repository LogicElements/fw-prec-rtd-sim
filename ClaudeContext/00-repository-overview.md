# 00 - Repository overview

## Resume here

- **State 2026-11-20:** FW 2 written, built (`Binary/PrecRtdSim-APP-20261120-2.bin`, 28424 B) and flashed over Modbus
  into the dhw-ng-maa emulator (verified: `FIRM_REVISION` 2, `FIRM_ASSEMBLY_INFO` 20261120, writes answered in 32 ms
  instead of ~2.2 s, TC023 test013 8/8 per channel twice, board re-init keeps 25.0 C).
- **Committed locally** (4 commits on `main` + annotated tag `FW/2`, 2026-11-20), **not pushed**: the remote
  `LogicElements/fw-prec-rtd-sim` (GitHub) refuses Ondřej's account (no write access). Push `main` + `FW/2` once
  access is granted, or move the repo to a Jablotron remote (Ondřej's decision).
- **Open:** other stands with a Prec emulator need the same update (le and luo have the old LE emulators).

## What the device is

Modbus RTU slave (19200 8E1, address 32 + DIP switches) emulating resistive sensors on 9 channels: channel 0 = main
board (MAX7300 expander), 1-4 = 1. expansion board, 5-8 = 2. expansion board (two PCA9505 expanders per board, each
expander drives channel A and B through 6 decade switches = 18 bits). Modes: direct resistance, NTC (beta), PT.
On the DHW-21x stands channels 1-8 = DCW, DHW, DHI, DHO, HR, HS, OUT, UFH (tools `RtdPrecRegHandler`).

Power-up state is not usable for the stand: mode 0 (direct resistance), expansion-board addresses 0/1/2/3 (stands
have DIP 1/2/3/4) - the stand tools / boot script initialise it.

## Layout

| Path | What |
|------|------|
| `Firmware/PrecRtdSim-APP` | Application (CubeIDE project `PrecRtdSim-APP`, `Debug` config = the release build) |
| `Firmware/PrecRtdSim-BTL` | Bootloader (installs a Modbus-uploaded image from the buffer area) |
| `Binary/` | Released binaries + `STLink-program.bat` (blank device: BTL + APP + serial number) |
| `Documents/PrecRtdSim.xlsm` | Register map workbook + LE VBA generator; `Documents/txts/*_temp.*` = templates |
| `Tests/prec-rtd-sim` | Generated JSON / Python map, VisualRtdSim GUI, `UpgradeSettings.json` |
| `Tests*.json`, `TestsPrecRtdSimRegs.py` (root) | Stray generator outputs (the workbook's "Tests destination" path misses a `\`) |

## FW structure

- `main.c` loop: `MbSlave_Handle()` every pass (Modbus frames are received by IRQ, **the reply is sent from here**),
  every 50 ms the low-priority slot: `Control_Handle`, flash handling, IWDG reload (3 s), `RTD_Handle()`.
- `RTD_Handle.c`: change tracking of `conf.rtd.*`; applies channels through `RTD_lib.c`
  (`RTD_SetChannelTemperature` -> `set_switch_rezistor(channel, ohm)` -> MAX7300 / `PCA_WriteCmd18`).
- **Channel modes (FW 2, `RTD_CHANNEL_MODE`):** 0 selected channel (FW 1 behaviour: `RTD_TEMPERATURE` /
  `RTD_RESISTANCE` go to the channel of `RTD_CHANNEL_SELECT`; selecting a channel applies the current value to it at
  once) - 1 per channel (NTC / PT: every channel 0-8 follows its own `RTD_TEMPERATURE_CHx`, only changed ones are
  applied; a change of mode, NTC / PT constants, channel mode or an expander re-init applies all). Slew rate and
  direct resistance work on the `RTD_CHANNEL_SELECT` channel in both modes (per channel: the others keep their own
  values). Default 0 after every power-up (RTD registers are not flash-stored).
- `PCA9505.c`: expander init (resets all outputs = every switch 0, i.e. near short on channels 1-8 - hence the
  re-apply after `RTD_EXP_BOARD_INIT`), batched port writes, channel LED flash (non-blocking, 500 ms), 10 ms I2C
  timeout + I2C master reset (PE toggle) after a timeout.

## Gotchas / decisions

- **The "Modbus stuck" of FW 1 (~2.2 s no answer after a write):** `PCA_BlinkLed` did `HAL_Delay(500)` per channel
  apply in the main loop, plus 36 I2C transactions per channel with `HAL_MAX_DELAY`; the master timed out, retried, and
  the late reply collided with the retry. FW 2: non-blocking LED, at most 3 port reads + 3 writes per channel.
- **Device id:** the bootloader compares word 8 of the image info block with its own device id (9153). FW 1 had
  `CONF_TARGET_DEVICE` 2051 (the old LE RTD emulator's id) -> every Modbus update was refused and the old FW kept
  running (unnoticed: FW 1 reported a hard-coded `FIRM_REVISION` 1 / 20250330). FW 2: 9153, and `FIRM_*` come from
  the info block (a debugger-flashed `.elf` reads 0xFFFFFFFF there).
- `SYS_REGMAP_VERSION` stays the FW's factory value (`Config_ReadStream` skips it; the stored value only has to match
  its major part) - before, a stored 1001 overwrote the new version.
- The MAX7300 (channel 0) path still uses `HAL_MAX_DELAY` - not used on the stands.
- The `FACT_*` registers are hard-coded (`configuration.h`), not taken from the bootloader block.
- Several student comments are Czech; keep them when touching nearby code.
- `Tests/prec-rtd-sim/ComSettings.json` (COM port) and `VisualSettings.json` (window, slave address) are tracked but
  hold local settings - Ondřej's clone has them `git update-index --skip-worktree` (local only, not in the repo), so
  git does not show them. A pull that changes them stops with "would be overwritten": `git update-index
  --no-skip-worktree <files>`, stash, pull, pop, skip again.
