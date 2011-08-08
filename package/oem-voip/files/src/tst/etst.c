#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "ab_api.h"

#define WAIT_INTERVAL 500000
#define EVENTS_PRECLEAN 8

enum action_e {action_PUT, action_GET};
enum ch_e {ch_FXS, ch_FXO};
enum ch_state_e {
	ch_state_NOT_PROCESSED, 
	ch_state_HAVE_NO_PAIR, 
	ch_state_HAVE_PAIR, 
};

struct ch_status_s {
	enum ch_state_e chan_state;
	int pair_idx;
} * g_ch_status = NULL;

int g_verbose = 0;

void
events_clean(ab_chan_t const * const chan)
{/*{{{*/
	ab_dev_event_t evt;
	int err = 0;
	int iter_N = EVENTS_PRECLEAN;

	memset(&evt, 0, sizeof(evt));

	do {
		unsigned char ch_av;
		err = ab_dev_event_get(chan->parent, &evt, &ch_av);
		if(err){
			break;
		}
		iter_N--;
	} while (evt.more && iter_N);

	return;
}/*}}}*/

int
make_a_couple(ab_chan_t const * const chan_fxs, 
		ab_chan_t const * const chan_fxo)
{/*{{{*/
	struct ch_status_s * const fxs_sts = (struct ch_status_s *)chan_fxs->ctx;
	struct ch_status_s * const fxo_sts = (struct ch_status_s *)chan_fxo->ctx;
	enum ch_state_e fxs_s = fxs_sts->chan_state;
	enum ch_state_e fxo_s = fxo_sts->chan_state;

	int cross_couples = 0;
	/* processed chans never been processed before */
	int new_couple = 
			(fxs_s == ch_state_NOT_PROCESSED) &&
			(fxo_s == ch_state_NOT_PROCESSED);
	/* one of processed chans already have
	 * been processed but not included in some couple */
	int fxs_broken_couple = (fxs_s == ch_state_HAVE_NO_PAIR);
	int fxo_broken_couple = (fxo_s == ch_state_HAVE_NO_PAIR);
	/* one of processed chans already belongs to other couple */
	int fxs_cross_couple =
			(fxs_s == ch_state_HAVE_PAIR) &&
			(fxs_sts->pair_idx != chan_fxo->abs_idx);
	int fxo_cross_couple =
			(fxo_s == ch_state_HAVE_PAIR) &&
			(fxo_sts->pair_idx != chan_fxs->abs_idx);

	if(new_couple){
		fxs_sts->chan_state = ch_state_HAVE_PAIR;
		fxo_sts->chan_state = ch_state_HAVE_PAIR;
		fxs_sts->pair_idx = chan_fxo->abs_idx;
		fxo_sts->pair_idx = chan_fxs->abs_idx;
		fprintf(stdout,"FXS[%d] and FXO[%d] is a new couple\n",
				chan_fxs->abs_idx, chan_fxo->abs_idx);
	} else {
		if(fxs_broken_couple) {
			fprintf(stderr,"!ERROR: FXS[%d] didn`t find FXO[%d] "
					"as pair before\n",
					chan_fxs->abs_idx, chan_fxo->abs_idx);
		}
		if(fxo_broken_couple) {
			fprintf(stderr,"!ERROR: FXO[%d] didn`t find FXS[%d] "
					"as pair before\n",
					chan_fxo->abs_idx, chan_fxs->abs_idx);
		}
		if(fxs_cross_couple) {
			fprintf(stderr,"!ERROR: FXS[%d] already in couple with FXO[%d]\n",
					chan_fxs->abs_idx, fxs_sts->pair_idx);
			cross_couples = 1;
		}
		if(fxo_cross_couple) {
			fprintf(stderr,"!ERROR: FXO[%d] already in couple with FXS[%d]\n",
					chan_fxo->abs_idx, fxo_sts->pair_idx);
			cross_couples = 1;
		}
	}
	return cross_couples;
}/*}}}*/

void
linefeed_keeper( ab_t * ab, enum action_e action )
{/*{{{*/
	/** */
	static enum ab_chan_linefeed_e * lf = NULL;
	int i;
	int j;
	int err;

	assert( (!action && !lf) || (action && lf) );

	j = ab->chans_num;

	if(action == action_PUT){
		lf = malloc(sizeof(*lf)* j);
		if( !lf){
			fprintf(stderr,"!ERROR : Not enough memory\n");
			exit(EXIT_FAILURE);
		}
		/* remember linefeeds */
		for (i=0; i<j; i++){
			if(ab->chans[i].parent->type == ab_dev_type_FXS){
				lf[i] = ab->chans[i].status.linefeed;
			}
		}
	} else if (action == action_GET){
		/* restore linefeeds */
		for (i=0; i<j; i++){
			if(ab->chans[i].parent->type == ab_dev_type_FXS){
				err = ab_FXS_line_feed (&ab->chans[i], lf[i]);
				if(err){
					fprintf(stderr,"!ERROR : restore linefeed on [%d]\n",
							ab->chans[i].abs_idx);
					exit(EXIT_FAILURE);
				}
			}
		}
		free(lf);
		lf = NULL;
	}
}/*}}}*/

void
digits_test (ab_chan_t * const cFXO, ab_chan_t * const cFXS, int const pulseDial)
{/*{{{*/
	/* play all digits in tone/pulse mode and find channel that detects it... 
	 * if other event on another chan - error */
	ab_t * ab = cFXO->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int event_acceptor_found = 0;
	int seq_length;
	int devs_num;
	int dial_idx;
	char to_dial[] = {'1','2','3','4','5','6','7','8','9','0',
					'*','#','A','B','C','D','\0'};
	int err;

	seq_length = strlen(to_dial);
	if(pulseDial){
		seq_length = 10;
	}
	for (dial_idx=0; dial_idx<seq_length; dial_idx++){
		/* dial digits all by one */
		err = ab_FXO_line_digit (cFXO, 1, &to_dial [dial_idx], 0, 0, pulseDial);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR: dial a '%c' on FXO[%d]\n",
					to_dial [dial_idx], cFXO->abs_idx);
			return;
		} 

		usleep (WAIT_INTERVAL);
		if(pulseDial){
			usleep (WAIT_INTERVAL * (i/5 + 1));
		}

		devs_num=ab->devs_num;
		for (i=0; i<devs_num; i++){
			/* for each dev - test event presense */
			int chan_idx;
			ab_dev_t * cur_dev = &ab->devs[i];

			err = ab_dev_event_get (cur_dev, &evt, &ca);
			if(err){
				/* error happen on event getting ioctl */
				fprintf(stderr,"!ERROR: dev[%d]: %s\n",i,ab_g_err_str);
				return;
			} else if(evt.id == ab_dev_event_NONE){
				/* no event on current device */
				continue;
			}
			if( !ca){
				/* no channel number available */
				fprintf(stderr,"!ERROR: !ca: dev[%d]: %s\n",i,ab_g_err_str);
				return;
			}
			event_acceptor_found = 1;

			chan_idx = (cur_dev->idx - 1) * ab->chans_per_dev + evt.ch;

			if(ab->chans[chan_idx].abs_idx != cFXS->abs_idx){
				/* got event on channel not in couple */
				fprintf(stderr,"!ERROR: event[%d:0x%lX] on wrong [%d]\n",
						evt.id, evt.data, ab->chans[chan_idx].abs_idx);
				return;
			} else if(
					(!pulseDial && (evt.id != ab_dev_event_FXS_DIGIT_TONE)) ||
					(pulseDial && (evt.id != ab_dev_event_FXS_DIGIT_PULSE)) ||
					((char)evt.data != to_dial[dial_idx])){
				/* wrong event or data */
				fprintf(stderr,"!ERROR: wrong event %d or data '%c' on [%d]\n",
						evt.id, to_dial[dial_idx], ab->chans[chan_idx].abs_idx);
				return;
			} else if(evt.more){
				/* additional events on correct chan */
				fprintf(stderr,"!ERROR: addit. events while diled '%c'on[%d]\n",
						to_dial[dial_idx], ab->chans[chan_idx].abs_idx);
				return;
			}
			if( g_verbose){
				fprintf(stdout,"FXS[%d] <<==<< %s '%c' << FXO[%d]\n",
						ab->chans[chan_idx].abs_idx, 
						pulseDial ? "pulse" : "tone",
						to_dial [dial_idx],
						cFXO->abs_idx);
			}
		}
		if( !event_acceptor_found){
			/* additional events on correct chan */
			fprintf(stderr,"!ERROR FXS[%d] <<=|=<< %s '%c' << FXO[%d]\n",
					cFXS->abs_idx,
					pulseDial ? "pulse" : "tone",
					to_dial [dial_idx],
					cFXO->abs_idx);
		}
	} 
}/*}}}*/

void
fax_test (ab_chan_t * const cFXO, ab_chan_t * const cFXS)
{/*{{{*/
	/* play all digits in tone mode (can`t generate pulse )
	 * and find channel that detects it... if other event on another chan
	 * 	- error */
	ab_t * ab = cFXO->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int event_acceptor_found = 0;
	int seq_length;
	int devs_num;
	int dial_idx;
	char to_dial[] = {'F','m','\0'};
	int err;

	seq_length=strlen(to_dial);
	for (dial_idx=0; dial_idx<seq_length; dial_idx++){
		/* dial digits all by one */
		err = ab_FXS_netlo_play (cFXS, to_dial [dial_idx], 1);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR: play a '%c' on FXS[%d]\n",
					to_dial [dial_idx], cFXS->abs_idx);
			return;
		} 

		usleep (WAIT_INTERVAL);
		usleep (WAIT_INTERVAL);
		usleep (WAIT_INTERVAL);

		devs_num=ab->devs_num;
		for (i=0; i<devs_num; i++){
			/* for each dev - test event presense */
			int chan_idx;
			int error_get;
			ab_dev_t * cur_dev = &ab->devs[i];

			err = ab_dev_event_get (cur_dev, &evt, &ca);
			if(err){
				/* error happen on event getting ioctl */
				fprintf(stderr,"!ERROR: dev[%d]: %s\n",i,ab_g_err_str);
				return;
			} else if(evt.id == ab_dev_event_NONE){
				/* no event on current device */
				continue;
			}
			if( !ca){
				/* no channel number available */
				fprintf(stderr,"!ERROR: !ca: dev[%d]: %s\n",i,ab_g_err_str);
				return;
			}
			event_acceptor_found = 1;

			chan_idx = (cur_dev->idx - 1) * ab->chans_per_dev + evt.ch;

			error_get =
				/* this is TONE event on not generator chan (not FXS)*/
				((evt.id == ab_dev_event_TONE) && 
				 (ab->chans[chan_idx].abs_idx != cFXS->abs_idx)) 
				||
				/* this is CED event on unexpected chan */
				((evt.id == ab_dev_event_FM_CED) &&
				 (ab->chans[chan_idx].abs_idx != cFXO->abs_idx))
				||
				/* this is nor CED, nor TONE event */
				((evt.id != ab_dev_event_FM_CED) &&
				 (evt.id != ab_dev_event_TONE));

			if(error_get){
				fprintf(stderr,"!ERROR: event[%d:0x%lX] on [%d]\n",
						evt.id, evt.data, ab->chans[chan_idx].abs_idx);
				return;
			} 
			if(evt.more){
				/* additional events on correct chan */
				fprintf(stderr,"!ERROR: addit. events while diled '%c' to [%d]\n",
						to_dial[dial_idx], ab->chans[chan_idx].abs_idx);
			}
			if( g_verbose){
				if(evt.id == ab_dev_event_FM_CED){
					fprintf(stdout,"FXO[%d] <<==<< 'CED%s' << FXS[%d]\n",
							ab->chans[chan_idx].abs_idx,
							evt.data? "":"END", cFXS->abs_idx);
				} else if(evt.id == ab_dev_event_TONE){
					fprintf(stdout,"FXS[%d] TONE-(%ld)\n",
							ab->chans[chan_idx].abs_idx, evt.data);
				}
			}
		}
		if( !event_acceptor_found){
			/* additional events on correct chan */
			fprintf(stderr,"!ERROR FXO[%d] <<=|=<< CED/CEDEND << FXS[%d]\n",
					cFXO->abs_idx, cFXS->abs_idx);
		}
	} 
}/*}}}*/

void
process_FXS(ab_chan_t * const chan)
{/*{{{*/
	ab_t * ab = chan->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int j;
	int err;
	int couple_has_been_found = 0;
	int ch;
	int chan_idx;

	/* emits ring and mutes it afrterwords {{{ */
	err = ab_FXS_line_ring(chan, ab_chan_ring_RINGING);
	if( err){
		fprintf(stderr,"!ERROR: FXS[%d]: %s\n", chan->abs_idx,ab_g_err_str);
		return;
	} else if( g_verbose){
		fprintf(stdout,"FXS[%d] >> RING\n",chan->abs_idx);
	}
	usleep (WAIT_INTERVAL);
	err = ab_FXS_line_ring(chan, ab_chan_ring_MUTE);
	if( err){
		fprintf(stderr,"!ERROR: FXS[%d]: %s\n", chan->abs_idx,ab_g_err_str);
		return;
	} else if( g_verbose){
		fprintf(stdout,"FXS[%d] Stops RINGING\n",chan->abs_idx);
	}/*}}}*/

	usleep (WAIT_INTERVAL);

	/* test all devices on event get to find a couple (it can exists) {{{*/ 
	j=ab->devs_num;
	for (i=0; i<j; i++){
		ab_dev_t * cur_dev = &ab->devs[i];
		err = ab_dev_event_get (cur_dev, &evt, &ca);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR: dev[%d]: %s\n",i,ab_g_err_str);
			return;
		} else if(evt.id == ab_dev_event_NONE){
			/* no event on current device */
			continue;
		}
		if( !ca){
			/* no channel number available */
			fprintf(stderr,"!ERROR !ca : dev[%d] : %s\n",i,ab_g_err_str);
			return;
		}

		ch = evt.ch;
		chan_idx = i * ab->chans_per_dev + evt.ch;

		/* got right event on right device type */
		if(	evt.id == ab_dev_event_FXO_RINGING &&
				cur_dev->type == ab_dev_type_FXO){
			if( g_verbose){
				fprintf(stdout,"FXO[%d] << RING (START or STOP)\n",
						ab->chans[chan_idx].abs_idx);
			}
			/* make them a couple */
			err = make_a_couple(chan, &ab->chans[chan_idx]);
			if(err){
				/* cross_couples find */
				return;
			} 
			couple_has_been_found = 1;

			/* parse other events on this device */
			while(evt.more){
				err = ab_dev_event_get(cur_dev, &evt, &ca);
				if(err){
					fprintf(stderr,"!ERROR : %s\n",ab_g_err_str);
					return;
				}
				chan_idx = i * ab->chans_per_dev + evt.ch;
				/* event on unexpected channel */
				if(evt.ch != ch){
					fprintf(stderr,"!ERROR Event on wrong channel [%d]\n",
							ab->chans[chan_idx].abs_idx);
					return;
				}
				/* unexpected event on channel in couple */
				if(evt.id != ab_dev_event_FXO_RINGING){
					fprintf(stderr,"!ERROR Wrong event [%d/0x%lX] on FXO[%d]\n",
							evt.id, evt.data, ab->chans[chan_idx].abs_idx);
					return;
				}
				if( g_verbose){
					fprintf(stdout,"FXO[%d] << RING (START or STOP)\n",
							ab->chans[chan_idx].abs_idx);
				}
			}
		} else {
			fprintf(stderr,"!ERROR FXS[%d] >> RING and [%d] << [%d/0x%lX]\n",
					chan->abs_idx, ab->chans[chan_idx].abs_idx,
					evt.id, evt.data);
			return;
		}
	}/*}}}*/
	/* if no couple - tell about it {{{*/
	if( !couple_has_been_found){
		struct ch_status_s * const sts = (struct ch_status_s * )chan->ctx;
		if(sts->chan_state == ch_state_HAVE_PAIR){
			fprintf(stdout,"!ERROR FXO[%d] <<=|=<< RING FXS[%d]\n", 
					sts->pair_idx, chan->abs_idx);
		} else {
			fprintf(stdout,"!ATT no couple found for FXS[%d]\n", chan->abs_idx);
			sts->chan_state = ch_state_HAVE_NO_PAIR;
		}
	}/*}}}*/
}/*}}}*/

void
process_FXO(ab_chan_t * const chan)
{/*{{{*/
	ab_t * ab = chan->parent->parent;
	ab_dev_event_t evt;
	unsigned char ca;
	int i;
	int j;
	int err;
	int couple_has_been_found = 0;
	int couple_idx = -1;
	int chan_idx;
	int ch;

	/* emits offhook {{{*/
	err = ab_FXO_line_hook(chan, ab_chan_hook_OFFHOOK);
	if( err){
		fprintf(stderr,"!ERROR: FXO[%d]: %s\n",
				chan->abs_idx,ab_g_err_str);
		return;
	} else if( g_verbose){
		fprintf(stdout,"FXO[%d] >> OFFHOOK\n",chan->abs_idx);
	}
	/*}}}*/

	usleep (WAIT_INTERVAL);

	/* test all devices on event get to find a couple (it can exists) {{{*/
	j=ab->devs_num;
	for (i=0; i<j; i++){
		ab_dev_t * cur_dev = &ab->devs[i];
		err = ab_dev_event_get (cur_dev, &evt, &ca);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR: dev[%d]: %s\n",i,ab_g_err_str);
			return;
		} else if(evt.id == ab_dev_event_NONE){
			/* no event on current device */
			continue;
		}
		if( !ca){
			/* no channel number available */
			fprintf(stderr,"!ERROR !ca: dev[%d]: %s\n",i,ab_g_err_str);
			return;
		}

		ch = evt.ch;
		chan_idx = i * ab->chans_per_dev + evt.ch;
		couple_idx = chan_idx;

		/* got right event on right device type */
		if(	evt.id == ab_dev_event_FXS_OFFHOOK &&
				cur_dev->type == ab_dev_type_FXS){
			if( g_verbose){
				fprintf(stdout,"FXS[%d] << OFFHOOK\n",
						ab->chans[chan_idx].abs_idx);
			}
			/* make them a couple */
			err = make_a_couple (&ab->chans[chan_idx], chan);
			if(err){
			 /* channels in differ couples */
				return;
			} 
			couple_has_been_found = 1;

			/* parse other events on this device */
			/* if they exist -> error */
			if(evt.more){
				err = ab_dev_event_get(cur_dev, &evt, &ca);
				if(err){
					fprintf(stderr,"!ERROR: %s\n",ab_g_err_str);
					return;
				}
				chan_idx = i * ab->chans_per_dev + evt.ch;
				
				/* event on unexpected channel */
				if(evt.ch != ch){
					fprintf(stderr,"!ERROR Wrong event [%d/0x%lX] on "
							"wrong FXS[%d]\n",
							evt.id, evt.data,
							ab->chans[chan_idx].abs_idx);
					return;
				}
				/* unexpected event on channel in couple */
				fprintf(stderr,"!ERROR Wrong event [%d/0x%lX] on FXS[%d]\n",
						evt.id, evt.data,
						ab->chans[chan_idx].abs_idx);
				return;
			}
		} else {
			fprintf(stderr,"!ERROR FXS[%d] >> OFFHOOK and [%d] << [%d/0x%lX]\n",
					chan->abs_idx, ab->chans[chan_idx].abs_idx,
					evt.id, evt.data);
			return;
		}
	} /*}}}*/
	/* if no couple - tell about it {{{*/
	if( !couple_has_been_found){
		struct ch_status_s * const sts = (struct ch_status_s * )chan->ctx;
		if(sts->chan_state == ch_state_HAVE_PAIR){
			fprintf(stdout,"!ERROR FXS[%d] <<=|=<< OFFHOOK FXO[%d]\n", 
					sts->pair_idx, chan->abs_idx);
			fprintf(stdout,"!=>!ATT SKIP digits test on FXS[%d] and FXO[%d]\n", 
					sts->pair_idx, chan->abs_idx);
		} else {
			fprintf(stdout,"!ATT no couple found for FXO[%d]\n", chan->abs_idx);
			sts->chan_state = ch_state_HAVE_NO_PAIR;
		} /*}}}*/
	} else {
		/* then emits digits and onhook for this chan {{{*/
		/* remember the last state and set linefeed to active on all FXSchans */
		linefeed_keeper (ab, action_PUT);
		j=ab->chans_num;
		for (i=0; i<j; i++){
			if(ab->chans[i].parent->type == ab_dev_type_FXS){
				err = ab_FXS_line_feed(&ab->chans[i],ab_chan_linefeed_ACTIVE);
				if(err){
					fprintf(stderr,"!ERROR: set lfeed to active on FXS[%d]\n",
							ab->chans[i].abs_idx);
					return;
				}
			}
		} 

		/* play digits and test events on couple chan */
		digits_test(chan, &ab->chans[couple_idx], 0);
		digits_test(chan, &ab->chans[couple_idx], 1);

		/* play fax event on couple chan and test events on fxo */
		fax_test(chan, &ab->chans[couple_idx]);

		/* onhook on the tested chan */
		err = ab_FXO_line_hook(chan, ab_chan_hook_ONHOOK);
		if(err){
			fprintf(stderr,"!ERROR: set hook to onhook [%d]\n",
					ab->chans[couple_idx].abs_idx);
			return;
		} else if( g_verbose){
			fprintf(stdout,"FXO[%d] >> ONHOOK\n",chan->abs_idx);
		}

		/* get onhook from couple */
		usleep(WAIT_INTERVAL);
		err = ab_dev_event_get (ab->chans[couple_idx].parent, &evt, &ca);
		if(err){
			/* error happen on event getting ioctl */
			fprintf(stderr,"!ERROR: chan[%d]: %s\n",
					ab->chans[couple_idx].abs_idx,ab_g_err_str);
			return;
		} else if(evt.id == ab_dev_event_NONE){
			/* no event on current device */
			fprintf(stderr,"!ERROR: FXS[%d] <<=|=<< ONHOOK\n",
					ab->chans[couple_idx].abs_idx);
			return;
		}
		if( !ca){
			/* no channel number available */
			fprintf(stderr,"!ERROR: !ca: FXS[%d]: %s\n",
					ab->chans[couple_idx].abs_idx,ab_g_err_str);
			return;
		}
		chan_idx = (ab->chans[couple_idx].parent->idx - 1) * 
				ab->chans_per_dev + evt.ch;

		if((chan_idx != couple_idx) || (evt.id != ab_dev_event_FXS_ONHOOK)){
			/* got unexpected event or(and) the unexpected channel */
			fprintf(stderr,"!ERROR: event [%d/0x%lX] on channel [%d]\n",
					evt.id, evt.data,
					ab->chans[chan_idx].abs_idx);
			return;
		} else if( g_verbose){
			fprintf(stdout,"FXS[%d] << ONHOOK\n",
					ab->chans[chan_idx].abs_idx);
		}

		/* set linefeed to previous on all FXSchans */
		linefeed_keeper (ab, action_GET);
	}/*}}}*/
}/*}}}*/

int 
main (int argc, char *argv[])
{/*{{{*/
	ab_t * ab;
	int i;
	int j;

	if(argc==2){
		if( !strcmp (argv[1], "-v")){
			g_verbose = 1;
		} else {
			fprintf(stdout,"use -v for verbosity output\n");
			exit (EXIT_SUCCESS);
		}
	}

	/* init ab */
	ab = ab_create();
	if( !ab){
		fprintf(stderr,"!ERROR : %s\n",ab_g_err_str);
		exit (EXIT_FAILURE);
	}

	g_ch_status = malloc(sizeof(*g_ch_status)*ab->chans_num);
	if( !g_ch_status){
		fprintf(stderr,"!ERROR : Not enough memory for ch_status\n");
		ab_destroy(&ab);
		exit (EXIT_FAILURE);
	}

	usleep (WAIT_INTERVAL);

	j=ab->chans_num;
	for (i=0; i<j; i++){
		ab_chan_t * curr_chan = &ab->chans[i];
		/* for initial onhook events from FXO channels */
		events_clean(curr_chan);
		/* init all channels context and state of it */
		curr_chan->ctx = &g_ch_status[i];
		g_ch_status[i].chan_state = ch_state_NOT_PROCESSED;
	}

	usleep (WAIT_INTERVAL);

	/* for every chan */
	j=ab->chans_num;
	for (i=0; i<j; i++){
		ab_chan_t * curr_chan = &ab->chans[i];
		if(curr_chan->parent->type == ab_dev_type_FXS){
			/* if FXS - process FXS */
			process_FXS(curr_chan);
		} else if(curr_chan->parent->type == ab_dev_type_FXO){
			/* if FXO - process FXO */
			process_FXO(curr_chan);
		}
	}

	/* destroy ab */
	ab_destroy(&ab);

	/* free g_cps memory */
	free (g_ch_status);

	return 0;
}/*}}}*/

