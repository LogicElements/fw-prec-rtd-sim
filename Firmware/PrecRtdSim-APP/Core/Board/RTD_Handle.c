/*
 * RTD_Handle.c
 *
 *  Created on: 15. 2. 2024
 *      Author: evzen
 */

#include <stddef.h>
#include <string.h>
#include "RTD_Handle.h"
#include "RTD_lib.h"
#include "configuration.h"
#include "PCA9505.h"
#include "i2c.h"

/* Channels 0-8: 0 = main board, 1-4 = 1. expansion board, 5-8 = 2. expansion board */
#define RTD_CHANNELS  9u

/* ------------------------------------------------------------------------- */
/* Change tracking                                                           */
/* ------------------------------------------------------------------------- */

/* Expander I2C address enums (force re-init if they change) */
static rtd_exp_board1_addr1_t last1_addr1 = (rtd_exp_board1_addr1_t)0xFF;
static rtd_exp_board1_addr2_t last1_addr2 = (rtd_exp_board1_addr2_t)0xFF;

static rtd_exp_board2_addr1_t last2_addr1 = (rtd_exp_board2_addr1_t)0xFF;
static rtd_exp_board2_addr2_t last2_addr2 = (rtd_exp_board2_addr2_t)0xFF;

/* Core selectors / modes */
static uint16_t             last_channel_select = 0xFFFFu;                   /* conf.rtd.channel_select */
static rtd_mode_t           last_mode           = (rtd_mode_t)0xFF;          /* conf.rtd.mode */
static rtd_slewrate_mode_t  last_slewrate_mode  = (rtd_slewrate_mode_t)0xFF; /* conf.rtd.slewrate_mode */
static rtd_channel_mode_t   last_channel_mode   = (rtd_channel_mode_t)0xFF;  /* conf.rtd.channel_mode */

/* Direct resistance (mode == 0) */
static uint32_t             last_resistance     = 0xFFFFFFFFu;               /* conf.rtd.resistance */

/* NTC (mode == 1) */
static float                last_temperature    = 1.0e30f;                   /* conf.rtd.temperature */
static uint16_t             last_ntc_beta       = 0xFFFFu;                   /* conf.rtd.ntc_beta */
static uint16_t             last_ntc_stock_res  = 0xFFFFu;                   /* conf.rtd.ntc_stock_res */

/* PT (mode == 2) */
static uint16_t             last_pt_stock_res   = 0xFFFFu;                   /* conf.rtd.pt_stock_res */

/* Per channel mode: temperature last applied to each channel (conf.rtd.temperature_ch0 .. ch8) */
static float                last_temperature_ch[RTD_CHANNELS];

/* Slew-rate parameters */
static float                last_slew_rate      = 1.0e30f;                   /* conf.rtd.slewrate (float) */
static uint16_t             last_slew_min       = 0xFFFFu;                   /* conf.rtd.slewrate_min (u16) */
static uint16_t             last_slew_max       = 0xFFFFu;                   /* conf.rtd.slewrate_max (u16) */


/* ------------------------------------------------------------------------- */
/* Per channel mode                                                          */
/* ------------------------------------------------------------------------- */

/**
 * @brief Applies the changed RTD_TEMPERATURE_CHx to their channels (NTC / PT mode).
 * @param pt   0 = NTC, 1 = platinum RTD.
 * @param all  Apply every channel - a common parameter (mode, NTC / PT constants, channel mode) changed.
 * @param skip Channel left alone - the slew rate drives it (RTD_CHANNELS = none).
 */
static void RTD_HandlePerChannel(uint8_t pt, uint8_t all, uint16_t skip)
{
    for (uint8_t ch = 0u; ch < RTD_CHANNELS; ch++)
    {
        if (ch == skip)
        {
            continue;
        }
        /* Copied out: the packed struct gives no aligned float pointer */
        float temp;
        memcpy(&temp, (const uint8_t *)&conf.rtd + offsetof(conf_reg_rtd_t, temperature_ch0) + ch * sizeof(float),
               sizeof(float));
        if (all || temp != last_temperature_ch[ch])
        {
            last_temperature_ch[ch] = temp;
            RTD_SetChannelTemperature(ch, pt, temp);
        }
    }
}


/* ------------------------------------------------------------------------- */
/* Handle                                                                    */
/* ------------------------------------------------------------------------- */

/**
 * @brief Main RTD handler.
 *
 * @details
 *   - Tracks changes in configuration (conf.rtd.*).
 *   - Re-initializes expanders when board addresses change.
 *   - Applies RTD settings depending on mode:
 *       - Direct resistance (mode 0) - on the channel of RTD_CHANNEL_SELECT
 *       - NTC (mode 1) / PT (mode 2) - RTD_CHANNEL_MODE selected channel: RTD_TEMPERATURE on the channel of
 *         RTD_CHANNEL_SELECT; per channel: every channel its own RTD_TEMPERATURE_CHx
 *   - Manages slew-rate logic (disabled for direct mode, active for temperature modes, on the channel of
 *     RTD_CHANNEL_SELECT in both channel modes - per channel, the other channels keep their own temperatures).
 *   - Switches the channel LEDs off after their flash.
 *
 * @note This function should be called periodically (e.g. in main loop).
 */
void RTD_Handle(void)
{
    /* 1) Re-init expanders if their board address bits changed */
    if (conf.rtd.exp_board_init == 1 ||
        conf.rtd.exp_board1_addr1 != last1_addr1 ||
        conf.rtd.exp_board1_addr2 != last1_addr2 ||
        conf.rtd.exp_board2_addr1 != last2_addr1 ||
        conf.rtd.exp_board2_addr2 != last2_addr2)
    {
        /* update stored addresses */
        last1_addr1 = conf.rtd.exp_board1_addr1;
        last1_addr2 = conf.rtd.exp_board1_addr2;
        last2_addr1 = conf.rtd.exp_board2_addr1;
        last2_addr2 = conf.rtd.exp_board2_addr2;

        /* clear init flag and re-init expanders */
        conf.rtd.exp_board_init = 0;
        PCA_InitFromConf(&hi2c2);

        /* The re-init switched every expander channel to position 0 - apply all channels again */
        last_mode = (rtd_mode_t)0xFF;
    }

    /* Snapshot current config */
    uint32_t cur_channel_select = conf.rtd.channel_select;
    uint32_t cur_mode           = conf.rtd.mode;
    uint32_t cur_slew_mode      = conf.rtd.slewrate_mode;
    uint32_t cur_channel_mode   = conf.rtd.channel_mode;
    uint8_t  per_channel        = (cur_mode == 1u || cur_mode == 2u) && cur_channel_mode == RTD_CHM_PER_CHANNEL;

    /* --- Force-disable slew-rate when in DIRECT mode (mode 0) ------------ */
    if (cur_mode == 0u && cur_slew_mode != 0u)
    {
        conf.rtd.slewrate_mode = 0u;     /* auto-disable SR (invalid for direct) */
        cur_slew_mode = 0u;              /* reflect locally so we enter non-slew branch */
        tempSlewRateSetMin();            /* reset SR internals; safe no-op for direct */
        /* @note We intentionally do not update last_slewrate_mode here,
           so the transition triggers setResistance() later. */
    }

    /* NTC / PT per channel: a change of a common parameter applies every channel again - with the slew rate on
       also a change of its channel, so the channel it leaves gets its own temperature back */
    uint8_t all = 0u;
    if (per_channel)
    {
        all = (cur_mode                != last_mode          ||
               cur_slew_mode           != last_slewrate_mode ||
               cur_channel_mode        != last_channel_mode  ||
               conf.rtd.ntc_beta       != last_ntc_beta      ||
               conf.rtd.ntc_stock_res  != last_ntc_stock_res ||
               conf.rtd.pt_stock_res   != last_pt_stock_res  ||
               (cur_slew_mode != 0u && cur_channel_select != last_channel_select));
        last_ntc_beta      = conf.rtd.ntc_beta;
        last_ntc_stock_res = conf.rtd.ntc_stock_res;
        last_pt_stock_res  = conf.rtd.pt_stock_res;
    }

    /* 2) Non-slew mode: trigger actions only on relevant changes (incl. SR edge) */
    if (cur_slew_mode == 0u)
    {
        /* keep SR cursor at MIN for clean handover */
        tempSlewRateSetMin();

        if (per_channel)
        {
            /* 2a) NTC / PT per channel */
            last_mode = cur_mode;
            RTD_HandlePerChannel((uint8_t)(cur_mode == 2u), all, RTD_CHANNELS);
        }
        else
        {
            switch (cur_mode)
            {
                case 0u: /* Direct resistance mode */
                    if (conf.rtd.resistance      != last_resistance     ||
                        cur_channel_select        != last_channel_select ||
                        cur_mode                  != last_mode           ||
                        cur_slew_mode             != last_slewrate_mode  ||
                        cur_channel_mode          != last_channel_mode)
                    {
                        last_resistance     = conf.rtd.resistance;
                        last_channel_select = cur_channel_select;
                        last_mode           = cur_mode;
                        setResistance();
                    }
                    break;

                case 1u: /* NTC mode */
                    if (conf.rtd.temperature     != last_temperature    ||
                        conf.rtd.ntc_beta        != last_ntc_beta       ||
                        conf.rtd.ntc_stock_res   != last_ntc_stock_res  ||
                        cur_channel_select        != last_channel_select ||
                        cur_mode                  != last_mode           ||
                        cur_slew_mode             != last_slewrate_mode  ||
                        cur_channel_mode          != last_channel_mode)
                    {
                        last_temperature    = conf.rtd.temperature;
                        last_ntc_beta       = conf.rtd.ntc_beta;
                        last_ntc_stock_res  = conf.rtd.ntc_stock_res;
                        last_channel_select = cur_channel_select;
                        last_mode           = cur_mode;
                        setNTC(conf.rtd.temperature);
                    }
                    break;

                case 2u: /* PT mode */
                    if (conf.rtd.temperature     != last_temperature    ||
                        conf.rtd.pt_stock_res    != last_pt_stock_res   ||
                        cur_channel_select        != last_channel_select ||
                        cur_mode                  != last_mode           ||
                        cur_slew_mode             != last_slewrate_mode  ||
                        cur_channel_mode          != last_channel_mode)
                    {
                        last_temperature    = conf.rtd.temperature;
                        last_pt_stock_res   = conf.rtd.pt_stock_res;
                        last_channel_select = cur_channel_select;
                        last_mode           = cur_mode;
                        setPT(conf.rtd.temperature);
                    }
                    break;

                default:
                    /* no operation */
                    break;
            }
        }
    }
    else
    {
        /* 3) Slew-rate mode ON: reset session on any relevant change or edge, then tick */
        if (cur_slew_mode           != last_slewrate_mode  ||
            conf.rtd.slewrate       != last_slew_rate      ||
            conf.rtd.slewrate_min   != last_slew_min       ||
            conf.rtd.slewrate_max   != last_slew_max       ||
            cur_channel_select       != last_channel_select ||
            cur_mode                 != last_mode)
        {
            last_slew_rate      = conf.rtd.slewrate;
            last_slew_min       = conf.rtd.slewrate_min;
            last_slew_max       = conf.rtd.slewrate_max;
            last_channel_select = cur_channel_select;
            last_mode           = cur_mode;

            /* restart from MIN and arm next tick; ensures first apply in RTD_lib */
            tempSlewRateSetMin();
        }

        /* Per channel: the other channels keep their own temperatures */
        if (per_channel)
        {
            RTD_HandlePerChannel((uint8_t)(cur_mode == 2u), all, cur_channel_select);
        }

        switch (cur_mode)
        {
            case 1u: /* NTC with slew-rate */
                tempSlewRate(0u);
                break;

            case 2u: /* PT with slew-rate */
                tempSlewRate(1u);
                break;

            case 0u:
            default:
                /* no-op while SR is ON but not in temperature modes */
                break;
        }
    }

    /* remember SR and channel mode for next edge detection */
    last_slewrate_mode = cur_slew_mode;
    last_channel_mode  = cur_channel_mode;

    /* 4) Channel LEDs flashed by the writes above */
    PCA_LedsHandle(&hi2c2);
}
