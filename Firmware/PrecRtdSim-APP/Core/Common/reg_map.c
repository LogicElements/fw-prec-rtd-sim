/**
 * @file       reg_map.c
 * @brief      Register map implementation
 * @addtogroup grRegMap
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "reg_map.h"
#include "configuration.h"

/* Private defines -----------------------------------------------------------*/

/* Private macros  -----------------------------------------------------------*/

/* Private typedefs ----------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/** @defgroup grRegMapStorage
 *  @ingroup grRegMap
 *  @brief Definition of storage memory structure for all registers
 *  @{
 */

/**
 * Storage for the entire register map (structure of structures)
 */
conf_reg_t conf;


uint8_t* const CONF_REG[CONF_REG_BLOCK_NUMBER] = {(uint8_t*)&conf.sys, (uint8_t*)&conf.fact, (uint8_t*)&conf.firm, (uint8_t*)&conf.com, NULL, (uint8_t*)&conf.rtd, (uint8_t*)&conf.dbg, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, };

const uint32_t CONF_REG_LIMIT[CONF_REG_BLOCK_NUMBER] = {
22, 16, 8, 10, 0, 68, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

const uint32_t CONF_REG_FLASH[CONF_REG_FLASH_NUMBER] = {
CONF_SYS_REGMAP_VERSION, CONF_COM_MB_BAUD_RATE, CONF_COM_MB_PARITY, CONF_COM_MB_STOP_BITS, CONF_COM_MB_TIMEOUT, };

const uint32_t CONF_REG_LOGGER[CONF_REG_LOGGER_NUMBER] = {
};

const uint32_t CONF_REG_CALIB[CONF_REG_CALIB_NUMBER] = {
CONF_FACT_SERIAL_NUMBER, };

const uint32_t CONF_REG_SYNCED[CONF_REG_SYNCED_NUMBER] = {
};



/** @} */

/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/


Status_t RegMap_RestoreFactoryValues(void)
{
  Status_t ret = STATUS_OK;
  //uint8_t *id = (uint8_t *)0x1FFF7A10;

  /* Copy firmware info */
  memcpy(&conf.firm, CONF_C_APPLICATION_OFFSET + (uint32_t)CONF_FW_INFO_OFFSET, sizeof(conf.firm));

  /* Copy Hardware info */
  memcpy(&conf.fact, CONF_C_BOOTLOADER_OFFSET + (uint32_t)CONF_FW_INFO_OFFSET, sizeof(conf.fact));

  CONF_INT(CONF_SYS_UPTIME)              = 0;
  CONF_INT(CONF_SYS_REGMAP_VERSION)      = 1002;
  CONF_BYTE(CONF_COM_MB_BAUD_RATE)       = 1;
  CONF_BYTE(CONF_COM_MB_PARITY)          = 1;
  CONF_BYTE(CONF_COM_MB_STOP_BITS)       = 0;
  CONF_SHORT(CONF_COM_MB_ADDRESS)        = 32;
  CONF_SHORT(CONF_COM_MB_TIMEOUT)        = 10;
  CONF_BYTE(CONF_RTD_MODE)               = 0;
  CONF_SHORT(CONF_RTD_CHANNEL_SELECT)    = 0;
  CONF_BYTE(CONF_RTD_EXP_BOARD1_ADDR1)   = 0;
  CONF_BYTE(CONF_RTD_EXP_BOARD1_ADDR2)   = 1;
  CONF_BYTE(CONF_RTD_EXP_BOARD2_ADDR1)   = 2;
  CONF_BYTE(CONF_RTD_EXP_BOARD2_ADDR2)   = 3;
  CONF_BYTE(CONF_RTD_EXP_BOARD_INIT)     = 0;
  CONF_SHORT(CONF_RTD_NTC_BETA)          = 3977;
  CONF_SHORT(CONF_RTD_NTC_STOCK_RES)     = 10000;
  CONF_SHORT(CONF_RTD_PT_STOCK_RES)      = 1000;
  CONF_INT(CONF_RTD_RESISTANCE)          = 10000;
  CONF_FLOAT(CONF_RTD_TEMPERATURE)       = 25.0;
  CONF_BYTE(CONF_RTD_SLEWRATE_MODE)      = 0;
  CONF_FLOAT(CONF_RTD_SLEWRATE)          = 5.0;
  CONF_SHORT(CONF_RTD_SLEWRATE_MAX)      = 50;
  CONF_SHORT(CONF_RTD_SLEWRATE_MIN)      = 15;
  CONF_BYTE(CONF_RTD_CHANNEL_MODE)       = 0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH0)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH1)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH2)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH3)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH4)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH5)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH6)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH7)   = 25.0;
  CONF_FLOAT(CONF_RTD_TEMPERATURE_CH8)   = 25.0;

  /* ---- Hard-coded FACT ---- */
  conf.fact.serial_number = FACT_SERIAL_NUMBER;
  conf.fact.device_id     = FACT_DEVICE_ID;
  conf.fact.hw_revision   = FACT_HW_REVISION;
  conf.fact.boot_revision = FACT_BOOT_REVISION;

  return ret;
}


/** @} */
