/* sg17hw_core.h:
 *
 * SDFE4 Library
 *
 *      Universal target device definitions
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */

#ifndef SG17HW_CORE_H
#define SG17HW_CORE_H

#include "sg17eoc.h"

#define EOC_MSG_MAX_LEN 112

struct sdfe4_channel{
	u16 state               :3;
	u16 state_change        :1;
	u16 conn_state          :5;
	u16 conn_state_change   :1;
	u16 sdi_dpll_sync       :1;
	u16 enabled             :1;
	struct sg17_eoc *eoc;
	u8 perf_prims;
	u8 eoc_rx_msg[EOC_MSG_MAX_LEN];
};
		
#define PWRBO_NORMAL 0
#define PWRBO_FORCED 1
									
#define TCPAM4		1
#define TCPAM8		2
#define TCPAM16		3
#define TCPAM32		4
#define TCPAM64		5
#define TCPAM128	6

struct sdfe4_if_cfg{
	u32 mode : 2;
	u32 repeater : 1;
	u32 startup_initialization:8;
	u32 transaction: 2;
	u32 annex:2;
	u32 input_mode:8;
	u32 loop :2;
	u32 tc_pam :3;
	u32 need_reconf :1;
	u32 clkmode :2;
	u32 pbo_mode :1;

	u16 frequency;
	u16 payload_bits;
	u16 rate;
	u8 pbo_vals[16];
	u8 pbo_vnum;
};

struct sdfe4_stat {
	u8 SNR_Margin_dB;
	u8 LoopAttenuation_dB;
	u8 ES_count;
	u8 SES_count;
	u16 CRC_Anomaly_count;
	u8 LOSWS_count;
	u8 UAS_Count;
	u16 SegmentAnomaly_Count;
	u8 SegmentDefectS_Count;
	u8 CounterOverflowInd;
	u8 CounterResetInd;
};
						
typedef enum {SDFE4v1,SDFE4v2} sdfe4_chipset_type;
#define SDFE4v1_MAX_RATE 5696
#define SDFE4v2_MAX_RATE 14144
																	
#define SG17_IF_MAX     4

#endif

