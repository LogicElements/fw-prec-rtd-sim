# Coding standards

Scope: C firmware in `Firmware/PrecRtdSim-APP` and `Firmware/PrecRtdSim-BTL`. Build facts are in [docs/10-build-release-update.md](docs/10-build-release-update.md).

## Conventions and preferences

- Match the surrounding file. Use spaces, never tabs (convert a tab file you edit). Keep comments short, giving only the why.
- Several student comments are Czech; keep them when touching nearby code.

## Implementation constraints

- The target is a Cortex-M0 (STM32F072C8): no unaligned access.
- Never block the main loop (`HAL_Delay`, `HAL_MAX_DELAY`): Modbus replies are sent only from it.
- The APP region is 28 kB (28672 B): check the size after each build.
- The device id is 9153 in the datasheet, the bootloader, `FACT_DEVICE_ID` and `CONF_TARGET_DEVICE`; keep them equal. FW 1 had 2051 in the APP and the bootloader refused every Modbus update ([docs/00-repository-overview.md](docs/00-repository-overview.md)).
