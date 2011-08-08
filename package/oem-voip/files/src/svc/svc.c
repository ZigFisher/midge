/* INCLUDES {{{*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include "ab_api.h"
/*}}}*/

#define DAEMON_NAME    "svc"
/** Test svd presence in this interval. */
#define CHECK_FRQ_SEC   3
/** After this count of missing svd will clean the channels. */
#define CHECK_MISS_CNT  3
/** This value and less means that no svd is up. */
#define CHECK_MISS_VAL  3
/** Wait at start to give chance to svd to start up. */
#define WAIT_AT_START  10


/** 
 * Switch main process to daemon mode.
 *
 * \param[in] close_stderr for debug - should daemon close stderr or not.
 * 
 * \retval 0 	etherything is fine
 * \retval -1 	error occures
 * \remark
 * 		Error messages will pass to stderr if debug is on, 
 * 		otherwise, stderr redirect to /dev/null as stdout and stdin
 */ 
static int
daemonize (int close_stderr)
{/*{{{*/
	pid_t pid; 
	pid_t sid; 

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "unable to fork daemon, code=%d (%s)\n",
				errno, strerror(errno));
		goto __exit_fail;
	}
	
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* At this point we are executing as the child process */

	/* Cancel certain signals */
	signal(SIGCHLD,SIG_DFL); /* A child process dies */
	signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM,SIG_DFL); /* Die on SIGTERM */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		fprintf(stderr,"unable to create a new session, code %d (%s)\n",
				errno, strerror(errno));
		goto __exit_fail;
	}

	/* Change the current working directory.  This prevents the current
	directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0) {
		fprintf(stderr,"unable to change directory to %s, "
				"code %d (%s)\n",
				"/", errno, strerror(errno));
		goto __exit_fail;
	}

	/* Redirect standard files to /dev/null */
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	if( close_stderr){
		freopen( "/dev/null", "w", stderr);
	}

	return 0;

__exit_fail:
	return -1;
}/*}}}*/

/** 
 * Cleares and proper close all channels.
 *
 * \param[out] cr clear_results - pointer to massive int[CHANS_MAX].
 * 
 * \retval 0 	etherything is fine
 * \retval -1 	error occures
 */ 
static int
close_and_clear (int * const cr)
{/*{{{*/
	int i;
	int chans_num;
	char rbuf[1000];
	int rode;

	ab_t * ab = NULL;

	ab = ab_create();
	if(!ab){
		syslog( LOG_INFO, ab_g_err_str );
		goto __exit_fail;
	}

	memset(cr,0,sizeof(int)*CHANS_MAX);

	chans_num = ab->chans_num;
	for (i=0; i<chans_num; i++){
		/* start generate rtp */
		ab_chan_media_switch (&ab->chans[i],1);
		/* read all data from rtp_stream */
		rode = 1000;
		while(rode == 1000){
			rode = read(ab->chans[i].rtp_fd,rbuf,1000);
			if(rode == -1){
				/* if buf is smaller, then the rtp data
				 * we got an error EINVAL */
				break;
			}
			cr[ab->chans[i].abs_idx] += rode;
		}
		/* stop generate rtp */
		ab_chan_media_switch (&ab->chans[i],0);
	} 
	for (i=0; i<chans_num; i++){
		/* start generate rtp */
		ab_chan_media_switch (&ab->chans[i],1);
		/* read all data from rtp_stream */
		rode = 1000;
		while(rode == 1000){
			rode = read(ab->chans[i].rtp_fd,rbuf,1000);
			if(rode == -1){
				/* if buf is smaller, then the rtp data
				 * we got an error EINVAL */
				break;
			}
			cr[ab->chans[i].abs_idx] += rode;
		}
		/* stop generate rtp */
		ab_chan_media_switch (&ab->chans[i],0);
	} 

	/* tag__ looks ugly, but it works :)
	 * if we clean channels just ones - some data are will be in channels on
	 * the svd start.
	 * */

	ab_destroy (&ab);

	return 0;
__exit_fail:
	return -1;
}/*}}}*/

int 
main( int argc, char *argv[] ) 
{/*{{{*/
	FILE * fd;
	char buf[20] = {0};
	int clear_rez[CHANS_MAX];
	int miss_count = 0;
	int clearing_done = 0;
	int i;
	int err;

	if(argc == 1){
		/* normal case */
	} else if((argc == 2) && (!strcmp(argv[1],"--clear-and-exit"))){
		return close_and_clear (clear_rez);
	} else {
		fprintf(stderr,"%s: just one option available - \"--clear-and-exit\"\n",
				argv[0]);
		return -1;
	}

	/* Initialize the logging interface */
	openlog( DAEMON_NAME, LOG_PID, LOG_LOCAL5 );
	syslog( LOG_INFO, "starting" );

	/* One may wish to process command line arguments here */

	/* Daemonize */
	daemonize (0);

	syslog( LOG_INFO, "waiting for svd.." );
	sleep(WAIT_AT_START);
	syslog( LOG_INFO, "START polling SVD process");

	while(1){/*{{{*/
		fd = popen("ps ax | grep svd | wc -l","r");
		fread (buf,sizeof(buf),1,fd);
		pclose(fd);

		if(strtol(buf,NULL,10) <= CHECK_MISS_VAL){
			/* no svd running */
			if(!clearing_done){
				/* it is first check after svd drop */
				if((++miss_count) == CHECK_MISS_CNT){
					/* close channels and clear rtp-traffic */
					syslog( LOG_NOTICE, "close_and_clear ACTIVATED" );
					err = close_and_clear (clear_rez);
					if(err){
						syslog( LOG_NOTICE, "close_and_clear ERROR" );
					} else {
						clearing_done = 1;
						/* print clear info */
						for (i=0; i<CHANS_MAX; i++){
							if(clear_rez[i]){
								char infob[50];
								snprintf(infob,sizeof(infob),
										">> %d cleared in [%d]\n",clear_rez[i],i);
								syslog( LOG_NOTICE, infob );
							}
						}
					}
					miss_count = 0;
				}
			}
		} else {
			clearing_done = 0;
			miss_count = 0;
		}
		sleep(CHECK_FRQ_SEC);
	}/*}}}*/

	/* Finish up */
	syslog( LOG_NOTICE, "terminated" );
	closelog();
	return 0;
}/*}}}*/
