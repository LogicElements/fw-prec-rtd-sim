# Documentation index - le-fw-prec-rtd-sim

Detailed documentation of **le-fw-prec-rtd-sim** (Prec RTD simulator FW) for developers and AI agents.
Start with `00`, then the doc for your task.

| Doc | Read it when… | Quick debrief |
|-----|---------------|---------------|
| [`00-repository-overview.md`](00-repository-overview.md) | **Start here.** Any work in this repo. | What the device is, layout, FW structure (main loop, RTD handling, expanders), channel modes, register map, gotchas. |
| [`10-build-release-update.md`](10-build-release-update.md) | Building, generating the register map, releasing a binary, updating a device over Modbus. | Headless Excel generator + CubeIDE build, BinBuild version block, bootloader checks (device id, CRC), update type 2051 / mode 1. |

## Naming

`index.md` + `NN-kebab-name.md`: `00` overview, `10`-`80` topics, `90`+ reserved. Every doc is listed here.
