#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include "ifx_types.h"
#include "drv_tapi_io.h"
#include "vinetic_io.h"
#include "ab_api.h"
#include "ab_ioctl.h"

#define RTP_READ_MAX 1024
#define ALAW_PT_DF 8
#define MLAW_PT_DF 0

#define DEFAULT_CFG_NAME    "*"
#define ALAW_CFG_NAME    "aLaw"
#define G729_CFG_NAME    "g729"
#define G729E_CFG_NAME   "g729e"
#define ILBC133_CFG_NAME "iLBC_133"
#define G723_CFG_NAME    "g723"
#define G72616_CFG_NAME  "g726_16"
#define G72624_CFG_NAME  "g726_24"
#define G72632_CFG_NAME  "g726_32"
#define G72640_CFG_NAME  "g726_40"
#define VADOFF_CFG_NAME  "off"
#define VADON_CFG_NAME   "on"
#define VADG711_CFG_NAME "g711"
#define VADCNG_CFG_NAME  "cng_only"
#define VADSC_CFG_NAME   "sc_only"



struct opts_s {/*{{{*/
	int help;
	codec_t vcod;
	codec_t fcod;
	int vol;
	enum vad_cfg_e vad;
	int hpf;
} g_so;/*}}}*/

struct status_s {/*{{{*/
	int c1_is_offhook;
	int c2_is_offhook;
	int enc_dec_is_on;
	int c1_id;
	int c2_id;
	int poll_fd_num;
} g_status;/*}}}*/

void
set_df_pt( void )
{/*{{{*/
	if       (g_so.vcod.type == cod_type_ALAW){
		g_so.vcod.sdp_selected_payload = 8;
	} else if(g_so.vcod.type == cod_type_G729){
		g_so.vcod.sdp_selected_payload = 18;
	} else if(g_so.vcod.type == cod_type_G729E){
		g_so.vcod.sdp_selected_payload = 99;
	} else if(g_so.vcod.type == cod_type_ILBC_133){
		g_so.vcod.sdp_selected_payload = 100;
		/*
	} else if(g_so.vcod.type == cod_type_ILBC_152){
		g_so.vcod.sdp_selected_payload = 100;
		*/
	} else if(g_so.vcod.type == cod_type_G723){
		g_so.vcod.sdp_selected_payload = 4;
	} else if(g_so.vcod.type == cod_type_G726_16){
		g_so.vcod.sdp_selected_payload = 101;
	} else if(g_so.vcod.type == cod_type_G726_24){
		g_so.vcod.sdp_selected_payload = 101;
	} else if(g_so.vcod.type == cod_type_G726_32){
		g_so.vcod.sdp_selected_payload = 101;
	} else if(g_so.vcod.type == cod_type_G726_40){
		g_so.vcod.sdp_selected_payload = 101;
	}
}/*}}}*/

void
set_df_sz( void )
{/*{{{*/
	if( g_so.vcod.type == cod_type_G729 ||
		g_so.vcod.type == cod_type_G729E ||
		g_so.vcod.type == cod_type_G726_16 ||
		g_so.vcod.type == cod_type_G726_24 ||
		g_so.vcod.type == cod_type_G726_32 ||
		g_so.vcod.type == cod_type_G726_40){
			g_so.vcod.pkt_size = cod_pkt_size_10;
	} else if(	
		g_so.vcod.type == cod_type_ALAW){
			g_so.vcod.pkt_size = cod_pkt_size_20;
	} else if(
		g_so.vcod.type == cod_type_ILBC_133 ||
		g_so.vcod.type == cod_type_G723){
			g_so.vcod.pkt_size = cod_pkt_size_30;
	}
}/*}}}*/

int
startup_init (int argc, char * const argv[])
{/*{{{*/
	int check_bitpack = 0;
	int option_IDX;
	int option_rez;
	char * short_options = "hc:f:v:pa:o";
	struct option long_options[ ] = {
		{ "help", no_argument, NULL, 'h' },
		{ "vcod", required_argument, NULL, 'c' },
		{ "fcod", required_argument, NULL, 'f' },
		{ "volume", required_argument, NULL, 'v' },
		{ "hpf", no_argument, NULL, 'p' },
		{ "vad", required_argument, NULL, 'a' },
		{ "use-aal2-order", no_argument, NULL, 'o' },
		{ NULL, 0, NULL, 0 }
	};


	memset(&g_so, 0, sizeof(g_so));

	/* Default values */
	g_so.vcod.type = cod_type_G729;
	g_so.vcod.sdp_selected_payload = 18;
	g_so.vcod.pkt_size = cod_pkt_size_10;
	g_so.vcod.bpack = bitpack_RTP;
	g_so.fcod.type = cod_type_ALAW;
	g_so.fcod.sdp_selected_payload = 8;
	g_so.vol = 0;
	g_so.hpf = 0;
	g_so.vad = vad_cfg_OFF;

	/* Get user values */
	opterr = 0;
	while ((option_rez = getopt_long ( 
			argc, argv, short_options, long_options, &option_IDX)) != -1){
		switch( option_rez ){
		case 'h':
			g_so.help = 1;
			return 1;
		case 'v':
			g_so.vol = strtol(optarg, NULL, 10);
			break;
		case 'c':{
			char * token = NULL;
			token = strtok(optarg, "[:]");
			if(token) {
				if       ( !strcmp(token,ALAW_CFG_NAME)){
					g_so.vcod.type = cod_type_ALAW;
				} else if( !strcmp(token,G729_CFG_NAME)) {
					g_so.vcod.type = cod_type_G729;
				} else if( !strcmp(token,G729E_CFG_NAME)) {
					g_so.vcod.type = cod_type_G729E;
				} else if( !strcmp(token,ILBC133_CFG_NAME)) {
					g_so.vcod.type = cod_type_ILBC_133;
				} else if( !strcmp(token,G723_CFG_NAME)) {
					g_so.vcod.type = cod_type_G723;
				} else if( !strcmp(token,G72616_CFG_NAME)) {
					g_so.vcod.type = cod_type_G726_16;
					check_bitpack = 1;
				} else if( !strcmp(token,G72624_CFG_NAME)) {
					g_so.vcod.type = cod_type_G726_24;
					check_bitpack = 1;
				} else if( !strcmp(token,G72632_CFG_NAME)) {
					g_so.vcod.type = cod_type_G726_32;
					check_bitpack = 1;
				} else if( !strcmp(token,G72640_CFG_NAME)) {
					g_so.vcod.type = cod_type_G726_40;
					check_bitpack = 1;
				} else {
					return -1;
				}
			} else {
				return -1;
			}

			token = strtok(NULL, "[:]");
			if(token) {
				if( !strcmp(token, DEFAULT_CFG_NAME)){
					set_df_pt();
				} else {
					g_so.vcod.sdp_selected_payload = strtol(token, NULL, 10);
				}
			} else {
				return -1;
			}

			token = strtok(NULL, "[:]");
			if(token) {
				if       ( !strcmp(token,"2.5")){
					g_so.vcod.pkt_size = cod_pkt_size_2_5;
				} else if( !strcmp(token,"5")){
					g_so.vcod.pkt_size = cod_pkt_size_5;
				} else if( !strcmp(token,"5.5")){
					g_so.vcod.pkt_size = cod_pkt_size_5_5;
				} else if( !strcmp(token,"10")){
					g_so.vcod.pkt_size = cod_pkt_size_10;
				} else if( !strcmp(token,"11")){
					g_so.vcod.pkt_size = cod_pkt_size_11;
				} else if( !strcmp(token,"20")){
					g_so.vcod.pkt_size = cod_pkt_size_20;
				} else if( !strcmp(token,"30")){
					g_so.vcod.pkt_size = cod_pkt_size_30;
				} else if( !strcmp(token,"40")){
					g_so.vcod.pkt_size = cod_pkt_size_40;
				} else if( !strcmp(token,"50")){
					g_so.vcod.pkt_size = cod_pkt_size_50;
				} else if( !strcmp(token,"60")){
					g_so.vcod.pkt_size = cod_pkt_size_60;
				} else if( !strcmp(token,DEFAULT_CFG_NAME)){
					set_df_sz();
				} else {
					return -1;
				}
			} else {
				return -1;
			}
			break;
		}
		case 'f':
			if       ( !strcmp(optarg,ALAW_CFG_NAME)){
				g_so.fcod.type = cod_type_ALAW;
			} else {
				return -1;
			}
			break;
		case 'p' :
			g_so.hpf = 1;
			break;
		case 'a' :
			if       ( !strcmp(optarg,VADOFF_CFG_NAME)){
				g_so.vad = vad_cfg_OFF;
			} else if( !strcmp(optarg,VADON_CFG_NAME)) {
				g_so.vad = vad_cfg_ON;
			} else if( !strcmp(optarg,VADG711_CFG_NAME)) {
				g_so.vad = vad_cfg_G711;
			} else if( !strcmp(optarg,VADCNG_CFG_NAME)) {
				g_so.vad = vad_cfg_CNG_only;
			} else if( !strcmp(optarg,VADSC_CFG_NAME)) {
				g_so.vad = vad_cfg_SC_only;
			} else {
				return -1;
			}
			break;
		case 'o' :
			g_so.vcod.bpack = bitpack_AAL2;
			break;
		case '?' :
			return -1;
		}
	}

	if(g_so.vcod.type == g_so.fcod.type){
		g_so.fcod.sdp_selected_payload = g_so.vcod.sdp_selected_payload;
	} else if(g_so.fcod.type == cod_type_ALAW){
		g_so.fcod.sdp_selected_payload = ALAW_PT_DF;
	}

	if( !check_bitpack){
		g_so.vcod.bpack = bitpack_RTP;
	}

	return 0;
}/*}}}*/

void
startup_print (void)
{/*{{{*/
	fprintf(stderr,"Choosed configuration:\n");
	fprintf(stderr,"====================================\n");
	fprintf(stderr,"  Voice coder type   : ");
	if       (g_so.vcod.type == cod_type_ALAW){
		fprintf(stderr,"%s\n",ALAW_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G729){
		fprintf(stderr,"%s\n",G729_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G729E){
		fprintf(stderr,"%s\n",G729E_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_ILBC_133){
		fprintf(stderr,"%s\n",ILBC133_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G723){
		fprintf(stderr,"%s\n",G723_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G726_16){
		fprintf(stderr,"%s\n",G72616_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G726_24){
		fprintf(stderr,"%s\n",G72624_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G726_32){
		fprintf(stderr,"%s\n",G72632_CFG_NAME);
	} else if(g_so.vcod.type == cod_type_G726_40){
		fprintf(stderr,"%s\n",G72640_CFG_NAME);
	}
	fprintf(stderr,"  Voice coder payload: %d\n",
			g_so.vcod.sdp_selected_payload);
	fprintf(stderr,"  Voice coder pk size: ");
	if       (g_so.vcod.pkt_size == 0){
		fprintf(stderr,"2.5\n");
	} else if(g_so.vcod.pkt_size == 1){
		fprintf(stderr,"5\n");
	} else if(g_so.vcod.pkt_size == 2){
		fprintf(stderr,"5.5\n");
	} else if(g_so.vcod.pkt_size == 3){
		fprintf(stderr,"10\n");
	} else if(g_so.vcod.pkt_size == 4){
		fprintf(stderr,"11\n");
	} else if(g_so.vcod.pkt_size == 5){
		fprintf(stderr,"20\n");
	} else if(g_so.vcod.pkt_size == 6){
		fprintf(stderr,"30\n");
	} else if(g_so.vcod.pkt_size == 7){
		fprintf(stderr,"40\n");
	} else if(g_so.vcod.pkt_size == 8){
		fprintf(stderr,"50\n");
	} else if(g_so.vcod.pkt_size == 9){
		fprintf(stderr,"60\n");
	}
	fprintf(stderr,"  BitPack            : ");
	if(g_so.vcod.bpack == bitpack_RTP){
		fprintf(stderr,"RTP\n");
	} else {
		fprintf(stderr,"AAL2\n");
	}
	fprintf(stderr,"============\n");
	fprintf(stderr,"  Fax coder type     : ");
	if       (g_so.fcod.type == cod_type_ALAW){
		fprintf(stderr,"%s\n",ALAW_CFG_NAME);
	}
	fprintf(stderr,"  Fax coder payload  : %d\n",
			g_so.fcod.sdp_selected_payload);
	fprintf(stderr,"============\n");
	fprintf(stderr,"  Voice activity detector: ");
	if       (g_so.vad == vad_cfg_OFF){
		fprintf(stderr,"%s\n",VADOFF_CFG_NAME);
	} else if(g_so.vad == vad_cfg_ON){
		fprintf(stderr,"%s\n",VADON_CFG_NAME);
	} else if(g_so.vad == vad_cfg_G711){
		fprintf(stderr,"%s\n",VADG711_CFG_NAME);
	} else if(g_so.vad == vad_cfg_CNG_only){
		fprintf(stderr,"%s\n",VADCNG_CFG_NAME);
	} else if(g_so.vad == vad_cfg_SC_only){
		fprintf(stderr,"%s\n",VADSC_CFG_NAME);
	} 
	fprintf(stderr,"  High-pass filter       : ");
	if(g_so.hpf){
		fprintf(stderr,"on\n");
	} else {
		fprintf(stderr,"off\n");
	}
	fprintf(stderr,"============\n");
	fprintf(stderr,"  Volume gains       : %d\n",g_so.vol);
	fprintf(stderr,"====================================\n");
}/*}}}*/

void
capabilities_print (ab_t * ab)
{/*{{{*/
	IFX_TAPI_CAP_t * caps = NULL;
	int fdc;
	int caps_cnt;
	int i;
	int j;
	int err;

	for (i=0; i<ab->chans_num; i++){
		fdc = ab->chans[i].rtp_fd;
		ioctl (fdc, IFX_TAPI_CAP_NR, &caps_cnt);
		caps = malloc (sizeof(*caps)*caps_cnt);
		err = ioctl (fdc, IFX_TAPI_CAP_LIST, caps);
		if(err){
			fprintf(stderr,"ERROR on IFX_TAPI_CAP_LIST\n");
		}

		fprintf(stderr,"Channel [%d]:\n",ab->chans[i].abs_idx);
		for (j=0; j<caps_cnt; j++){
			if(caps[j].captype == IFX_TAPI_CAP_TYPE_CODEC){
				fprintf(stderr,"Codec: %s\n",caps[j].desc);
			} else if(caps[j].captype == IFX_TAPI_CAP_TYPE_PCM){
				fprintf(stderr,"PCM: %d\n",caps[j].cap);
			} else if(caps[j].captype == IFX_TAPI_CAP_TYPE_CODECS){
				fprintf(stderr,"Coder: %d\n",caps[j].cap);
			} else if(caps[j].captype == IFX_TAPI_CAP_TYPE_PHONES){
				fprintf(stderr,"Phones: %d\n",caps[j].cap);
			}
		}
		free(caps);
		caps = NULL;
	}
}/*}}}*/

void 
print_help(void)
{/*{{{*/
	printf("Use it with options:\n\
\t-h, --help - help message.\n\
==============================\n\
\t-c, --vcod                   - set voice coder.\n\
\t-f, --fcod                   - set fax data coder.\n\
\t-v, --volume                 - set coder volume gains.\n\
\t-p, --hpf                    - set turns on high-pass filter in decoder.\n\
\t-a, --vad                    - set voice activity detector mode.\n\
\t-o, --use-aal2-order         - set aal2 bitpack instead of rtp.\n\
\n\
\t\tvcod must be set in the format '[<type>:<pt>:<pkt_sz>]',\n\
\t\t\twhere <type> is one of \"aLaw\",\"uLaw\",\"iLBC_133\",\n\
\t\t\t\"iLBC_152\",\"g723\",\"g726_16\",\"g726_24\",\"g726_32\",\n\
\"g726_40\", \"g729\" or \"g729e\"\n\
\t\t\t<pt> - any digit value or \"*\" for default value, and\n\
\t\t\t<pkt_size> can be \"2.5\",\"5\", \"5.5\", \"10\",\"11\",\n\
\t\t\t\"\"20\",\"30\", \"40\",\"50\" and \"60\", or \"*\" for default value.\n\
\t\tfcod must be set in the format '<type>',\n\
\t\t\twhere <type> is one of \"aLaw\"or \"uLaw\".\n\
\t\tvolume the digit from '-24' to '+24'\n\
\t\thpf just switch on the filter - it is not required argument.\n\
\t\tvad can be one of \"on\",\"off\",\"g711\",\"cng_only\" and \"sc_only\".\n\
\t\tuse-aal2-order just switch bitpack - works just with g726_* codecs.\n\
==============================\n\
\t\tBy default the follow options are set:\n\
\t\t--vcod='[g729:18:10]' --fcod='aLaw' --volume='0' --vad='off'\n\
==============================\n");
}/*}}}*/

int 
start_connection(ab_t * const ab)
{/*{{{*/
	ab_chan_t * c1 = &ab->chans[g_status.c1_id];
	ab_chan_t * c2 = &ab->chans[g_status.c2_id];
	rtp_session_prms_t rtpp;

	int err = 0;

	memset(&rtpp, 0, sizeof(rtpp));

	/* tune all */
	rtpp.enc_dB = g_so.vol;
	rtpp.dec_dB = g_so.vol;
	rtpp.ATX_dB = 0;
	rtpp.ARX_dB = 0;
	rtpp.VAD_cfg = g_so.vad;
	rtpp.HPF_is_ON = g_so.hpf;

	err += ab_chan_media_rtp_tune(c1, &g_so.vcod, &g_so.fcod, &rtpp);
	err += ab_chan_media_rtp_tune(c2, &g_so.vcod, &g_so.fcod, &rtpp);

	/* start enc / dec */
	err += ab_chan_media_switch (c1, 1);
	err += ab_chan_media_switch (c2, 1);

	return err;
}/*}}}*/

int 
stop_connection(ab_t * const ab)
{/*{{{*/
	int err = 0;
	/* stop enc / dec */
	err += ab_chan_media_switch (&ab->chans[g_status.c1_id], 0);
	err += ab_chan_media_switch (&ab->chans[g_status.c2_id], 0);
	return err;
}/*}}}*/

void
rwdata(int ffd,int tfd, unsigned char const f_aid, unsigned char const t_aid)
{/*{{{*/
	//static int iter = 0;
	int rode;
	int written;
	unsigned char buff[RTP_READ_MAX];

	memset(buff, 0, RTP_READ_MAX);
	rode = read(ffd, buff, RTP_READ_MAX);
	if(rode == -1){
		fprintf(stderr,"[%d]: ", f_aid);
		perror("read(): ");
	} else if(rode == 0){
		fprintf(stderr,"[%d]: Unexpected event (nothing to read)\n", f_aid);
	} else {
		written = write (tfd, buff, rode);
		if(written == -1){
			fprintf(stderr,"[%d]: ", t_aid);
			perror("write(): ");
		} else if( written != rode ){
			fprintf(stderr,"[%d]=>[%d]: RWD error: [%d/%d]\n",
					f_aid,t_aid,rode,written);
		} else {
#if 0
			int first = (buff[12] >> 1) & 0x01;
			int second = buff[12] & 0x01;
			fprintf(stderr,"%d_[%d]=/%d/=>..._0x%02X_%d_%d\n",
					iter,
					f_aid,
					rode,
					buff[12],
					first,second);
			if(iter == 500){
				IFX_TAPI_ENC_CFG_t encCfg;
				int err;
				memset (&encCfg, 0, sizeof(encCfg));
				encCfg.nFrameLen = IFX_TAPI_COD_LENGTH_30;
				encCfg.nEncType = IFX_TAPI_COD_TYPE_G723_63;

				/* Set the encoder */ 
				err = ioctl(ffd, IFX_TAPI_ENC_CFG_SET, &encCfg);
				if(err){
					fprintf(stderr,"Can`t set bitrate.\n");
				}
				err = ioctl(tfd, IFX_TAPI_ENC_CFG_SET, &encCfg);
				if(err){
					fprintf(stderr,"Can`t set bitrate.\n");
				}
			}
			iter++;
			/* for g723 */
			bits  content           	           octets/frame
			00    high-rate speech (6.3 kb/s)            24 (36) 12 - header
			01    low-rate speech  (5.3 kb/s)            20 (32) 12 - header
			10    SID frame                               4
			11    reserved

			int i;
			fprintf(stderr,"[%d]=>_",f_aid);
			for(i=0;i<12;i++){
				fprintf(stderr, "%02X_",buff[i]);
			}
			fprintf(stderr,"|");
			for(;(i<rode)&&(i<20);i++){
				fprintf(stderr, "%02X|",buff[i]);
			}
			fprintf(stderr,"...%d=>[%d]\n",rode,t_aid);
#endif
		}
	}
}/*}}}*/

void
chaev (ab_t * ab, struct pollfd * fds)
{/*{{{*/
	if(fds[g_status.c1_id].revents){
		if(fds[g_status.c1_id].revents != POLLIN){
			fprintf(stderr,"[%d]: revents: 0x%X\n",
					ab->chans[g_status.c1_id].abs_idx,
					fds[g_status.c1_id].revents);
		}
		/* data in channel 1 */
		rwdata(fds[g_status.c1_id].fd,fds[g_status.c2_id].fd, 
				ab->chans[g_status.c1_id].abs_idx,
				ab->chans[g_status.c2_id].abs_idx);
	}
	if(fds[g_status.c2_id].revents){
		if(fds[g_status.c2_id].revents != POLLIN){
			fprintf(stderr,"[%d]: revents: 0x%X\n",
					ab->chans[g_status.c2_id].abs_idx,
					fds[g_status.c1_id].revents);
		}
		/* data in channel 2 */
		rwdata(fds[g_status.c2_id].fd,fds[g_status.c1_id].fd, 
				ab->chans[g_status.c2_id].abs_idx,
				ab->chans[g_status.c1_id].abs_idx);
	}
}/*}}}*/

void
devact(ab_t * ab, int dev_id)
{/*{{{*/
	ab_dev_t * dev = &ab->devs[dev_id];
	ab_dev_event_t evt;
	unsigned char ca;
	int chan_id;
	int err;

	/* data on device */
	err = ab_dev_event_get(dev, &evt, &ca);
	if(err || !ca || evt.more){
		fprintf(stderr,">> DEV: (%s) [e%d/c%d/m%d/d0x%lX]\n",
				ab_g_err_str, err, ca, evt.more, evt.data);
		return;
	} 

	/* if evt.ch == 0 -> chans[i+1] if 1 -> chans[i] */
	chan_id = evt.ch + dev_id*ab->chans_per_dev;

	if(evt.id == ab_dev_event_FXS_OFFHOOK){
		/* OFFHOOK on some chan */
		err = ab_FXS_line_feed (&ab->chans[chan_id],ab_chan_linefeed_ACTIVE);
		if(err){
			fprintf(stderr,"LFA_%d ERROR",ab->chans[chan_id].abs_idx);
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr,"LFA_%d\n",ab->chans[chan_id].abs_idx);	
		}
		
		if(g_status.c1_is_offhook && g_status.c2_is_offhook){
			/* third actor - ignore event */
			fprintf(stderr,"(offhook) there is no conference "
					"implementation :) [%d]\n",
					chan_id);
		} else if(!g_status.c1_is_offhook && g_status.c2_is_offhook){
			/* c1 was onhook -> it will be the first chan */
			g_status.c1_id = chan_id;
			g_status.c1_is_offhook = 1;
		} else if(!g_status.c1_is_offhook && !g_status.c2_is_offhook){
			/* c1 and c2 was onhook -> it will be the first chan */
			g_status.c1_id = chan_id;
			g_status.c1_is_offhook = 1;
		} else if(g_status.c1_is_offhook && !g_status.c2_is_offhook){
			/* c2 was onhook -> it will be the second chan */
			g_status.c2_id = chan_id;
			g_status.c2_is_offhook = 1;
		}

		if(g_status.c1_is_offhook && g_status.c2_is_offhook){
			err = start_connection (ab);
			if(err){
				fprintf(stderr,"UP ERROR");
				exit(EXIT_FAILURE);
			} else {
				g_status.enc_dec_is_on = 1;
				fprintf(stderr,"UP\n");	
			}
		}
	} else if(evt.id == ab_dev_event_FXS_ONHOOK){
		int conf_err = 0;
		/* ONHOOK on some chan */
		err = ab_FXS_line_feed (&ab->chans[chan_id],ab_chan_linefeed_STANDBY);
		if(err){
			fprintf(stderr,"LFS_%d ERROR",ab->chans[chan_id].abs_idx);
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr,"LFS_%d\n",ab->chans[chan_id].abs_idx);	
		}

		if(chan_id == g_status.c1_id){
			g_status.c1_is_offhook = 0;
		} else if(chan_id == g_status.c2_id){
			g_status.c2_is_offhook = 0;
		} else {
			fprintf(stderr,"(onhook) there is no conference "
					"implementation :) [%d]\n",
					chan_id);
			conf_err = 1;
		}

		if(!conf_err && g_status.enc_dec_is_on){
			err = stop_connection(ab);
			if(err){
				fprintf(stderr,"DOWN ERROR");
				exit(EXIT_FAILURE);
			} else {
				g_status.enc_dec_is_on = 0;
				fprintf(stderr,"DOWN\n");	
			}
		}
	} else if(evt.id == ab_dev_event_FM_CED){
		if(evt.data == 0){
			/*start fax transmitting*/
			err = ab_chan_fax_pass_through_start
					(&ab->chans[chan_id]);
			fprintf(stderr,"ab_chan_fax_pass_through_start() "
					"on [%d] error: %s\n",
					chan_id, ab_g_err_str);
		}
	} else {
		fprintf(stderr,"UNCATCHED EVENT: [%d/%d] (%d| 0x%lX)\n", 
				dev_id, evt.ch, evt.id, evt.data);
	}
}/*}}}*/

void
devev(ab_t * ab, struct pollfd * fds)
{/*{{{*/
	int i;
	int j;

	/* test event for all devices */
	j=g_status.poll_fd_num;
	for (i=ab->chans_num; i<j; i++){
		if(fds[i].revents){
			if(fds[i].revents != POLLIN){
				fprintf(stderr,"revents on dev[%d] is 0x%X\n",
						i-ab->chans_num, fds[i].revents);
			}
			devact(ab, i-ab->chans_num);
		}
	} 
}/*}}}*/

void
start_polling(ab_t * const ab)
{/*{{{*/
	struct pollfd * fds;
	int i;
	int j;

	g_status.poll_fd_num = ab->chans_num + ab->devs_num;
	fds = malloc(sizeof(*fds)*g_status.poll_fd_num);

	j=ab->chans_num;
	for (i=0; i<j; i++){
		fds[i].fd = ab->chans[i].rtp_fd;
		fds[i].events = POLLIN;
	} 
	for(j=0;i<g_status.poll_fd_num;i++,j++){
		fds[i].fd = ab->devs[j].cfg_fd;
		fds[i].events = POLLIN;
	}

	/* poll on chans and dev */
	while(1){
		if(poll(fds, g_status.poll_fd_num, -1) == -1){
			perror("poll : ");
			return;
		}
		/* test events on channels */
		chaev (ab,fds);
		/* test events on devices */
		devev (ab,fds);
	}
}/*}}}*/

int 
main (int argc, char *argv[])
{/*{{{*/
	ab_t * ab;

	int ret;
	ret = startup_init (argc,argv);
	if(ret == 1){
		print_help();
		return 0;
	} else if(ret == -1){
		printf("ERROR!\n");
		print_help();
		return 1;
	}

	startup_print();

	ab = ab_create();
	if(!ab) {
		fprintf(stderr,"ERROR: %s\n", ab_g_err_str);
		return -1;
	}

	/*capabilities_print(ab);*/

	memset(&g_status, 0, sizeof(g_status));
	/*
 	// before set in devev()
	g_status.c1_is_offhook = 0;
	g_status.c2_is_offhook = 0;
	g_status.enc_dec_is_on = 0;
	g_status.c1_id = 0; // to indicate, that it not has been set
	g_status.c2_id = 0;
	*/

	fprintf(stderr,"Iinitialization SUCCESSFUL\n");

	start_polling (ab);

	fprintf(stderr,"THE END: %d:%s\n",ab_g_err_idx, ab_g_err_str);
	ab_destroy(&ab);

	return 0;
}/*}}}*/

