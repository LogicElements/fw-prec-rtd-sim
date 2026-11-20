# 10 - Build, register map, release, device update

## Register map (workbook generator, headless)

`Documents/PrecRtdSim.xlsm` (sheet `Registers`, rows from 18; `Common` = block / type / access codes). Its sheet
buttons (ActiveX) call VBA functions that write into `Documents/txts/` and copy to the destinations F8 (register map),
F9 (Modbus), F10 (Tests) - all three relative to the workbook since 2026-11-20 (F9 / F10 held the original author's
`C:\Users\evzen\...` paths: "Run-time error 76 Path not found"). The buttons need ActiveX allowed (Trust Center:
"Prompt me before enabling UFI controls ... SFI controls with minimal restrictions") and the macros enabled.
Without the buttons: **`pwsh Documents\generate_regmap.ps1 [-Tools <jac-sw-dhw21x-tools>]`** - the same VBA functions through
Excel COM (`CreateDefineFile`, `CreateStorageFile`, `CreateFactoryFile`, `RegMap_Generate`, `CreateJsonFileModbus`,
`CreatePythonClass`, `CreateJsonFileCSharp`), then copies `reg_map.*` -> `Core/Common`, `mb_rtu_app.*` ->
`Core/Serial`, the JSON / Python maps -> `Tests/prec-rtd-sim` (and with `-Tools` the Modbus JSON to the tools repo;
its `RtdEmulPrecRegs.py` has a hand-added `RTD_EXP_CHANNEL` class - merge the generated class by hand). It opens the
workbook read-only; edit rows in Excel first. Verified 2026-11-20: reproduces the tree byte for byte.
- Datasheet: `python Tests/prec-rtd-sim/UpdateDatasheet.py` (lecore) rewrites the two register tables of
  `Documents/PrecRtdSim-datasheet.docx` from the Modbus JSON; text (use cases, revisions) by hand.
- Rows: new registers at the end of their block keep every existing Modbus address. FLOAT = one Modbus register x10
  unless `F32` in "Modbus special". Numbers written into cells via PowerShell 7 COM need `[double]`.
- Hand code belongs in the templates (`Documents/txts/*_temp.*`), e.g. the hard-coded FACT block of `reg_map.c`.

## Build (headless STM32CubeIDE 1.19.0)

Work-folder script `build_prec.sh [Debug|Release] [build|clean]`: `stm32cubeidec.exe -application
org.eclipse.cdt.managedbuilder.core.headlessbuild -data <scratch ws> -import file:/<repo>/Firmware/PrecRtdSim-APP
-build PrecRtdSim-APP/Debug`, `LOCALAPPDATA` in the work folder, `unset NoDefaultCurrentDirectoryInExePath`.
`Debug` is `-Os` and has the post-build step `../BinBuild.exe 6 ${ProjName}.bin <FW version>` (`Release` is `-Og`
and has none). Output `Debug/PrecRtdSim-APP-<yyyymmdd>-<version>.bin` (folder git-ignored). The only linker warning
(RWX LOAD segment) is old. APP limit 28672 B.

## FW info block (image offset 512)

BinBuild writes `FIRM_REVISION`, `FIRM_ASSEMBLY_INFO` (build date), CRC, size into words 0-3; word 8 is
`CONF_TARGET_DEVICE` from the source. The bootloader (`Btl_VerifyImage`) accepts an image when size <= max, CRC
(STM32 CRC32 over [0..512), 8 words 0xFF, word 8..end) matches and **word 8 == its own device id (9153)**. The APP
copies words 0-1 into `FIRM_*` at boot.

## Release

1. Raise the post-build version argument in `.cproject` (FW 2 -> next 3).
2. Build, copy `Debug/PrecRtdSim-APP-<date>-<version>.bin` to `Binary/`, point `Binary/STLink-program.bat` at it
   (keep its CRLF).
3. Update the stand devices (ask Ondřej first) and verify (`FIRM_*`, TC023 test013).
4. Commit when asked; tag `FW/<N>` like the other tool FW repos.

## Device update over Modbus (stand)

Tools repo `StandRemoteTool/update_tool_fw.py --check|--flash RtdEmulPrec=/tmp/<bin>` on the stand (= the Stand
Remote Tool's "Update FW" code; stop the listener first). `StandRemoteRtdEmulPrec`: `FW_BINARY_TYPE = 2051` (FW 1
accepts only that; FW 2 also 9153), `FW_MODE = 1` (the APP's page callback only programs; the 28 kB erase fits the
3 s IWDG). After the update the device reboots into its power-up state (mode 0) - the tests / tools re-initialise it.
FW 1 -> FW 2 on dhw-ng-maa 2026-11-20: done and verified.
