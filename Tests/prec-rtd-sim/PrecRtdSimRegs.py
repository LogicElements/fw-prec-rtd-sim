﻿class PrecRtdSimRegs:
    SYS_UPTIME = "SYS_UPTIME"
    SYS_REGMAP_VERSION = "SYS_REGMAP_VERSION"
    SYS_STATUS = "SYS_STATUS"
    SYS_COMMAND = "SYS_COMMAND"
    SYS_TEST = "SYS_TEST"
    SYS_IO_INPUT = "SYS_IO_INPUT"
    FACT_SERIAL_NUMBER = "FACT_SERIAL_NUMBER"
    FACT_DEVICE_ID = "FACT_DEVICE_ID"
    FACT_HW_REVISION = "FACT_HW_REVISION"
    FACT_BOOT_REVISION = "FACT_BOOT_REVISION"
    FIRM_REVISION = "FIRM_REVISION"
    FIRM_ASSEMBLY_INFO = "FIRM_ASSEMBLY_INFO"
    FIRM_APP_CHECKSUM = "FIRM_APP_CHECKSUM"
    FIRM_APP_SIZE = "FIRM_APP_SIZE"
    COM_MB_BAUD_RATE = "COM_MB_BAUD_RATE"
    COM_MB_PARITY = "COM_MB_PARITY"
    COM_MB_STOP_BITS = "COM_MB_STOP_BITS"
    COM_RESERVED = "COM_RESERVED"
    COM_MB_ADDRESS = "COM_MB_ADDRESS"
    COM_MB_APPLY = "COM_MB_APPLY"
    COM_MB_TIMEOUT = "COM_MB_TIMEOUT"
    RTD_MODE = "RTD_MODE"
    RTD_TEMP_CALIB = "RTD_TEMP_CALIB"
    RTD_NTC_BETA = "RTD_NTC_BETA"
    RTD_NTC_STOCK_RES = "RTD_NTC_STOCK_RES"
    RTD_PT_STOCK_RES = "RTD_PT_STOCK_RES"
    RTD_RESISTANCE = "RTD_RESISTANCE"
    RTD_TEMPERATURE = "RTD_TEMPERATURE"
    DBG_WRITES_CONF = "DBG_WRITES_CONF"


class COM_MB_BAUD_RATE:
    MB_BAUD_9600 = 0
    MB_BAUD_19200 = 1
    MB_BAUD_38400 = 2
    MB_BAUD_57600 = 3
    MB_BAUD_115200 = 4


class COM_MB_PARITY:
    MB_PARITY_NONE = 0
    MB_PARITY_EVEN = 1
    MB_PARITY_ODD = 2


class COM_MB_STOP_BITS:
    MB_STOP_1 = 0
    MB_STOP_2 = 1


class RTD_MODE:
    RTD_MD_RESISTANCE = 0
    RTD_MD_NTC = 1
    RTD_MD_PLATINUM = 2


class RTD_TEMP_CALIB:
    RTD_CALIB_OFF = 0
    RTD_CALIB_ON = 1


