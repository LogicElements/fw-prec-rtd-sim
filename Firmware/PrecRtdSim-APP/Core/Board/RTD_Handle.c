/*
 * RTD_Handle.c
 *
 *  Created on: 15. 2. 2024
 *      Author: evzen
 */

#include "RTD_Handle.h"
#include "RTD_lib.h"
#include "configuration.h"
#include "PCA9505.h"
#include "i2c.h"

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

/* Direct resistance (mode == 0) */
static uint32_t             last_resistance     = 0xFFFFFFFFu;               /* conf.rtd.resistance */

/* NTC (mode == 1) */
static float                last_temperature    = 1.0e30f;                   /* conf.rtd.temperature */
static uint16_t             last_ntc_beta       = 0xFFFFu;                   /* conf.rtd.ntc_beta */
static uint16_t             last_ntc_stock_res  = 0xFFFFu;                   /* conf.rtd.ntc_stock_res */

/* PT (mode == 2) */
static uint16_t             last_pt_stock_res   = 0xFFFFu;                   /* conf.rtd.pt_stock_res */

/* Slew-rate parameters */
static float                last_slew_rate      = 1.0e30f;                   /* conf.rtd.slewrate (float) */
static uint16_t             last_slew_min       = 0xFFFFu;                   /* conf.rtd.slewrate_min (u16) */
static uint16_t             last_slew_max       = 0xFFFFu;                   /* conf.rtd.slewrate_max (u16) */


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
 *       - Direct resistance (mode 0)
 *       - NTC (mode 1)
 *       - PT (mode 2)
 *   - Manages slew-rate logic (disabled for direct mode, active for temperature modes).
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
	}

    /* Snapshot current config */
    uint32_t cur_channel_select = conf.rtd.channel_select;
    uint32_t cur_mode           = conf.rtd.mode;
    uint32_t cur_slew_mode      = conf.rtd.slewrate_mode;

    /* --- Force-disable slew-rate when in DIRECT mode (mode 0) ------------ */
    if (cur_mode == 0u && cur_slew_mode != 0u)
    {
        conf.rtd.slewrate_mode = 0u;     /* auto-disable SR (invalid for direct) */
        cur_slew_mode = 0u;              /* reflect locally so we enter non-slew branch */
        tempSlewRateSetMin();            /* reset SR internals; safe no-op for direct */
        /* @note We intentionally do not update last_slewrate_mode here,
           so the transition triggers setResistance() later. */
    }

    /* 2) Non-slew mode: trigger actions only on relevant changes (incl. SR edge) */
    if (cur_slew_mode == 0u)
    {
        /* keep SR cursor at MIN for clean handover */
        tempSlewRateSetMin();

        switch (cur_mode)
        {
            case 0u: /* Direct resistance mode */
                if (conf.rtd.resistance      != last_resistance     ||
                    cur_channel_select        != last_channel_select ||
                    cur_mode                  != last_mode           ||
                    cur_slew_mode             != last_slewrate_mode)
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
                    cur_slew_mode             != last_slewrate_mode)
                {
                    last_temperature    = conf.rtd.temperature;
                    last_ntc_beta       = conf.rtd.ntc_beta;
                    last_ntc_stock_res  = conf.rtd.ntc_stock_res;
                    last_channel_select = cur_channel_select;
                    last_mode           = cur_mode;
                    /* @note Passing 0.0f makes setNTC use conf.rtd.temperature internally */
                    setNTC(0.0f);
                }
                break;

            case 2u: /* PT mode */
                if (conf.rtd.temperature     != last_temperature    ||
                    conf.rtd.pt_stock_res    != last_pt_stock_res   ||
                    cur_channel_select        != last_channel_select ||
                    cur_mode                  != last_mode           ||
                    cur_slew_mode             != last_slewrate_mode)
                {
                    last_temperature    = conf.rtd.temperature;
                    last_pt_stock_res   = conf.rtd.pt_stock_res;
                    last_channel_select = cur_channel_select;
                    last_mode           = cur_mode;
                    /* @note Passing 0.0f makes setPT use conf.rtd.temperature internally */
                    setPT(0.0f);
                }
                break;

            default:
                /* no operation */
                break;
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

    /* remember SR mode for next edge detection */
    last_slewrate_mode = cur_slew_mode;
}
