				/*
				IFX_boolean_t hook;
				err = ioctl (ab->chans[2].rtp_fd,
					IFX_TAPI_FXO_HOOK_GET, &hook);
				*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "ifx_types.h"
#include "drv_tapi_io.h"
#include "vinetic_io.h"
#include "ab_api.h"
#include "ab_ioctl.h"

#define FXS_CHAN_ID 2
#define FXO_CHAN_ID 0

int main (int argc, char *argv[])
{
	ab_t * ab;
	int choise;
	char str[ 10 ];
	int err = 1234;

	ab = ab_create();
	if(!ab) {
		fprintf(stderr,"Iinitialization FAILED\n");
		fprintf(stderr,"ERROR: %s\n", ab_g_err_str);
		return -1;
	}

	fprintf(stderr,"Iinitialization SUCCESSFUL\n");

	choise = 0;
	while(1) {
		printf("\n");
		printf("[1] Event test.\n");
		printf("======== FXS ========\n");
		printf("[21/22]        Ring     START / STOP.\n");
		printf("[23/24/25/26]  PLAY     DIAL / BUSY / RINGBACK / MUTE.\n");
		printf("[27/28/29]     LINEFEED DIS / STNDBY / ACT.\n");
		printf("======== FXO ========\n");
		printf("[31/32]        OFFHOOK / ONHOOK.\n");
		printf("[33/34]        TONE / PULSE DIAL #*1234.\n");
		printf("[35/36]        TONE / PULSE DIAL 567890.\n");
		printf("[37/38]        TONE / PULSE DIAL ABCD.\n");
		printf("=====================\n");
		printf("[0] Exit.\n");
		printf("=> ");
		fgets(str, 9, stdin);
		choise = strtol(str, NULL, 10);
		switch(choise){
			case 1: {
				ab_dev_event_t evt;

		fprintf(stderr,"\t---=== FXS_DEVICE_EVENTS ===---\n");	
				do {
					unsigned char chan_avail;
					err = ab_dev_event_get( 
							ab->chans[FXS_CHAN_ID].parent, &evt, &chan_avail);
					if( !chan_avail){
						fprintf(stderr,"No CHAN AVAILABLE\n");
						break;
					}
		switch(evt.id){
			case ab_dev_event_NONE: {
				fprintf(stderr,"NONE\n");
				break;
			}	
			case ab_dev_event_UNCATCHED: {
				fprintf(stderr,"[%d]-UNCATCHED : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXO_RINGING: {
				fprintf(stderr,"[%d]-FXO_RINGING : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_TONE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_TONE : '%c'\n", 
						evt.ch, (unsigned char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_PULSE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_PULSE : '%c'\n", 
						evt.ch, (char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_ONHOOK: {
				fprintf(stderr,"[%d]-FXS_ONHOOK\n", evt.ch);	
				break;
			}	
			case ab_dev_event_FXS_OFFHOOK: {
				fprintf(stderr,"[%d]-FXS_OFFHOOK\n", evt.ch);	
				break;
			}	
			case ab_dev_event_FM_CED: {
				fprintf(stderr,"[%d]-FXS_FM_CED\n", evt.ch);	
				break;
			}	
			case ab_dev_event_COD: {
				fprintf(stderr,"[%d]-COD\n", evt.ch);	
				break;
			}	
			case ab_dev_event_TONE: {
				fprintf(stderr,"[%d]-TONE\n", evt.ch);	
				break;
			}	
		}
				} while(evt.more);

		fprintf(stderr,"\t---=========================---\n");	
		fprintf(stderr,"\t---=== FXO_DEVICE_EVENTS ===---\n");	
				do {
					unsigned char chan_avail;
					err = ab_dev_event_get( 
							ab->chans[FXO_CHAN_ID].parent, &evt, &chan_avail);
					if( !chan_avail){
						fprintf(stderr,"No CHAN AVAILABLE\n");
						break;
					}
		switch(evt.id){
			case ab_dev_event_NONE: {
				fprintf(stderr,"NONE\n");
				break;
			}	
			case ab_dev_event_UNCATCHED: {
				fprintf(stderr,"[%d]-UNCATCHED : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXO_RINGING: {
				fprintf(stderr,"[%d]-FXO_RINGING : 0x%lX\n", 
						evt.ch, evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_TONE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_TONE : '%c'\n", 
						evt.ch, (char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_DIGIT_PULSE: {
				fprintf(stderr,"[%d]-FXS_DIGIT_PULSE : '%c'\n", 
						evt.ch, (char)evt.data);	
				break;
			}	
			case ab_dev_event_FXS_ONHOOK: {
				fprintf(stderr,"[%d]-FXS_ONHOOK\n", evt.ch);	
				break;
			}	
			case ab_dev_event_FXS_OFFHOOK: {
				fprintf(stderr,"[%d]-FXS_OFFHOOK\n", evt.ch);
				break;
			}	
			case ab_dev_event_FM_CED: {
				fprintf(stderr,"[%d]-FXS_FM_CED\n", evt.ch);	
				break;
			}	
			case ab_dev_event_COD: {
				fprintf(stderr,"[%d]-COD\n", evt.ch);	
				break;
			}	
			case ab_dev_event_TONE: {
				fprintf(stderr,"[%d]-TONE\n", evt.ch);	
				break;
			}	
		}
				} while(evt.more);
		fprintf(stderr,"\t---=========================---\n");	
			err = ioctl(ab->chans[FXO_CHAN_ID].rtp_fd, 
						IFX_TAPI_TONE_CPTD_STOP, 0); // stop detect tones
			fprintf(stderr,"stop detect tones on FXS err : %d\n",err);
				break;
			}

			case 21: {
 				err = ab_FXS_line_ring( &ab->chans[FXS_CHAN_ID], 
						ab_chan_ring_RINGING );
				fprintf(stderr,"ERR = %d\n",err);	

				break;
			}
			case 22: {
 				err = ab_FXS_line_ring( &ab->chans[FXS_CHAN_ID], 
						ab_chan_ring_MUTE );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 23: {
				IFX_TAPI_TONE_CPTD_t cpt;
				memset (&cpt, 0, sizeof(cpt));

				cpt.tone = 25; //dial tone
				/* Tone to be detected in transmit direction (typical for FXO) */
				cpt.signal = IFX_TAPI_TONE_CPTD_DIRECTION_TX;
				/* Start CPT detector */
				err = ioctl(ab->chans[FXO_CHAN_ID].rtp_fd, 
						IFX_TAPI_TONE_CPTD_START, &cpt);
				fprintf(stderr,"start dialtone detection ERR = %d\n",err);

 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_DIAL);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 24: {
				IFX_TAPI_TONE_CPTD_t cpt;
				memset (&cpt, 0, sizeof(cpt));

				cpt.tone = 27; //busy tone
				/* Tone to be detected in transmit direction (typical for FXO) */
				cpt.signal = IFX_TAPI_TONE_CPTD_DIRECTION_TX;
				/* Start CPT detector */
				err = ioctl(ab->chans[FXO_CHAN_ID].rtp_fd, 
						IFX_TAPI_TONE_CPTD_START, &cpt);
				fprintf(stderr,"start busytone detection ERR = %d\n",err);

 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_BUSY);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 25: {
IFX_TAPI_TONE_t tone;
memset(&tone, 0, sizeof(tone));
/* define a simple tone for tone table index 71 */
tone.simple.format = IFX_TAPI_TONE_TYPE_SIMPLE;
tone.simple.index = 71;
/* using two frequencies */
/* 0 <= till < 4000 Hz */
tone.simple.freqA = 480;
tone.simple.freqB = 620;
/* tone level for freqA */
/* -300 < till < 0 */
tone.simple.levelA = -11;
/* tone level for freqB */
tone.simple.levelB = -9;
/* program first cadences (on time) */
tone.simple.cadence[0] = 2000;
/* program second cadences (off time) */
tone.simple.cadence[1] = 2000;
/* in the first cadence, both frequencies must be played */
tone.simple.frequencies[0] = IFX_TAPI_TONE_FREQA | IFX_TAPI_TONE_FREQB;
/* in the second cadence, all frequencies are off */
tone.simple.frequencies[1] = IFX_TAPI_TONE_FREQA | IFX_TAPI_TONE_FREQB;
//tone.simple.frequencies[1] = IFX_TAPI_TONE_FREQNONE;
/* the tone will be played two times (2 loops) */
tone.simple.loop = 2;
/* at the end of each loop there is a pause */
//tone.simple.pause = 200;
tone.simple.pause = 0;
/* update the tone table with the simple tone */
err = ioctl(ab->chans[FXO_CHAN_ID].rtp_fd, IFX_TAPI_TONE_TABLE_CFG_SET, &tone);
fprintf(stderr,"set new tone to FXO table ERR = %d\n",err);
err = ioctl(ab->chans[FXS_CHAN_ID].rtp_fd, IFX_TAPI_TONE_TABLE_CFG_SET, &tone);
fprintf(stderr,"set new tone to FXS table ERR = %d\n",err);
/* now the simple tone is added to the tone table at index 71 */

				IFX_TAPI_TONE_CPTD_t cpt;
				IFX_TAPI_LINE_VOLUME_t vol;
				memset (&cpt, 0, sizeof(cpt));
				memset (&vol, 0, sizeof(vol));


				//cpt.tone = 26; //ringing tone
				cpt.tone = 71; //ringing tone
				/* Tone to be detected in transmit direction (typical for FXO) */
				cpt.signal = IFX_TAPI_TONE_CPTD_DIRECTION_TX;
				/* Start CPT detector */
				err = ioctl(ab->chans[FXO_CHAN_ID].rtp_fd, 
						IFX_TAPI_TONE_CPTD_START, &cpt);
				fprintf(stderr,"start ringingtone detection ERR = %d\n",err);

				err = ioctl(ab->chans[FXS_CHAN_ID].rtp_fd, 
					IFX_TAPI_LINE_LEVEL_SET, 0); //en(1)(dis-0)able
				fprintf(stderr,"line high level enable ERR = %d\n",err);

				vol.nGainRx = -4;
				vol.nGainTx = -4;
				err = ioctl(ab->chans[FXS_CHAN_ID].rtp_fd, 
					IFX_TAPI_PHONE_VOLUME_SET, &vol); 
				fprintf(stderr,"line volume set ERR = %d\n",err);

				/* play the tone 71 */
				err = ioctl( ab->chans[FXS_CHAN_ID].rtp_fd, 
						IFX_TAPI_TONE_LOCAL_PLAY, 71);

				sleep (4);
				err = ioctl( ab->chans[FXS_CHAN_ID].rtp_fd, 
						IFX_TAPI_TONE_LOCAL_PLAY, 0);

				vol.nGainRx = 0;
				vol.nGainTx = 0;
				err = ioctl(ab->chans[FXS_CHAN_ID].rtp_fd, 
					IFX_TAPI_PHONE_VOLUME_SET, &vol); 
				fprintf(stderr,"line volume set ERR = %d\n",err);


 				//err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
				//		ab_chan_tone_RINGBACK);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 26: {
 				err = ab_FXS_line_tone( &ab->chans[FXS_CHAN_ID], 
						ab_chan_tone_MUTE);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 27: {
				err = ab_FXS_line_feed( &ab->chans[FXS_CHAN_ID], 
						ab_chan_linefeed_DISABLED );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 28: {
				err = ab_FXS_line_feed( &ab->chans[FXS_CHAN_ID], 
						ab_chan_linefeed_STANDBY );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 29: {
				err = ab_FXS_line_feed( &ab->chans[FXS_CHAN_ID], 
						ab_chan_linefeed_ACTIVE );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 31: {
				err = ab_FXO_line_hook( &ab->chans[FXO_CHAN_ID], 
						ab_chan_hook_OFFHOOK);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 32: {
				err = ab_FXO_line_hook( &ab->chans[FXO_CHAN_ID], 
						ab_chan_hook_ONHOOK);
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 33: 
			case 34: {
				char data[] = {'#','*','1','2','3','4'};
				int pulse = 0;
				if(choise == 34){
					pulse = 1;
				}
				err = ab_FXO_line_digit( &ab->chans[FXO_CHAN_ID],
						6, data, 100,100, pulse );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 35: 
			case 36: {
				char data[] = {'5','6','7','8','9','0'};
				int pulse = 0;
				if(choise == 36){
					pulse = 1;
				}
				err = ab_FXO_line_digit( &ab->chans[FXO_CHAN_ID],
						6, data, 100,100, pulse );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 37: 
			case 38: {
				char data[] = {'A','B','C','D'};
				int pulse = 0;
				if(choise == 38){
					pulse = 1;
				}
				err = ab_FXO_line_digit( &ab->chans[FXO_CHAN_ID],
						4, data, 100,100, pulse );
				fprintf(stderr,"ERR = %d\n",err);	
				break;
			}
			case 0: {
				goto main_exit;
			}
			default: {
				printf("Error: enter the valid value!\n");
			}
		}
	}

main_exit:
	fprintf(stderr,"THE END : %d : %s\n",ab_g_err_idx, ab_g_err_str);
	ab_destroy(&ab);
	return 0;
};

