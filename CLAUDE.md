# CLAUDE.md - le-fw-prec-rtd-sim

Precise RTD Simulator ("Prec", "RTD Emulator (new)") FW - an LE student project the DHW-21x test stands use to emulate
the NTC sensors of the DHW-21x (one Modbus device, 8 channels on two I2C expansion boards + channel 0 on the main
board). STM32F072C8 (Cortex-M0: no unaligned access), STM32CubeIDE. Stand-side driver: `jac-sw-dhw21x-tools`
`VeriFrame/Tools/RTDEmul/Prec` + `StandRemoteTool/StandRemoteTools/StandRemoteRtdEmulPrec.py`.

Detailed context: [`ClaudeContext/index.md`](ClaudeContext/index.md) - read `00-repository-overview.md` first.

## Essentials

- **Register map = the workbook `Documents/PrecRtdSim.xlsm`** - change it there, then `pwsh Documents\generate_regmap.ps1`
  or its sheet buttons (ActiveX - need ActiveX allowed); never hand-edit generated files - hand code goes into
  the templates `Documents/txts/*_temp.*`. Datasheet tables: `Tests/prec-rtd-sim/UpdateDatasheet.py`.
- **Device id is 9153** (datasheet, bootloader, `FACT_DEVICE_ID`, `CONF_TARGET_DEVICE`). FW 1 had 2051 in the APP and
  the bootloader refused every Modbus update - keep them equal.
- **Never block the main loop** (`HAL_Delay`, `HAL_MAX_DELAY`): Modbus replies only come from it.
- APP region is 28 kB - check the size after each build (FW 2: 28424 B).
- Build = `Debug` config (it is the `-Os` one with the BinBuild version step); the post-build argument is the FW
  version.

## Code style

Match the surrounding file; spaces, never tabs (convert a tab file you edit); comments short, the why only.

## Git

Don't commit or push unless Ondřej asks. Branch `main`. `Tests/prec-rtd-sim/ComSettings.json` and
`VisualSettings.json` hold local settings - not part of changes.
