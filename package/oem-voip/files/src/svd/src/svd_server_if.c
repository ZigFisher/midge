/** 
 * @file svd_server_if.c
 * Interface server command functions.
 * It containes executions of interface commands.
 * */ 

/* Includes {{{ */
#include "svd.h"
#include "svd_if.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
/*}}}*/

/** Create interface for svd_if.*/ 
int svd_create_interface(svd_t * svd);
/** Interface handler.*/ 
static int svd_if_handler(su_root_magic_t * root, su_wait_t * w, 
		su_wakeup_arg_t * user_data);
/** Execute given interface message.*/ 
static int svd_exec_msg(svd_t * const svd, struct svdif_msg_s * const msg,
		struct sockaddr_un * const cl_addr);
/** Execute 'test' command.*/ 
static int svd_exec_test(svd_t * const svd, struct svdif_msg_s * const msg, 
		char ** const buff, int * const buff_sz);
/** Execute 'get_jb_stat' command.*/ 
static int svd_exec_get_jb_stat(svd_t * const svd, struct svdif_msg_s * const msg, 
		char ** const buff, int * const buff_sz);
/** Execute 'get_rtcp_stat' command.*/ 
static int svd_exec_get_rtcp_stat(svd_t * const svd, struct svdif_msg_s * const msg, 
		char ** const buff, int * const buff_sz);
/** Add to string another and resize it if necessary */
static int svd_addtobuf(char ** const buf, int * const palc, char const * fmt, ...);
/** Put chan rtcp statistics to buffer */
static int svd_rtcp_for_chan(ab_chan_t * const chan, 
		char ** const buf, int * const palc);
static int svd_jb_for_chan(ab_chan_t * const chan, 
		char ** const buf, int * const palc);
/**
 * Create socket and allocate handler for interface.
 *
 * \param[in] svd 	pointer to svd structure 
 * \retval 0 	etherything is fine
 * \retval -1 	error occures
 * \remark
 * 		svd should be allocated already
 * 		It creates interface socket
 * 		It creates sofia-sip wait object
 * 		It allocates handler for interface socket
 */ 
int 
svd_create_interface(svd_t * svd)
{/*{{{*/
	su_wait_t wait[1];
	char err_msg[ERR_MSG_SIZE];
	int err;

	assert(svd);

	err = svd_if_srv_create(&svd->ifd, err_msg);
	if(err){
		SU_DEBUG_0 (("%s\n",err_msg));
		goto __exit_fail;
	}
	err = su_wait_create (wait, svd->ifd, POLLIN);
	if(err){
		SU_DEBUG_0 ((LOG_FNC_A ("su_wait_create() fails" ) ));
		goto __exit_fail;
	}
	err = su_root_register (svd->root, wait, svd_if_handler, svd, 0);
	if (err == -1){
		SU_DEBUG_0 ((LOG_FNC_A ("su_root_register() fails" ) ));
		goto __exit_fail;
	}

	return 0;
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Read command from the interface and calls appropriate functions.
 *
 * \param[in] 		root 		root object that contain wait object.
 * \param[in] 		w			wait object that emits.
 * \retval -1	if somthing nasty happens.
 * \retval 0 	if etherything is ok.
 */ 
static int 
svd_if_handler(su_root_magic_t * root, su_wait_t * w, su_wakeup_arg_t * user_data)
{/*{{{*/
	unsigned char buf [MAX_MSG_SIZE];
	int received;
	struct svdif_msg_s msg;
	struct sockaddr_un cl_addr;
	int cl_addr_len;
	char err_msg[ERR_MSG_SIZE];
	int err;

	svd_t * svd = user_data;

	assert( svd->ifd != -1 );

	memset(&cl_addr, 0, sizeof(cl_addr));
	cl_addr_len = sizeof(cl_addr);
	
	/* read socket data */
	received = recvfrom (svd->ifd, buf, sizeof(buf), 0, 
			(struct sockaddr * __restrict__)&cl_addr, &cl_addr_len ); 
	if (received == 0){
		SU_DEBUG_2 ((LOG_FNC_A("wrong interface event - no data")));
		goto __exit_fail;
	} else if (received < 0){
		SU_DEBUG_2 (("IF ERROR: recvfrom(): %d(%s)\n", errno, strerror(errno)));
		goto __exit_fail;
	} 
	/* Parse msg and call appropriate function */
	memset(&msg, 0, sizeof(msg));
	err = svd_if_srv_parse (buf, &msg, err_msg);
	if(err){
		SU_DEBUG_2 (("parsing (%s) error: %s\n", buf, err_msg));
		goto __exit_fail;
	}
	err = svd_exec_msg (svd, &msg, &cl_addr);
	if(err){
		goto __exit_fail;
	}
	
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Do the all necessory job on given message.
 *
 * \param[in]	svd		svd.
 * \param[in]	msg		message to execute.
 * \param[in]	cl_addr	address of client socket to answer.
 * \retval -1	if somthing nasty happens.
 * \retval 0 	if etherything is ok.
 */ 
static int
svd_exec_msg(svd_t * const svd, struct svdif_msg_s * const msg,
		struct sockaddr_un * const cl_addr)
{/*{{{*/
	int cnt;
	char * buff = NULL;
	int buff_sz;
	int err;
	
	if       (msg->type == msg_type_TEST){
		err = svd_exec_test(svd, msg, &buff, &buff_sz);
	} else if(msg->type == msg_type_GET_JB_STAT){
		err = svd_exec_get_jb_stat(svd, msg, &buff, &buff_sz);
	} else if(msg->type == msg_type_GET_RTCP_STAT){
		err = svd_exec_get_rtcp_stat(svd, msg, &buff, &buff_sz);
	} else {
		SU_DEBUG_2(("Wrong parsing result type[%d]\n", msg->type));
		goto __exit_fail;
	}
	if(err){
		goto __exit_fail;
	}

	cnt = sendto(svd->ifd, buff, buff_sz, 0, 
			(struct sockaddr * __restrict__)cl_addr, sizeof(*cl_addr));
	if(cnt == -1){
		SU_DEBUG_2(("sending error (%s)\n",strerror(errno)));
		goto __buff_alloc;
	} else if(cnt != buff_sz){
		SU_DEBUG_2(("sending error %d of %d sent\n",cnt,buff_sz));
	}

	free (buff);
	return 0;
__buff_alloc:
	free (buff);
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Executes 'test' command and creates buffer with answer.
 *
 * \param[in]	svd		svd.
 * \param[in]	msg		message to execute.
 * \param[out]	buff	buffer to put the answer.
 * \param[out]	buff_sz	buffer size.
 * \retval -1	if somthing nasty happens.
 * \retval 0 	if etherything is ok.
 * \remark
 *	It allocates memory for buffer with answer. Caller must free this 
 *	memory than he is not need it more.
 */ 
static int
svd_exec_test(svd_t * const svd, struct svdif_msg_s * const msg, 
		char ** const buff, int * const buff_sz)
{/*{{{*/
	goto __exit_fail;
/*{{{ show what come */
#if 0
	fprintf(stderr,"We got message: ");
	if(msg->type == msg_type_NONE){
		fprintf(stderr,"NONE");
	} else if(msg->type == msg_type_TEST){
		fprintf(stderr,"testing");
	} else if(msg->type == msg_type_GET_JB_STAT){
		fprintf(stderr,"jb");
	} else if(msg->type == msg_type_GET_RTCP_STAT){
		fprintf(stderr,"RTCP");
	} else if(msg->type == msg_type_COUNT){
		fprintf(stderr,"COUNT!!!");
	}
	fprintf(stderr,":[");
	if(msg->ch_sel.ch_t == ch_t_NONE){
		fprintf(stderr,"none_chan");
	} else if(msg->ch_sel.ch_t == ch_t_ONE){
		fprintf(stderr,"one_chan=%d",msg->ch_sel.ch_if_one);
	} else if(msg->ch_sel.ch_t == ch_t_ALL){
		fprintf(stderr,"all_chans");
	} else if(msg->ch_sel.ch_t == ch_t_ACTIVE){
		fprintf(stderr,"active_chans");
	} else if(msg->ch_sel.ch_t == ch_t_SPECIFIC){
		fprintf(stderr,"specific_chans");
	}
	fprintf(stderr,";");
	if(msg->fmt_sel == msg_fmt_JSON){
		fprintf(stderr,"json]\n");
	} else if(msg->fmt_sel == msg_fmt_CLI){
		fprintf(stderr,"cli]\n");
	}
#endif
/*}}}*/
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Executes 'get_jb_stat' command and creates buffer with answer.
 *
 * \param[in]	svd		svd.
 * \param[in]	msg		message to execute.
 * \param[out]	buff	buffer to put the answer.
 * \param[out]	buff_sz	buffer size.
 * \retval -1	if somthing nasty happens.
 * \retval 0 	if etherything is ok.
 * \remark
 *	It allocates memory for buffer with answer. Caller must free this 
 *	memory than he is not need it more.
 */ 
static int
svd_exec_get_jb_stat(svd_t * const svd, struct svdif_msg_s * const msg, 
		char ** const buff, int * const buff_sz)
{/*{{{*/
	if       ((msg->fmt_sel == msg_fmt_CLI) || (msg->fmt_sel == msg_fmt_JSON)){
		if       (msg->ch_sel.ch_t == ch_t_ONE){/*{{{*/
			if(svd_jb_for_chan(svd->ab->pchans[msg->ch_sel.ch_if_one], 
					buff,buff_sz)){
				goto __exit_fail;
			}/*}}}*/
		} else if(msg->ch_sel.ch_t == ch_t_ALL){/*{{{*/
			int i;
			int cn = svd->ab->chans_num;
			if(svd_addtobuf(buff, buff_sz,"[\n")){
				goto __exit_fail;
			}
			for (i=0; i<cn-1; i++){
				if(svd_jb_for_chan(&svd->ab->chans[i], buff,buff_sz)){
					goto __exit_fail;
				}
				if(svd_addtobuf(buff, buff_sz,",\n")){
					goto __exit_fail;
				}
			}
			if(svd_jb_for_chan(&svd->ab->chans[i], buff,buff_sz)){
				goto __exit_fail;
			}
			if(svd_addtobuf(buff, buff_sz,"]\n")){
				goto __exit_fail;
			}/*}}}*/
		} else if(msg->ch_sel.ch_t == ch_t_ACTIVE){/*{{{*/
			int i;
			int cn = svd->ab->chans_num;
			if(svd_addtobuf(buff, buff_sz,"[\n")){
				goto __exit_fail;
			}
			for (i=0; i<cn-1; i++){
				if(svd->ab->chans[i].statistics.is_up){
					if(svd_jb_for_chan(&svd->ab->chans[i], buff,buff_sz)){
						goto __exit_fail;
					}
					if(svd_addtobuf(buff, buff_sz,",\n")){
						goto __exit_fail;
					}
				}
			}
			if(svd->ab->chans[i].statistics.is_up){
				if(svd_jb_for_chan(&svd->ab->chans[i], buff,buff_sz)){
					goto __exit_fail;
				}
			}
			if(svd_addtobuf(buff, buff_sz,"]\n")){
				goto __exit_fail;
			}/*}}}*/
		} else if(msg->ch_sel.ch_t == ch_t_SPECIFIC){/*{{{*/
			/* tag__ not released yet *//*}}}*/
		} else {/*{{{*/
			SU_DEBUG_2(("Error in parsing channel num"));
			goto __exit_fail;
		}/*}}}*/
	/*} else if(msg->fmt_sel == msg_fmt_CLI){*/
		/* tag__ not released yet */
	} else {
		SU_DEBUG_2(("Error in parsing output format"));
		goto __exit_fail;
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

/**
 * Executes 'get_rtcp_stat' command and creates buffer with answer.
 *
 * \param[in]	svd		svd.
 * \param[in]	msg		message to execute.
 * \param[out]	buff	buffer to put the answer.
 * \param[out]	buff_sz	buffer size.
 * \retval -1	if somthing nasty happens.
 * \retval 0 	if etherything is ok.
 * \remark
 *	It allocates memory for buffer with answer. Caller must free this 
 *	memory than he is not need it more.
 */ 
static int
svd_exec_get_rtcp_stat(svd_t * const svd, struct svdif_msg_s * const msg, 
		char ** const buff, int * const buff_sz)
{/*{{{*/
	if       ((msg->fmt_sel == msg_fmt_JSON) || (msg->fmt_sel == msg_fmt_CLI)){
		if       (msg->ch_sel.ch_t == ch_t_ONE){/*{{{*/
			if(svd_rtcp_for_chan(svd->ab->pchans[msg->ch_sel.ch_if_one], 
					buff,buff_sz)){
				goto __exit_fail;
			}/*}}}*/
		} else if(msg->ch_sel.ch_t == ch_t_ALL){/*{{{*/
			int i;
			int cn = svd->ab->chans_num;
			if(svd_addtobuf(buff, buff_sz,"[\n")){
				goto __exit_fail;
			}
			for (i=0; i<cn-1; i++){
				if(svd_rtcp_for_chan(&svd->ab->chans[i], buff,buff_sz)){
					goto __exit_fail;
				}
				if(svd_addtobuf(buff, buff_sz,",\n")){
					goto __exit_fail;
				}
			}
			if(svd_rtcp_for_chan(&svd->ab->chans[i], buff,buff_sz)){
				goto __exit_fail;
			}
			if(svd_addtobuf(buff, buff_sz,"]\n")){
				goto __exit_fail;
			}/*}}}*/
		} else if(msg->ch_sel.ch_t == ch_t_ACTIVE){/*{{{*/
			int i;
			int cn = svd->ab->chans_num;
			if(svd_addtobuf(buff, buff_sz,"[\n")){
				goto __exit_fail;
			}
			for (i=0; i<cn-1; i++){
				if(svd->ab->chans[i].statistics.is_up){
					if(svd_rtcp_for_chan(&svd->ab->chans[i], buff,buff_sz)){
						goto __exit_fail;
					}
					if(svd_addtobuf(buff, buff_sz,",\n")){
						goto __exit_fail;
					}
				}
			}
			if(svd->ab->chans[i].statistics.is_up){
				if(svd_rtcp_for_chan(&svd->ab->chans[i], buff,buff_sz)){
					goto __exit_fail;
				}
			}
			if(svd_addtobuf(buff, buff_sz,"]\n")){
				goto __exit_fail;
			}/*}}}*/
		} else if(msg->ch_sel.ch_t == ch_t_SPECIFIC){/*{{{*/
			/* tag__ not released yet *//*}}}*/
		} else {/*{{{*/
			SU_DEBUG_2(("Error in parsing channel num"));
			goto __exit_fail;
		}/*}}}*/
/*	} else if(msg->fmt_sel == msg_fmt_CLI){ */
		/* tag__ not released yet */
	} else {
		SU_DEBUG_2(("Error in parsing output format"));
		goto __exit_fail;
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static int
svd_addtobuf(char ** const buf, int * const palc, char const * fmt, ...)
{/*{{{*/
	va_list ap;
	int n;
	int psz;
	char * nbuf; 

	if(!(*buf)){
		*palc = 300;
		*buf = malloc(*palc);
		if(!(*buf)){
			SU_DEBUG_2((LOG_NOMEM_A("malloc for resizer")));
			printf("no mem for malloc\n");
			goto __exit_fail;
		}    
		memset(*buf, 0, sizeof(**buf));
	}     

	psz = strlen(*buf);

	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		n = vsnprintf((*buf)+psz, (*palc)-psz, fmt, ap);
		va_end(ap);
		/* If that worked, return */
		if (n > -1 && n < (*palc)-psz){
			goto __exit_success;
		}
		if(n < 0){
			/* error */
			SU_DEBUG_2(("vsprintf:%s",strerror(errno)));
			goto __exit_fail;
		} else {
			/* not enough space */
			if ((nbuf = realloc (*buf, (*palc)*2)) == NULL) {
				SU_DEBUG_2(("realloc:%s",strerror(errno)));
				goto __exit_fail;
			}
			*buf = nbuf;
			memset((*buf)+(*palc), 0, sizeof(*palc));
			(*palc) *= 2;
		}
	}
__exit_success:
return 0;
__exit_fail:
return -1;
}/*}}}*/

static int 
svd_rtcp_for_chan(ab_chan_t * const chan, char ** const buf, int * const palc)
{/*{{{*/
	int err;
	char yn[5] = {0,};
	struct ab_chan_rtcp_stat_s const * const s = &chan->statistics.rtcp_stat;
	err = ab_chan_media_rtcp_refresh(chan);
	if(err){
		goto __exit_fail;
	}
	if(chan->statistics.is_up){
		strcpy(yn,"YES");
	} else {
		strcpy(yn,"NO");
	}
	err = svd_addtobuf(buf, palc, 
"{\"chanid\": \"%02d\",\"isUp\":\"%s\",\"con_N\":\"%d\",\"RTCP statistics\":{\n\
\"ssrc\":\"0x%08lX\",\"rtp_ts\":\"0x%08lX\",\"psent\":\"%08ld\",\"osent\":\"%08ld\",\n\
\"fraction\":\"0x%02lX\",\"lost\":\"%08ld\",\"last_seq\":\"%08ld\",\"jitter\":\"0x%08lX\"}}\n", 
	chan->abs_idx,yn,chan->statistics.con_cnt,s->ssrc,s->rtp_ts,s->psent,
	s->osent,s->fraction,s->lost,s->last_seq,s->jitter);
	if(err){
		goto __exit_fail;
	}

	return 0;
__exit_fail:
	return -1;
}/*}}}*/

static int 
svd_jb_for_chan(ab_chan_t * const chan, char ** const buf, int * const palc)
{/*{{{*/
	int err;
	char yn[5] = {0,};
	char tp[5] = {0,}; 
	struct ab_chan_jb_stat_s const * const s = &chan->statistics.jb_stat;
	err = ab_chan_media_jb_refresh(chan);
	if(err){
		goto __exit_fail;
	}
	if(chan->statistics.is_up){
		strcpy(yn,"YES");
	} else {
		strcpy(yn,"NO");
	}
	if(chan->statistics.jb_stat.nType == jb_type_FIXED){
		strcpy(tp,"FIX");
	} else {
		strcpy(tp,"ADP");
	}
	err = svd_addtobuf(buf, palc, 
"{\"chanid\": \"%02d\",\"isUp\":\"%s\",\"con_N\":\"%d\",\"JB statistics\":{\"tp\":\"%s\",\n\
\"PksAvg\":\"%08lu\",\"invPC\":\"%4.2f\",\"latePC\":\"%4.2f\",\"earlyPC\":\"%4.2f\",\"resyncPC\":\"%4.2f\",\n\
\"BS\":\"%04u\",\"maxBS\":\"%04u\",\"minBS\":\"%04u\",\"POD\":\"%04u\",\"maxPOD\":\"%04u\",\"minPOD\":\"%04u\",\n\
\"nPks\":\"%08lu\",\"nInv\":\"%04u\",\"nLate\":\"%04u\",\"nEarly\":\"%04u\",\"nResync\":\"%04u\",\n\
\"nIsUn\":\"%08lu\",\"nIsNoUn\":\"%08lu\",\"nIsIncr\":\"%08lu\",\n\
\"nSkDecr\":\"%08lu\",\"nDsDecr\":\"%08lu\",\"nDsOwrf\":\"%08lu\",\n\
\"nSid\":\"%08lu\",\"nRecvBytesH\":\"%08lu\",\"nRecvBytesL\":\"%08lu\"}}\n", 
	chan->abs_idx,yn,chan->statistics.con_cnt,tp,chan->statistics.pcks_avg,
	chan->statistics.invalid_pc,chan->statistics.late_pc, chan->statistics.early_pc,
	chan->statistics.resync_pc,s->nBufSize,s->nMaxBufSize,s->nMinBufSize,
	s->nPODelay,s->nMaxPODelay,s->nMinPODelay,s->nPackets,s->nInvalid,s->nLate,
	s->nEarly,s->nResync,s->nIsUnderflow,s->nIsNoUnderflow,s->nIsIncrement,
	s->nSkDecrement,s->nDsDecrement,s->nDsOverflow,s->nSid,
	s->nRecBytesH,s->nRecBytesL);
	if(err){
		goto __exit_fail;
	}
	return 0;
__exit_fail:
	return -1;
}/*}}}*/
