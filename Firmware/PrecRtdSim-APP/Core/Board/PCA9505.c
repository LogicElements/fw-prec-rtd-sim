/*
 * PCA9505.c
 *
 *  Created on: Jul 31, 2025
 *      Author: evzen
 */

#include "stm32f0xx_hal.h"
#include "PCA9505.h"
#include "reg_map.h"
#include <stdint.h>
#include "configuration.h"

/* Základní registry PCA9505/9506 */
#define REG_INPUT_BASE     0x00
#define REG_OUTPUT_BASE    0x08
#define REG_POLARITY_BASE  0x10
#define REG_CONFIG_BASE    0x18

PCA_Module g_B1 = {0}, g_B2 = {0};

typedef struct { PCA9505_Port port; uint8_t bit; } PcaPin;

/* ====== MAPA PINŮ PODLE SCHÉMATU ======
   Kanál A (horní blok): 6 analog. spínačů = sloupce A..F
   Každý sloupec má 3 bity (S1-*, S2-*, S3-*).
   Pořadí bitů v cmd18: [A]{S1,S2,S3}, [B]{S1,S2,S3}, ... [F]{S1,S2,S3}
*/
static const PcaPin kMapA[18] = {
	    /* A  */ {PCA9505_PORT2,3}, {PCA9505_PORT2,4}, {PCA9505_PORT2,5},   // S4-A, S5-A, S6-A
	    /* B  */ {PCA9505_PORT2,6}, {PCA9505_PORT2,7}, {PCA9505_PORT3,0},   // S4-B, S5-B, S6-B
	    /* C  */ {PCA9505_PORT3,1}, {PCA9505_PORT3,2}, {PCA9505_PORT3,3},   // S4-C, S5-C, S6-C
	    /* D  */ {PCA9505_PORT3,4}, {PCA9505_PORT3,5}, {PCA9505_PORT3,6},   // S4-D, S5-D, S6-D
	    /* E  */ {PCA9505_PORT3,7}, {PCA9505_PORT4,0}, {PCA9505_PORT4,1},   // S4-E, S5-E, S6-E
	    /* F  */ {PCA9505_PORT4,2}, {PCA9505_PORT4,3}, {PCA9505_PORT4,4}    // S4-F, S5-F, S6-F
};

/* Kanál B (dolní blok): 6 spínačů = A..F přes S4..S6 */
static const PcaPin kMapB[18] = {
    /* A  */ {PCA9505_PORT0,0}, {PCA9505_PORT0,1}, {PCA9505_PORT0,2},   // S1-A, S2-A, S3-A
    /* B  */ {PCA9505_PORT0,3}, {PCA9505_PORT0,4}, {PCA9505_PORT0,5},   // S1-B, S2-B, S3-B
    /* C  */ {PCA9505_PORT0,6}, {PCA9505_PORT0,7}, {PCA9505_PORT1,0},   // S1-C, S2-C, S3-C
    /* D  */ {PCA9505_PORT1,1}, {PCA9505_PORT1,2}, {PCA9505_PORT1,3},   // S1-D, S2-D, S3-D
    /* E  */ {PCA9505_PORT1,4}, {PCA9505_PORT1,5}, {PCA9505_PORT1,6},   // S1-E, S2-E, S3-E
    /* F  */ {PCA9505_PORT1,7}, {PCA9505_PORT2,0}, {PCA9505_PORT2,1}    // S1-F, S2-F, S3-F
};

uint8_t pca_addr_from_a(uint8_t a2a1a0)
{
    return (uint8_t)((0x20u | (a2a1a0 & 0x07u)) << 1);
}

HAL_StatusTypeDef PCA_WriteReg1(I2C_HandleTypeDef *hi2c, uint8_t addr8, uint8_t reg, uint8_t val)
{
    return HAL_I2C_Mem_Write(hi2c, addr8, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef PCA_InitDevice(I2C_HandleTypeDef *hi2c, uint8_t addr8)
{
    HAL_StatusTypeDef st = HAL_OK;

    // 1) polarity = 0 pro porty 0..4
    for (uint8_t p = 0; p < 5; ++p) {
        st |= PCA_WriteReg1(hi2c, addr8, (uint8_t)(REG_POLARITY_BASE + p), 0x00);
    }

    // 2) config = 0 (vše output) pro porty 0..4
    for (uint8_t p = 0; p < 5; ++p) {
        st |= PCA_WriteReg1(hi2c, addr8, (uint8_t)(REG_CONFIG_BASE + p), 0x00);
    }

    // 3) výstupy do defaultu:
    //    port2 bit2 = 1 (LED A OFF), port4 bit5 = 1 (LED B OFF), jinak 0
    st |= PCA_WriteReg1(hi2c, addr8, REG_OUTPUT_BASE + 0, 0x00);                // port0
    st |= PCA_WriteReg1(hi2c, addr8, REG_OUTPUT_BASE + 1, 0x00);                // port1
    st |= PCA_WriteReg1(hi2c, addr8, REG_OUTPUT_BASE + 2, (uint8_t)(1u<<2));    // port2
    st |= PCA_WriteReg1(hi2c, addr8, REG_OUTPUT_BASE + 3, 0x00);                // port3
    st |= PCA_WriteReg1(hi2c, addr8, REG_OUTPUT_BASE + 4, (uint8_t)(1u<<5));    // port4

    return (st == HAL_OK) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef PCA_InitFromConf(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef st = HAL_OK;

    // --- modul B1 ---
    g_B1.addr8_E1 = pca_addr_from_a(conf.rtd.exp_board1_addr1 & 0x07u);
    g_B1.addr8_E2 = pca_addr_from_a(conf.rtd.exp_board1_addr2 & 0x07u);

    g_B1.has_E1 = (HAL_I2C_IsDeviceReady(hi2c, g_B1.addr8_E1, 2, HAL_MAX_DELAY) == HAL_OK);
    g_B1.has_E2 = (HAL_I2C_IsDeviceReady(hi2c, g_B1.addr8_E2, 2, HAL_MAX_DELAY) == HAL_OK);

    if (g_B1.has_E1) st |= PCA_InitDevice(hi2c, g_B1.addr8_E1);
    if (g_B1.has_E2) st |= PCA_InitDevice(hi2c, g_B1.addr8_E2);

    // --- modul B2 ---
    g_B2.addr8_E1 = pca_addr_from_a(conf.rtd.exp_board2_addr1 & 0x07u);
    g_B2.addr8_E2 = pca_addr_from_a(conf.rtd.exp_board2_addr2 & 0x07u);

    g_B2.has_E1 = (HAL_I2C_IsDeviceReady(hi2c, g_B2.addr8_E1, 2, HAL_MAX_DELAY) == HAL_OK);
    g_B2.has_E2 = (HAL_I2C_IsDeviceReady(hi2c, g_B2.addr8_E2, 2, HAL_MAX_DELAY) == HAL_OK);

    if (g_B2.has_E1) st |= PCA_InitDevice(hi2c, g_B2.addr8_E1);
    if (g_B2.has_E2) st |= PCA_InitDevice(hi2c, g_B2.addr8_E2);

    return (st == HAL_OK) ? HAL_OK : HAL_ERROR;
}



HAL_StatusTypeDef PCA_PinWrite(I2C_HandleTypeDef *hi2c, uint8_t addr8,
                               PCA9505_Port port, uint8_t bit, uint8_t state)
{
    uint8_t ra = (uint8_t)(REG_OUTPUT_BASE + (uint8_t)port);
    uint8_t v  = 0;
    if (HAL_I2C_Mem_Read(hi2c, addr8, ra, I2C_MEMADD_SIZE_8BIT, &v, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;
    if (state) v |=  (uint8_t)(1u << bit);
    else       v &= (uint8_t)~(1u << bit);
    return HAL_I2C_Mem_Write(hi2c, addr8, ra, I2C_MEMADD_SIZE_8BIT, &v, 1, HAL_MAX_DELAY);
}

/* Rozbaluje 18bit cmd (6×3 bity) dle mapy a zapisuje bit-po-bitu */
void PCA_WriteCmd18(I2C_HandleTypeDef *hi2c, uint8_t addr8, uint32_t cmd18, uint8_t channel_is_A)
{
    const PcaPin *map = channel_is_A ? kMapA : kMapB;

    for (uint8_t i = 0; i < 18; i++) {
        uint8_t val = (uint8_t)((cmd18 >> i) & 0x1u);   // pořadí bitů jako u MAXu
        PCA_PinWrite(hi2c, addr8, map[i].port, map[i].bit, val);
    }
}

void PCA_WriteCmd18_Module(I2C_HandleTypeDef *hi2c,
                           uint8_t module_idx,
                           uint32_t cmd18,
                           uint8_t channel_is_A)
{
    PCA_Module *m = (module_idx == 1) ? &g_B1 : &g_B2;

    if (!(m->has_E1 || m->has_E2)) return;  // modul není připojen

    if (m->has_E1) {
        PCA_WriteCmd18(hi2c, m->addr8_E1, cmd18, channel_is_A);
    }
    if (m->has_E2) {
        PCA_WriteCmd18(hi2c, m->addr8_E2, cmd18, channel_is_A);
    }
}


void PCA_BlinkLed(I2C_HandleTypeDef *hi2c,
                  PCA_Module *m,
                  uint8_t use_E1,
                  uint8_t channel_is_A)
{
    if (!m) return;

    uint8_t addr8 = use_E1 ? m->addr8_E1 : m->addr8_E2;
    uint8_t has   = use_E1 ? m->has_E1   : m->has_E2;

    if (!has) return;  // vybraný expander není připojen

    if (channel_is_A) {
        // LED B = IO4_5 (aktivní v 0)
        PCA_PinWrite(hi2c, addr8, PCA9505_PORT4, 5, 0);
        HAL_Delay(500);
        PCA_PinWrite(hi2c, addr8, PCA9505_PORT4, 5, 1);
    } else {
        // LED A = IO2_2 (aktivní v 0)
        PCA_PinWrite(hi2c, addr8, PCA9505_PORT2, 2, 0);
        HAL_Delay(500);
        PCA_PinWrite(hi2c, addr8, PCA9505_PORT2, 2, 1);
    }
}

void PCA_LedA(I2C_HandleTypeDef *hi2c, uint8_t addr8, uint8_t on)
{
    PCA_PinWrite(hi2c, addr8, PCA9505_PORT2, 2, on ? 0 : 1); // aktivní v 0
}
void PCA_LedB(I2C_HandleTypeDef *hi2c, uint8_t addr8, uint8_t on)
{
    PCA_PinWrite(hi2c, addr8, PCA9505_PORT4, 5, on ? 0 : 1); // aktivní v 0
}

