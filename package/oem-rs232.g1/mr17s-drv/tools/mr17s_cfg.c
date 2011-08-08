#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <dirent.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#define DEVBASEPATH "/dev/"
#define DEVBASENAME "ttyRS"

#define MXMAGIC 0xAFAF
#define TIOCGHW 0xA002 /* Get HW configuration */
#define TIOCSHW	0xA003 /* Set HW configuration */


typedef struct {
    unsigned short magic;
    unsigned char fctrl : 1;
    unsigned char sigfwd : 1;
} devsetup_t;


void show_node(char *node)
{
    devsetup_t set;
    int fd;
        
    // Open device node
    if( (fd=open(node,O_NONBLOCK)) < 0){
		printf("ERROR: cannot open node %s\n",node);
		return;
    }
    // Test mux ability and get current settings
    set.magic = 0xAFAF;
    if( ioctl(fd,TIOCGHW,(char*)&set,sizeof(set)) ){
    	printf("ERROR: cannot retreive settings for %s",node);
        return;
    }
	printf("%s:\n",node);
	printf("Hardware Flow Control: %s\n",(set.fctrl) ? "enabled" : "disabled");
	printf("Modem Control Signals Forwarding: %s\n",(set.sigfwd) ? "enabled" : "disabled");
}


void
setup_node(char *node,int fctrl,int sigfwd)
{
    devsetup_t set;
    int fd;
        
    // Open device node
    if( (fd=open(node,O_NONBLOCK)) < 0){
		printf("ERROR: cannot open node %s\n",node);
		return;
    }
    // Test mux ability and get current settings
    set.magic = 0xAFAF;
    if( ioctl(fd,TIOCGHW,(char*)&set,sizeof(set)) ){
    	printf("ERROR: cannot retreive settings for %s",node);
        return;
    }

	if( fctrl >= 0 ){
		set.fctrl = fctrl && 1;
	}
	if( sigfwd >= 0 ){
		set.sigfwd = sigfwd && 1;
	}
    set.magic = 0xAFAF;
    if( ioctl(fd,TIOCSHW,(char*)&set,sizeof(set)) ){
    	printf("ERROR: cannot setup %s",node);
        return;
    }
}



int main(int argc, char **argv)
{
	char fctrl = -1, sigfwd = -1;
	char *node = NULL;
	
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
			{"hw-fctrl", 1, 0, 'f'},
			{"sig-fwd", 1, 0, 's'},
			{"node", 1, 0, 'n'},
			{0, 0, 0, 0}
		};
		char *endp;
		int val; 

		int c = getopt_long (argc, argv, "f:s:n:",
							 long_options, &option_index);
        if (c == -1)
    	    break;
		switch (c) {
        case 'f':
			fctrl = strtoul(optarg,&endp,0) && 1;
            break;
        case 's':
			sigfwd = strtoul(optarg,&endp,0) && 1;
    	    break;
		case 'n':
			node = strdup(optarg);
			break;
		}
	}
	
	if( node == NULL ){
		printf("ERROR: no node name\n");
		return 0;
	}

	if( fctrl < 0 && sigfwd < 0 ){
		show_node(node);
		return 0;
	}
	
	setup_node(node,fctrl,sigfwd);
	return 0;
}