# Contributing to Precise RTD Simulator

## Local development

1. Open `Firmware/PrecRtdSim-APP` (CubeIDE project `PrecRtdSim-APP`) in STM32CubeIDE. Use the `Debug`
   configuration: it is the `-Os` build with the post-build step `../BinBuild.exe 6 ${ProjName}.bin <FW version>`
   (`Release` is `-Og` and has no such step). The APP region is 28 kB (28672 B): check the size after each build.
2. Register map: edit `Documents/PrecRtdSim.xlsm` (sheet `Registers`), then regenerate with
   `pwsh Documents\generate_regmap.ps1` or with the workbook's sheet buttons (they need macros and ActiveX
   allowed). The script runs the workbook's generator functions through Excel COM, opens the workbook read-only,
   and copies `reg_map.*` to `Core/Common`, `mb_rtu_app.*` to `Core/Serial` and the JSON / Python maps to
   `Tests/prec-rtd-sim`. Add new registers at the end of their block to keep existing Modbus addresses.
   Datasheet tables: `python Tests/prec-rtd-sim/UpdateDatasheet.py`. More:
   [docs/10-build-release-update.md](docs/10-build-release-update.md).
3. Headless build (STM32CubeIDE 1.19.0, run from the IDE install folder):

       stm32cubeidec.exe -application org.eclipse.cdt.managedbuilder.core.headlessbuild
           -data <scratch workspace> -import file:/<repo>/Firmware/PrecRtdSim-APP
           -build PrecRtdSim-APP/Debug

   Set `LOCALAPPDATA` to a folder you control before the launch and, from Git Bash, `unset
   NoDefaultCurrentDirectoryInExePath`. The output is `Debug/PrecRtdSim-APP-<yyyymmdd>-<version>.bin` (the folder
   is git-ignored).
4. Update a device over Modbus with the tools repo `jac-sw-dhw21x-tools`:
   `StandRemoteTool/update_tool_fw.py --check|--flash RtdEmulPrec=<bin>` on the stand (stop the listener first).
   The device reboots into its power-up state afterwards; verify `FIRM_REVISION` and `FIRM_ASSEMBLY_INFO`.
   Ask before updating a stand device. A blank device is programmed with the ST-Link as described in
   [README.md](README.md) ("Programming bootloader").
5. Release: raise the post-build version argument in `.cproject`, build, copy the binary to `Binary/`, point
   `Binary/STLink-program.bat` at it (keep its CRLF), update and verify the stand devices, then tag the release
   commit `FW/<N>`.

Python tools in `Tests/prec-rtd-sim/` (Python 3.7 or newer): `pip install -r requirements.txt`, edit
`ComSettings.json` (COM port), run `python VisualRtdSim.py`.

## Testing

There are no host unit tests. After an update, check `FIRM_*` on the device and run the stand test TC023 test013
from `jac-sw-dhw21x-tools` (used to verify FW 2).

## Commits and releases

The default branch is `main`. Release commits are tagged `FW/<N>` like the other tool FW repositories.

## Set up repository skills

The shared Jablotron skill set is pinned in `skills-lock.json` (v0.2.0). After cloning, install it for your
agent from the repository root (needs Node.js 22.20 or newer):

    DISABLE_TELEMETRY=1 npx skills add git@gitlab.jablotron.cloud:ai/jablotron-skills.git#v0.2.0 --skill '*' -a claude-code codex cursor -y

The install writes `.agents/skills/` and `.claude/skills/` (git-ignored). Do not install with `-g`: a global
copy overrides the project pin in Claude Code. Open a new agent session in the repository; its skill list shows
`ja-repo-setup`, `to-spec`, `implement` and the rest. Run `/ja-repo-setup` after an upgrade of the skill set.
