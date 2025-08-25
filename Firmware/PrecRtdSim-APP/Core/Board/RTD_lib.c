/*
 * RTD_lib.c
 *
 *  Created on: 16.10.2023
 *      Author: Evzen Steif
 */

#include "RTD_lib.h"
#include "MAX7300.h"
#include "configuration.h"
#include "PCA9505.h"
#include "i2c.h"

/* Private constants ---------------------------------------------------------*/
#define tick_second 1000  /* 1 second in ms */

/* Private data structure ----------------------------------------------------*/
typedef struct
{
    uint32_t switch_on_resistance;   /* mΩ, average ON resistance of switches */
    uint8_t  switch_position[6];     /* switch setting for each decade, 0..7 */
    uint32_t resistor_array[6];      /* base resistor values in mΩ */

    float    resistance;             /* resulting resistance in ohms */
    float    temperature;            /* current simulated temperature in °C */
    float    tempSR;                 /* current temperature for slew-rate mode */

    uint32_t tick;                   /* tick counter for 1s update */
    uint8_t  config;                 /* flag for slew-rate initialization */

} RTD_t;

static RTD_t rtd_app;

/* Forward declarations ------------------------------------------------------*/
static void switch_position(uint32_t request);

/**
 * @brief Helper: exponential polynomial approximation.
 * @param x Input value
 * @param n Number of terms
 * @return Approximated exp(x)
 * @note Used for NTC resistance calculation.
 */
static float exponential(float x, int n)
{
    float result = 1.0f;
    float term   = 1.0f;
    for (int i = 1; i <= n; ++i) {
        term *= x / (float)i;
        result += term;
    }
    return result;
}

/* ------------------------------------------------------------------------- */
/* Initialization                                                            */
/* ------------------------------------------------------------------------- */

/**
 * @brief Initialize RTD library.
 * @details Sets resistor array values, switch ON resistance,
 *          and initializes slew-rate state from configuration.
 */
void RTD_Init(void)
{
    /* Values of resistor array soldered on the board (in mΩ) */
    rtd_app.resistor_array[0] = 1.2e3;
    rtd_app.resistor_array[1] = 9.53e3;
    rtd_app.resistor_array[2] = 75e3;
    rtd_app.resistor_array[3] = 590e3;
    rtd_app.resistor_array[4] = 4700e3;
    rtd_app.resistor_array[5] = 37400e3;

    /* Average switch ON resistance (mΩ) */
    rtd_app.switch_on_resistance = 0.36e3;

    rtd_app.tempSR = conf.rtd.slewrate_min;
    rtd_app.config = 1;
}

/* PCA expander base address */
#define PCA_BASE_ADDR  0x20u
#define PCA_ADDR_MASK  0x07u

/**
 * @brief Build 8-bit PCA I2C address from A2..A0 bits.
 */
static inline uint8_t PCA_AddrFromBits(uint8_t addr_bits_0_7)
{
    return (uint8_t)((PCA_BASE_ADDR | (addr_bits_0_7 & PCA_ADDR_MASK)) << 1);
}

/* ------------------------------------------------------------------------- */
/* Switch calculation and programming                                        */
/* ------------------------------------------------------------------------- */

/**
 * @brief Compute switch positions for requested resistance.
 * @param request_ohm Requested resistance (Ω).
 * @note Fills rtd_app.switch_position[0..5].
 */
static void switch_position(uint32_t request_ohm)
{
    uint8_t multiple = 0;
    int8_t  i        = 5;

    float resistance_mohm = (float)(rtd_app.switch_on_resistance * 6u);
    uint32_t request_mohm = request_ohm * 1000u;

    /* clear previous state */
    for (int k = 0; k < 6; ++k) rtd_app.switch_position[k] = 0;

    /* fill switches until requested value is reached */
    while (i >= 0 && resistance_mohm < (float)request_mohm) {

        uint32_t step = rtd_app.resistor_array[i]; /* mΩ */

        uint32_t remain = request_mohm - (uint32_t)resistance_mohm;
        multiple = (uint8_t)(remain / step);
        if (multiple > 7u) multiple = 7u;

        rtd_app.switch_position[i] = multiple;
        resistance_mohm += (float)(multiple * step);
        i--;
    }
}

/**
 * @brief Configure resistor switches via MAX7300 or PCA9505 expanders.
 * @param request Requested resistance in ohms.
 * @note Selects channel using conf.rtd.channel_select.
 */
void set_switch_rezistor(uint32_t request)
{
    /* 1) calculate switch positions */
    switch_position(request);

    /* 2) channel 0 = on-board MAX7300 */
    if (conf.rtd.channel_select == 0u)
    {
        uint32_t cmd = 0u;
        for (int i = 0; i < 6; i++)
            cmd |= ((uint32_t)(rtd_app.switch_position[i] & 0x07u)) << (i * 3);

        MAX_write_bin(cmd);
        return;
    }

    /* 3) build 18-bit command for PCA  */
    uint32_t cmd18 = 0u;
    for (int i = 0; i < 6; i++)
        cmd18 |= ((uint32_t)(rtd_app.switch_position[i] & 0x07u)) << (i * 3);

    /* 4) select module B1/B2, expander E1/E2 and channel A/B */
    uint8_t ch = conf.rtd.channel_select;
    PCA_Module *m = NULL;
    uint8_t use_E1 = 0;
    uint8_t channel_is_A = 0;

    switch (ch)
    {
        /* --- B1 --- */
        case 1: m = &g_B1; use_E1 = 1; channel_is_A = 1; break; // B1 E1 A
        case 2: m = &g_B1; use_E1 = 1; channel_is_A = 0; break; // B1 E1 B
        case 3: m = &g_B1; use_E1 = 0; channel_is_A = 1; break; // B1 E2 A
        case 4: m = &g_B1; use_E1 = 0; channel_is_A = 0; break; // B1 E2 B

        /* --- B2 --- */
        case 5: m = &g_B2; use_E1 = 1; channel_is_A = 1; break; // B2 E1 A
        case 6: m = &g_B2; use_E1 = 1; channel_is_A = 0; break; // B2 E1 B
        case 7: m = &g_B2; use_E1 = 0; channel_is_A = 1; break; // B2 E2 A
        case 8: m = &g_B2; use_E1 = 0; channel_is_A = 0; break; // B2 E2 B

        default: return; // out of range
    }

    /* 5) verify expander exists and write command */
    if (m == NULL) return;

    if (use_E1) {
        if (!m->has_E1) return;
        PCA_WriteCmd18(&hi2c2, m->addr8_E1, cmd18, channel_is_A);
        PCA_BlinkLed(&hi2c2, m, 1 /*E1*/, channel_is_A);
    } else {
        if (!m->has_E2) return;
        PCA_WriteCmd18(&hi2c2, m->addr8_E2, cmd18, channel_is_A);
        PCA_BlinkLed(&hi2c2, m, 0 /*E2*/, channel_is_A);
    }
}

/* ------------------------------------------------------------------------- */
/* Simulation modes                                                           */
/* ------------------------------------------------------------------------- */

/**
 * @brief Direct resistance mode.
 * @note Only valid for 10–290kΩ range.
 */
void setResistance(void)
{
    if (conf.rtd.resistance >= 10u && conf.rtd.resistance <= 290000u)
    {
        set_switch_rezistor(conf.rtd.resistance);
    }
}

/**
 * @brief NTC thermistor simulation.
 * @param temp Temperature (°C). If 0.0f, uses conf.rtd.temperature.
 * @note Uses exponential approximation with Beta parameter.
 */
void setNTC(float temp)
{
    rtd_app.temperature = (temp != 0.0f) ? temp : conf.rtd.temperature;

    if (rtd_app.temperature >= -30.0f && rtd_app.temperature <= 200.0f)
    {
        float val = conf.rtd.ntc_beta *
            (1.0f/(rtd_app.temperature + 273.15f) - 1.0f/(25.0f + 273.15f));

        rtd_app.resistance = conf.rtd.ntc_stock_res * exponential(val, 20);
        set_switch_rezistor((uint32_t)rtd_app.resistance);
    }
}

/**
 * @brief Platinum RTD simulation.
 * @param temp Temperature (°C). If 0.0f, uses conf.rtd.temperature.
 * @note Uses linear approximation R = R0 * (1 + A*T).
 */
void setPT(float temp)
{
    rtd_app.temperature = (temp != 0.0f) ? temp : conf.rtd.temperature;

    if (rtd_app.temperature >= -30.0f && rtd_app.temperature <= 200.0f)
    {
        const float A = 3.91e-3f;
        rtd_app.resistance = conf.rtd.pt_stock_res * (1.0f + A * rtd_app.temperature);
        set_switch_rezistor((uint32_t)rtd_app.resistance);
    }
}

/**
 * @brief Slew-rate temperature simulation.
 * @param rtd_mode 0 = NTC, 1 = PT
 * @note Increments temperature by slewrate every second until max is reached.
 */
void tempSlewRate(uint8_t rtd_mode)
{
    if (rtd_app.config) {
        rtd_app.tick = HAL_GetTick() + tick_second;
        rtd_app.config = 0;
    }

    if (rtd_app.tick != 0u && TICK_EXPIRED(rtd_app.tick)) {

        rtd_app.tempSR += conf.rtd.slewrate;
        rtd_app.tick = 0u;
        rtd_app.config = 1u;

        if (rtd_app.tempSR <= conf.rtd.slewrate_max)
        {
        	(rtd_mode) ? setPT(rtd_app.tempSR) : setNTC(rtd_app.tempSR);
        }
        else
        {
        	rtd_app.tempSR = conf.rtd.slewrate_max;
        }
    }
}

/**
 * @brief Reset slew-rate temperature to minimum.
 * @note Re-arms the 1s scheduler for tempSlewRate().
 */
void tempSlewRateSetMin(void)
{
    rtd_app.tempSR = conf.rtd.slewrate_min;

    /* re-arm the 1s scheduler */
    rtd_app.tick   = 0;   // force tempSlewRate() to set a new tick on next call
    rtd_app.config = 1;   // start a new session
}
