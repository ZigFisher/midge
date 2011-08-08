#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <malloc.h>
#include <string.h>

#include "mxconfig.h"
#include "debug.h"

#ifndef ARCH
#define IFS_ROOT "/home/artpol/work/tmp/ifs-root"
#else
#define IFS_ROOT "/sys/class/net"
#endif

int debug_lev=DFULL;

typedef enum { help,setup,info,full_info,chck } action_t;

void print_usage(char *name)
{
    printf("Usage: %s [Options]\n"
		   "Options:\n"
		   "  -i, --iface\tinterface name\n"
		   "  -l, --list\tdevice multiplexing settings\n"
		   "  -c, --check\tcheck multiplexing configuration\n"
		   "  -r, --rline\tTransmit multiplexer bus line\n"
		   "  -t, --tline\tReceive multiplexer bus line\n"
		   "  -a, --mxrate\tSet multiplexing rate (use for SHDSL)\n"
		   "  -o, --mxsmap\tSlotmap used for multiplexing (use for E1)\n"
		   "  -j, --tfs\tTransmit Frame Start (TFS)\n"
		   "  -g, --rfs\tReceive Frame Start (RFS)\n"
		   "  -e, --mxen\tEnable(1)/disable(0) multiplexing on interface\n"
		   "  -m, --clkm\tSet interface as clock master(1)/slave(0)\n"
		   "  -d, --clkab\tSet clock domain: A(0), B(1)\n"
		   "  -u, --clkr\tClock master uses local(0) or remote(1) oscillator\n",
		   name);
}

action_t
process_args(int argc,char **argv,char **ifname,devsetup_t *settings)
{
    action_t type = help;
    char *endp;


    // Default settings
    settings->type = unknown_ts;
    settings->mx_slotmap = 0;
    settings->mxrate = -1;
    settings->rline = settings->tline = -1;
    settings->rfs = settings->tfs = -1;    
    settings->clkm = settings->clkr = settings->clkab = -1;
    settings->mxen = -1;
    
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
		{"iface", 1, 0, 'i'},
		{"list", 0, 0, 'l'},
		{"check", 0, 0, 'c'},
		{"rline", 1, 0, 'r'},
		{"tline", 1, 0, 't'},
		{"mxrate", 1, 0, 'a'},
		{"mxsmap", 1, 0, 'o'},
		{"tfs", 1, 0, 'j'},
		{"rfs", 1, 0, 'g'},
		{"mxen", 1, 0, 'e'},
		{"clkr", 1, 0, 'u'},
		{"clkab", 1, 0, 'd'},
		{"clkm", 1, 0, 'm'},
		{0, 0, 0, 0}
	};

		int c = getopt_long (argc, argv, "i:slr:t:a:b:cd:e:f:j:g:m:u:o:",
							 long_options, &option_index);
        if (c == -1)
    	    break;
		switch (c) {
        case 'i':
			*ifname = strdup(optarg);
            break;
        case 'l':
			type = info;
    	    break;
		case 'c':
			switch( type ){
			case setup:
			case help:
			case info:
				type = chck;
				break;
			}
			break;
		case 'r':
			type = setup;
			settings->rline = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --rline option: %s not integer\n",optarg);
				return 0;
			}else if( settings->rline > 15 || settings->rline < 0 ){
				printf("Error: --rline must be in [0..15]\n");
				return 0;
			}
			break;
		case 't':
			type = setup;
			settings->tline = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --tline option: %s not integer\n",optarg);
				return 0;
			}else if( settings->tline > 15 || settings->tline < 0 ){
				printf("Error: --tline must be in [0..15]\n");
				return 0;
			}
			break;
		case 'a':
			type = setup;
			if( settings->type != unknown_ts ){
				printf("Error: using both timeslots and continual modes for one device\n");
				return 0;
			}
			settings->mxrate = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --mxrate option: %s not integer\n",optarg);
				return 0;
			}
			settings->type = continual_ts;
			break;
		case 'o':
			{
				type = setup;
				int err;
				if( settings->type != unknown_ts ){
					printf("Error: using both timeslots and continual modes for one device\n");
					return 0;
				}
				settings->mx_slotmap = str2slotmap(optarg,strlen(optarg),&err);
				if( err ){
					printf("Error: in --mxsmap mask: %s",optarg);
					return 0;
				}
				settings->type = selective_ts;
				break;
			}
		case 'j':
			type = setup;
			settings->tfs = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --tfs option: %s not integer\n",optarg);
				return 0;
			}else if( settings->tfs > 255 || settings->tfs < 0){
				printf("Error: --tfs must be in [0..255]\n");
				return 0;
			}
			break;
		case 'g':
			type = setup;
			settings->rfs = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --rfs option: %s not integer\n",optarg);
				return 0;
			}else if( settings->rfs > 255 || settings->rfs < 0){
				printf("Error: --rfs must be in [0..255]\n");
				return 0;
			}
			break;
		case 'e':
			type = setup;
			settings->mxen = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --mxen option: %s not integer\n",optarg);
				return 0;
			}else if( settings->mxen > 1 || settings->mxen < 0){
				printf("Error: --mxen must be in 0 or 1\n");
				return 0;
			}
			break;
		case 'u':
			type = setup;
			settings->clkr = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --clkr option: %s not integer\n",optarg);
				return 0;
			}else if( settings->clkr > 1 || settings->clkr < 0){
				printf("Error: --clkr must be in 0 or 1\n");
				return 0;
			}
			break;
		case 'm':
			type = setup;
			settings->clkm = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --clkm option: %s not integer\n",optarg);
				return 0;
			}else if( settings->clkm > 1 || settings->clkm < 0){
				printf("Error: --clkm must be in 0 or 1\n");
				return 0;
			}
			break;
		case 'd':
			type = setup;
			settings->clkab = strtoul(optarg,&endp,0);
			if( endp == optarg ){
				printf("Error: in --clkab option: %s not integer\n",optarg);
				return 0;
			}else if( settings->clkab > 1 || settings->clkab < 0){
				printf("Error: --clkab must be in 0 or 1\n");
				return 0;
			}
			break;
		}
    }

    return type;
}

void print_device(ifdescr_t *ifd)
{
    devsetup_t *settings = &ifd->settings;

    printf("  %s: mxen=%d clkm=%d clkab=%d clkr=%d rline=%d tline=%d rfs=%d tfs=%d ",
		   ifd->name,settings->mxen,settings->clkm,settings->clkab,settings->clkr,
		   settings->rline,settings->tline,settings->rfs,settings->tfs);

    switch( settings->type ){
    case selective_ts:
		{
			char buf[BUF_SIZE];
			int err;
			slotmap2str(settings->mx_slotmap,buf,0);
			printf(" mxsmap=%s\n",buf);
		}
		break;
    case continual_ts:
		printf(" mxrate=%d\n",settings->mxrate);
		break;
    }
} 

void
mxline_print(mxline_t *l,int lind,char *sname)
{
    int i;
    printf("Line%d %s devices:\n",lind,sname);
    for(i=0;i<l->devcnt;i++){
		print_device(l->devs[i].ifd);
    }
}

inline int 
is_crossing(u8 start1,u8 width1,u8 start2,u8 width2)
{
    return !( (start1+(width1-1))<start2 || (start2+(width2-1))<start1 );
}			    

inline int 
is_contain(u8 start1,u8 width1,u8 start2,u8 width2)
{
    return !( (start1<start2) && (start1+(width1-1))<(start2+(width2-1)) );
}			    

int
check_xline(mxline_t *l,int bindex,char *sname,int rw)
{
    int i,j;
    for(i=0;i<l->devcnt;i++){
		mxelem_t *cur =&l->devs[i];
		for(j=i+1;j<l->devcnt;j++){
			mxelem_t *tmp =&l->devs[j];
			u32 end1 = cur->xfs+(cur->mxrate-1);
			u32 end2 = tmp->xfs+(tmp->mxrate-1);
			if( is_crossing(cur->xfs,cur->mxrate,tmp->xfs,tmp->mxrate) ){
				switch( rw ){
				case 0:
					mxwarn("Line%d: %s timeslots overlap - %s(%d-%d) and %s(%d-%d)",
						   bindex,sname,cur->ifd->name,cur->xfs,end1,
						   tmp->ifd->name,tmp->xfs,end2 );
					break;
				case 1:
					mxerror("Line%d: %s timeslots overlap - %s(%d-%d) and %s(%d-%d)",
							bindex,sname,cur->ifd->name,cur->xfs,end1,
							tmp->ifd->name,tmp->xfs,end2 );
					break;
				}

			}
		}
    }
}

int
mxline2lmap(mxline_t *l,u32 lmap[MX_LWIDTH32])
{
    int i,j,k;
    memset(lmap,0,sizeof(u32)*MX_LWIDTH32);
    if( !l )
		return 0;
    for(i=0;i<l->devcnt;i++){
		int start32 = l->devs[i].xfs/32;
		int start_ts = l->devs[i].xfs%32;
		int end32 = (l->devs[i].xfs+l->devs[i].mxrate)/32;
		int end_ts = (l->devs[i].xfs+l->devs[i].mxrate)%32;
		if( !(end32<MX_LWIDTH32) )
			return -1;
	
		if( start32 == end32 ){
			for(j=start_ts;j<end_ts;j++)
				lmap[start32] |= (1<<j);
		}else{
			// fill first word
			for(j=start_ts;j<32;j++)
				lmap[start32] |= (1<<j);
			// fill last word	
			for(j=0;j<end_ts;j++)
				lmap[end32] |= (1<<j);
			// fill middle words
			for(j=start32+1;j<end32;j++){
				for(k=0;k<32;k++)
					lmap[j] |= (1<<k);
			}
		}
    }
    return 0;
}


int
check_hanging_tslots(mxline_t *rline,mxline_t *tline,int lnum)
{
    u32 rlmap[MX_LWIDTH32],tlmap[MX_LWIDTH32];
    u32 rdiff[MX_LWIDTH32],tdiff[MX_LWIDTH32];
    u8 rerr = 0, terr = 0;
    int i;

    if( mxline2lmap(rline,rlmap) )
		return -1;

    if( mxline2lmap(tline,tlmap) )
		return -1;
    
    for(i=0;i<MX_LWIDTH32;i++){
		rdiff[i] = rlmap[i] & ~(rlmap[i]&tlmap[i]);
		tdiff[i] = tlmap[i] & ~(tlmap[i]&rlmap[i]);
		if( rdiff[i] )
			rerr = 1;
		if( tdiff[i] )
			terr = 1;
    }

    if( terr ){
		char buf[BUF_SIZE], *ptr = buf;
		char flag = 0;
		for(i=0;i<MX_LWIDTH32;i++){
			int offs;
			if( flag && tdiff[i]){
				*ptr = ',';
				*(++ptr) = 0;
			}
			slotmap2str(tdiff[i],ptr,i*32);
			offs = strlen(ptr);
			if( offs ){
				flag = 1;
				ptr += offs;
			}
		}
		mxwarn("Line%d: timeslots %s read but not written",lnum,buf);
    }

    if( rerr ){
		char buf[BUF_SIZE], *ptr = buf;
		char flag=0;
		for(i=0;i<MX_LWIDTH32;i++){
			int offs;
			if( flag && rdiff[i]){
				*ptr = ',';
				*(++ptr) = 0;
			}
			slotmap2str(rdiff[i],ptr,i*32);
			offs = strlen(ptr);
			if( offs ){
				flag = 1;
				ptr += offs;
			}
		}
		mxwarn("Line%d: timeslots %s written but not read",lnum,buf);
    }
    return 0;
}

int
check_hdsc_mux(ifdescr_t *ifd)
{
	char cfg_path[]="/sys/bus/pci/drivers/mr16g";
	char fname[256],buf[256],*rptr;
	FILE *stream;
	unsigned int smap;
	int err;

	if( ifd->settings.type == continual_ts )
		return 0;
	
	snprintf(fname,256,"%s/%s/framed",cfg_path,ifd->name);
	if( !(stream = fopen(fname,"r")) ){
		return -1;
	}
	rptr = fgets(buf,256,stream);
	fclose(stream);
	if( !rptr ) // error while reading from stream
		return -1;
	if( strncmp(buf,"framed",256) )
		return 0;

	snprintf(fname,256,"%s/%s/slotmap",cfg_path,ifd->name);
	if( !(stream = fopen(fname,"r")) ){
		return -1;
	}
	rptr = fgets(buf,256,stream);
	fclose(stream);
	if( !rptr ) // error while reading from stream
		return -1;
	smap = str2slotmap(buf,strlen(buf),&err);
	if( err )
		return -1;
	smap &= ifd->settings.mx_slotmap;
	slotmap2str(smap,buf,0);
	if( smap ){
		mxerror("%s: HDLC & multiplexing timeslots overlaps: %s",ifd->name,buf);
	}
	return 0;
}


int
check(ifdescr_t **iflist,int iflsize)
{
    int i;
    mxline_t *rlines[MX_LINES],*tlines[MX_LINES];
    int clkAm[MAX_IFS],clkBm[MAX_IFS];
    u16 clkAm_cnt=0,clkBm_cnt=0,clkA_cnt=0,clkB_cnt=0;
    int derr = 0;

    memset(rlines,0,sizeof(rlines));
    memset(tlines,0,sizeof(tlines));
    memset(clkAm,0,sizeof(clkAm));
    memset(clkBm,0,sizeof(clkBm));

PDEBUG(DFULL,"Start");

    // Fill lines
    for(i=0;i<iflsize;i++){
		u32 mxrate = 0;
		mxelem_t el;
		devsetup_t *set = &iflist[i]->settings;

		if( !set->mxen )
			continue;
		switch( set->clkab ){
		case clkA:
			clkA_cnt++;
			break;
		case clkB:
			clkB_cnt++;
			break;
		}
	
		if( set->clkm ){
			switch( set->clkab ){
			case clkA:
				clkAm[clkAm_cnt] = i;
				clkAm_cnt++;
				break;
			case clkB:
				clkBm[clkBm_cnt] = i;
				clkBm_cnt++;
				break;
			}
		}else if( set->clkr ){
			mxwarn("%s: clkR option is ignored - not clock master",iflist[i]->name);
		}

		switch( set->type ){
		case continual_ts:
			mxrate = set->mxrate;
			break;
		case selective_ts:
			mxrate = slotmap2mxrate(set->mx_slotmap);
			break;
		}
	
		el.ifd = iflist[i];
	
		el.xfs = set->rfs;
		el.mxrate = mxrate;
		if( !rlines[set->rline] ){
			rlines[set->rline] = mxline_init();
		}
        
		if( mxline_add(rlines[set->rline],set->clkab,el,set->tline) ){
			mxerror("Cannot add device %s to line %d, too many interfaces",
					iflist[i]->name,set->rline);
		}

		el.xfs = set->tfs;
		el.mxrate = mxrate;
		if( !tlines[set->tline] ){
			tlines[set->tline] = mxline_init();
		}

		if( mxline_add(tlines[set->tline],set->clkab,el,set->tline) ){
			mxerror("Cannot add device %s to line %d, too many interfaces",
					iflist[i]->name,set->tline);
		}
    }

PDEBUG(DFULL,"#1");    
    // 1. In each domain one and only one master 
    if( !clkAm_cnt && clkA_cnt )
		mxerror("No clock master in domain A");
    if( clkAm_cnt>1 ){
		int i;
		mxerror("More than one clock master in domain A:");
		for(i=0;i<clkAm_cnt;i++){
			print_device(iflist[clkAm[i]]);
		}
    }
    if( !clkBm_cnt && clkB_cnt)
		mxerror("No clock master in domain B");

PDEBUG(DFULL,"#2");    

    if( clkBm_cnt>1 ){
		int i;
		mxerror("More than one clock master in domain B:");
		for(i=0;i<clkBm_cnt;i++){
			print_device(iflist[clkBm[i]]);
		}
    }

PDEBUG(DFULL,"#3");    

    // 2. All devices of line in one domain
    for(i=0;i<MX_LINES;i++){
PDEBUG(DFULL,"#3.%d",i);    

		int rerr=0,terr=0;
		if( !rlines[i] && !tlines[i] )
			continue; // line is not used

		if( rlines[i] && rlines[i]->domain_err ){
			mxerror("Line%d: not all writing devices in same domain",i);
			rerr = 1;
		}

		if( tlines[i] && tlines[i]->domain_err){
			mxerror("Line%d: not all reading devices in same domain",i);
			terr = 1;
		}

		if( tlines[i] && !rlines[i] ){
			continue;
		} else if( !tlines[i] && rlines[i] ){
			continue;
		} else if( tlines[i]->domain != rlines[i]->domain ){
			mxerror("Line%d: not all reading devices in same domain",i);
			rerr = 1;
			terr = 1;
		}
		if( terr && tlines[i] )
			mxline_print(tlines[i],i,"read");
		if( rerr && rlines[i])
			mxline_print(rlines[i],i,"write");
    }

PDEBUG(DFULL,"#4");    

    // 3. Check that timeslots inside line do not cross
    for(i=0;i<MX_LINES;i++){
		if( !rlines[i] && !tlines[i] )
			continue;
		if( rlines[i] ){
			check_xline(rlines[i],i,"rline",1);
		}
		if( tlines[i] ){    
			check_xline(tlines[i],i,"tline",0);
		}
    }

    for(i=0;i<MX_LINES;i++){
		if( check_hanging_tslots(rlines[i],tlines[i],i) )
			mxerror("Unknown error whileprocess Line%d",i);
    }

PDEBUG(DFULL,"#5");    
    

    // 4. Check for E1 timeslots that HDLC & MUX timeslots don't cross
	for(i=0;i<iflsize;i++){
		check_hdsc_mux(iflist[i]);
	}

PDEBUG(DFULL,"#6");    

}

int main(int argc, char *argv[] )
{
    char *ifname = NULL;
    int iflen = 0;
    char *pname = strdup(argv[0]);
    action_t type = help;    
    devsetup_t settings;
    iftype_t iftype = unknown_ts;
    ifdescr_t *iflist[MAX_IFS];    
    int cnt = fill_iflist(IFS_ROOT,iflist,MAX_IFS);

    type = process_args(argc,argv,&ifname,&settings);

    if( ifname )
		iflen = strlen(ifname);
    
    switch( type ){
    case help:
		print_usage(pname);
		break;
    case info:
		if( ifname ){
			printf("Display short info about %s\n",ifname);
			int i;
			for(i=0;i<cnt;i++){
				int tmplen = strlen(iflist[i]->name);
				if( tmplen != iflen )
					continue;
				if( !strcmp(ifname,iflist[i]->name) ){
					print_device(iflist[i]);
					break;
				}
			}
		}else{
			int i;
			for(i=0;i<cnt;i++){
				print_device(iflist[i]);
			}
		}
		return 0;
    case full_info:
		if( ifname ){
			printf("Display short info about %s\n",ifname);
			int i;
			for(i=0;i<cnt;i++){
				int tmplen = strlen(iflist[i]->name);
				if( tmplen != iflen )
					continue;
				if( !strcmp(ifname,iflist[i]->name) ){
					//		    print_full(IFS_ROOT,iflist[i]);
					break;
				}
			}
		}else{
			int i;
			for(i=0;i<cnt;i++){
				//		print_full(IFS_ROOT,iflist[i]);
			}
		}
		return 0;
    case setup:
		{
			devsetup_t *set = NULL;
			int i;
			for(i=0;i<cnt;i++){
				set = &iflist[i]->settings;
				int tmplen = strlen(iflist[i]->name);
				if( tmplen != iflen )
					continue;
				if( !strcmp(ifname,iflist[i]->name) )
					break;
			}
			if( i == cnt || !set){
				mxerror("Error: cannot find interface %s\n",ifname);
				return -1;
			}
			iflist[i]->settings = settings;
            switch( iflist[i]->type ){
            case network:
    			apply_settings_net(IFS_ROOT,iflist[i]);
                break;
            case serial:
    			apply_settings_ser(iflist[i]);
                break;
            }
		}
		break;
    case chck:
		check(iflist,cnt);
		break;
    }
    
    return 0;
}

