# Precise RTD Simulator

Firmware and tools of the Precise RTD Simulator ("Prec", "RTD Emulator (new)"): one Modbus device that emulates resistive sensors, used by the DHW-21x test stands to emulate NTC sensors.

## Language

**Channel**:
One emulated sensor output. Channel 0 is on the main board; channels 1-4 are on the first expansion board and 5-8 on the second. On the DHW-21x stands channels 1-8 are DCW, DHW, DHI, DHO, HR, HS, OUT, UFH.

**Expansion board**:
Board with two PCA9505 expanders on I2C. Each expander drives channel A and B through 6 decade switches (18 bits). Stands use board addresses 0/1/2/3 (DIP 1/2/3/4).

**Channel mode (`RTD_CHANNEL_MODE`)**:
FW 2 setting. 0 = selected channel: `RTD_TEMPERATURE` / `RTD_RESISTANCE` go to the channel in `RTD_CHANNEL_SELECT`. 1 = per channel: every channel follows its own `RTD_TEMPERATURE_CHx`. Default 0 after every power-up.

**Mode (resistance mode)**:
How a value is turned into resistance: direct resistance, NTC (beta) or PT. Power-up state is mode 0, direct resistance.

**Register map workbook**:
`Documents/PrecRtdSim.xlsm`: the source of the register map and internal storage, with a VBA generator that writes the firmware, Python and JSON map files.

**Device id**:
9153. The bootloader accepts an image only when word 8 of its info block (`CONF_TARGET_DEVICE`) equals it.

**FW info block**:
Block 512 B into the image. `BinBuild.exe` writes `FIRM_REVISION`, `FIRM_ASSEMBLY_INFO`, CRC and size into words 0-3; the APP copies words 0-1 into the `FIRM_*` registers at boot.
