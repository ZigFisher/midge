/* Includes {{{*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ab_api.h"
#include "ifx_types.h"
#include "vinetic_io.h"
/*}}}*/

#define DEV_COUNT_MAX 16
#define DEV_NODE_LENGTH 30
#define PRINT_SCALE	10000
#define PRINT_SCALE_STR	"0000"

unsigned long int g_error_counters[DEV_COUNT_MAX] = {0};
unsigned long int g_devs_num = 0;

void
print_it_info(int it_val)
{/*{{{*/
	static unsigned long int scale_counter=0;
	int i;

	if(!(it_val % PRINT_SCALE)){
		scale_counter++;
		fprintf(stderr, "\r[%ld%s] errors:", scale_counter,PRINT_SCALE_STR);
		for(i=0; i<g_devs_num; i++){
			fprintf(stderr, " [%ld]",g_error_counters[i]);
		}
	}
}/*}}}*/

int 
main (int argc, char * const argv[])
{/*{{{*/
	char dnode[DEV_NODE_LENGTH];
	int  cfg_fds[DEV_COUNT_MAX];
	unsigned short int vars[DEV_COUNT_MAX];
	unsigned short int wr_val = 2;
	unsigned long int it_val = 1;
	int i;

	for(i=0; i<DEV_COUNT_MAX; i++){
		int fd;
		memset(dnode, 0, DEV_NODE_LENGTH);
		sprintf(dnode, "/dev/vin%d0",i+1);
		fd = open(dnode,O_RDWR);
		if(fd == -1){
			g_devs_num = i;
			break;
		} else {
			cfg_fds[i] = fd;
		}
	}

	while(1){
		print_it_info(it_val);

		if(!wr_val){
			wr_val+=2;
		}

		for(i=0; i<g_devs_num; i++){
			/* write */
			ioctl(cfg_fds[i], FIO_VINETIC_TCA, wr_val);
			/* read */
			vars[i] = ioctl(cfg_fds[i], FIO_VINETIC_TCA, 0);
		}

		for(i=0; i<g_devs_num; i++){
			/* test values */
			if(vars[i] != wr_val){
				fprintf(stderr,"\n[%ld]DEV_[%d] W/R fail: "
						"Write=0x%04X W/R=0x%04X\n",
						it_val, i, wr_val, vars[i]);
				g_error_counters[i]++;
			}
		}
		it_val++;
		wr_val++;
	}

	for(i=0; i<g_devs_num; i++){
		close(cfg_fds[i]);
	}
	return 0;
}/*}}}*/

