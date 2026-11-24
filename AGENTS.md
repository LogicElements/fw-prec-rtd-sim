# Local skills

- When creating a repository-local skill, place its source in `.jablotron/skills/<skill-name>/SKILL.md`.
- When writing or editing a skill or this file, use the `writing-for-agents` skill.

# Agent rules

- Do not commit or push unless the developer explicitly asks. Leave changes in the working tree.
- Ask before updating a stand device with new firmware: the test stands are shared and switched.
- The register map is the workbook `Documents/PrecRtdSim.xlsm`. Change it there and regenerate; never hand-edit
  generated files. Hand-written code goes into the templates `Documents/txts/*_temp.*`.
- `Tests/prec-rtd-sim/ComSettings.json` and `Tests/prec-rtd-sim/VisualSettings.json` hold local settings; they are
  not part of a change.
- Never block the main loop (`HAL_Delay`, `HAL_MAX_DELAY`): Modbus replies are sent only from it.

# Context

- Read [README.md](README.md) for the repository contents, the register-map generator, bootloader programming and
  the Python tools.
- Before changing or reviewing code, read [CODING_STANDARDS.md](CODING_STANDARDS.md) for code style and
  implementation constraints.
- Before building, regenerating the register map, releasing or updating a device locally, read
  [CONTRIBUTING.md](CONTRIBUTING.md).
- When interpreting requirements or naming concepts, read [GLOSSARY.md](GLOSSARY.md) for domain language.
- Before working in unfamiliar code, read [docs/index.md](docs/index.md), starting with `00`.
- Before creating or reading Specs and Tickets, read [.jablotron/issue-tracker.md](.jablotron/issue-tracker.md)
  for the issue tracker and how to use it.
- The stand-side driver is in `jac-sw-dhw21x-tools`: `VeriFrame/Tools/RTDEmul/Prec` and
  `StandRemoteTool/StandRemoteTools/StandRemoteRtdEmulPrec.py`. Read it when a change affects registers the
  stand tools use.
