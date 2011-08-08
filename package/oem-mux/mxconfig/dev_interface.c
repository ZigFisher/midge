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

#include "debug.h"
#include "dev_interface.h"
#include "mxconfig.h"

#define MAX_TS_BIT 32
static char *mux_dirs[] = {"sg_multiplexing","hw_multiplexing"};
static int mux_dirs_cnt = sizeof(mux_dirs)/sizeof(char *);


int
check_for_mxsupport(char *conf_path,ifdescr_t *ifd) 
{
    DIR *dir;
    int ocnt = 0,i;
    struct dirent *ent;
    char d[MAX_FNAME];
    char *opts[] = { "mx_clkab","mx_clkm","mx_clkr","mx_enable",
					 "mx_rline","mx_rxstart","mx_tline","mx_txstart" };
    int optsize = sizeof(opts)/sizeof(char *);
    iftype_t type;
    
    
    for(i=0;i<mux_dirs_cnt;i++){
        snprintf(d,MAX_FNAME,"%s/%s/%s",conf_path,ifd->name,mux_dirs[i]);
        if( dir = opendir(d) ){
            break;
        }
    }
    if( i == mux_dirs_cnt ){
        PDEBUG(DERR,"Iface %s do not support multiplexing",ifd->name);
        return -1;
    }
    ifd->conf_ind = i;

    type = unknown_ts;
    while( ent = readdir(dir) ){
        int flen = strlen(ent->d_name);
		int optlen = strlen("mxrate");
		if( flen == optlen && !strncmp("mxrate",ent->d_name,flen) ){
			if( type != unknown_ts ){
				closedir(dir);
				return -1;
			}
			type = continual_ts;
			continue;
		}

		optlen = strlen("mx_slotmap");
		if( flen == optlen && !strncmp("mx_slotmap",ent->d_name,flen) ){
			if( type != unknown_ts ){
				closedir(dir);
				return -1;
			}
			type = selective_ts;
			continue;
		}

		for(i=0;i<optsize;i++){
			int optlen = strlen(opts[i]);
			if( flen != optlen )
				continue; // not equal
			PDEBUG(DFULL,"check equal of: %s - %s",opts[i],ent->d_name);
			if( !strncmp(opts[i],ent->d_name,flen) ){
				PDEBUG(DFULL,"%s - %s is equal",opts[i],ent->d_name);
				ocnt++;
				break;
			}
		}
    }
    closedir(dir);
    ifd->settings.type = type;
    PDEBUG(DFULL,"ocnt = %d",ocnt);
    return (ocnt == optsize ) ? 0 : -1;	
}




/* - Old one. Simple alphabetic comparision 
static int
compare_func(const void *p1, const void *p2)
{
    ifdescr_t *d1 = *(ifdescr_t**)p1;
    ifdescr_t *d2 = *(ifdescr_t**)p2;
    
	return strcmp((const char*)d1->name,(const char*)d2->name);
}

*/

int mux_split(char *source,char *symb,int symbsize)
{
    int len,ret,i;

    // Split to symbol & ineger parts
    for(len = strlen(source)-1; len>0 && (source[len] >= '0' && source[len] <= '9') ;len--);
    
//    printf("Word=%s, len=%d\n",source,len);
    if( !(source[len] >= '0' && source[len] <= '9') ){
        // we have symbol part
        for(i=0;i<=len && i<symbsize-1;i++){
            symb[i] = source[i];
        }
        symb[i] = '\0';
//        printf("transform %s, len=%d\n",source+len+1,len);
        ret = atoi(source+len+1);
    }else{
        symb[0] = '\0';
        ret = atoi(source);
    }
    return ret;
}


static int
compare_func(const void *p1, const void *p2)
{
    ifdescr_t *if1 = *(ifdescr_t**)p1;
    ifdescr_t *if2 = *(ifdescr_t**)p2;

    char *d1 = if1->name;
    char *d2 = if2->name;
    
    char symb_d1[256],symb_d2[256];
    int int_d1,int_d2;
    int ret;

    // Split first ifname to symbol & ineger parts
    int_d1 = mux_split(d1,symb_d1,256);
    int_d2 = mux_split(d2,symb_d2,256);
/*    
    printf("Compare: %s, %s\n",d1,d2);
    printf("symb1=%s,int1=%d\n",symb_d1,int_d1);
    printf("symb2=%s,int2=%d\n",symb_d2,int_d2);
*/    
    ret =  strcasecmp((const char*)symb_d1,(const char*)symb_d2);
    if( !ret )
        ret = (int_d1 > int_d2) ? 1 : 0;
    if( !ret )
        ret = (int_d1 < int_d2) ? -1 : 0;
        
//    printf("ret=%d\n\n",ret);
        
	return ret;
}


int
fill_iflist(char *conf_path,ifdescr_t **iflist,int ifcnt)
{
    DIR *dir;
    struct dirent *ent;
    int cnt = 0,i;
    char fname[MAX_FNAME];
    struct stat statbuf;
    devsetup_t settings;
    
    for(i=0;i<ifcnt;i++){
		iflist[i] = NULL;
    }

    // Network devices supporting Sigrand multiplexing
    if( !(dir = opendir(conf_path)) ){
		PDEBUG(DERR,"Cannot open dir: %s",conf_path);
		return -1;
    }
    while( (ent = readdir(dir)) && (cnt < ifcnt)){
		iflist[cnt] = malloc(sizeof(ifdescr_t));
        if( !iflist[cnt] ){
            mxerror("not enough memory");
            return 0;
        }
        iflist[cnt]->name = strdup(ent->d_name);
        iflist[cnt]->type = network;
		if( check_for_mxsupport(conf_path,iflist[cnt]) ){
			free(iflist[cnt]->name);
			free(iflist[cnt]);
			iflist[cnt] = NULL;
		}else{
			if( accum_settings_net(conf_path,iflist[cnt]) ){
				free(iflist[cnt]->name);
				free(iflist[cnt]);
				iflist[cnt] = NULL;
				continue;
			}
			cnt++;
		}
    }
    closedir(dir);

    // Serial devices supporting Sigrand multiplexing
    for(i=0;;i++){
        char name[64];
        struct mxsettings dset;
        int fd, ret;
        devsetup_t *set;
        snprintf(name,64,"%s/%s%d",DEVBASEPATH,DEVBASENAME,i);
        
        // Open device node
        if( (fd=open(name,O_NONBLOCK)) < 0){
            break;
        }
        // Test mux ability and get current settings
        dset.magic = 0xAFAF;
        if( ioctl(fd,TIOCGMX,(char*)&dset,sizeof(dset)) ){
            mxwarn("Device with name %s not support multiplexing!",name);
            continue;
        }

        // Add new element        
        iflist[cnt] = malloc(sizeof(ifdescr_t));
        if( !iflist[cnt] ){
            mxerror("not enough memory");
            return 0;
        }
        snprintf(name,64,"%s%d",DEVBASENAME,i);
        iflist[cnt]->name = strdup(name);
        iflist[cnt]->type = serial;
        // Fill settings
        set = &iflist[cnt]->settings;
        set->type = continual_ts;
        set->mxrate = dset.mxrate;
        set->rline = dset.rline;
        set->tline = dset.tline;
        set->rfs = dset.rfs;
        set->tfs = dset.tfs;
        set->clkm = dset.clkm;    
        set->clkab = dset.clkab;
        set->clkr = dset.clkr;
        set->mxen = dset.mxen;
        cnt++;
    }    
    
    
    // Quick sort of channels
    qsort((void*)iflist,cnt,sizeof(ifdescr_t *),compare_func);

    return cnt;
}

unsigned int
str2slotmap(char *str,size_t size,int *err)
{
    char *e,*s=str;
    unsigned int fbit,lbit,ts=0;
    int i;

    PDEBUG(4,"start");	
    for (;;) {
        fbit=lbit=strtoul(s, &e, 0);
        if (s == e)
            break;
        if (*e == '-') {
            s = e+1;
            lbit = strtoul(s, &e, 0);
        }

        if (*e == ','){
            e++;
        }

        if( !(fbit < MAX_TS_BIT && lbit < MAX_TS_BIT) )
            break;
        
		for (i=fbit; i<=lbit;i++){
			ts |= 1L << i;
		}
        s=e;
    }
    PDEBUG(4,"str=%08x, s=%08x,size=%d",(u32)str,(u32)s,size);
    *err=0;	
    if( s != (str+size) && s != str )
        *err=1;
    return ts;
}


int
slotmap2str(unsigned int smap, char *buf,int offset)
{
    int start = -1,end, i;
    char *p=buf;

    if( !smap ){
		buf[0] = 0;
		return 0;
    }
    
    for(i=0;i<32;i++){
		if( start<0 ){
			start = ((smap >> i) & 0x1) ? i : -1;
			if( start>0 && i==31 )
				p += sprintf(p,"%d",start+offset);
		}else if( !((smap >> i) & 0x1) || i == 31){
			end = ((smap >> i) & 0x1) ? i : i-1;
			if( p>buf )
				p += sprintf(p,",");
			p += sprintf(p,"%d",start+offset);
			if( start<end )
				p += sprintf(p,"-%d",end+offset);
			start=-1;
		}
    }
    return strlen(buf);
}

u8 
slotmap2mxrate(u32 smap)
{
    int mxrate = 0;
    while(smap){
		if(smap&0x01)
			mxrate++;
		smap = (smap>>1) &0x7fffffff;
    }
    return mxrate;
}


int 
set_int_option(char *conf_path,char *name,int val)
{
    int fd;
    int len,cnt;
    char fname[MAX_FNAME];
    char buf[BUF_SIZE];
	
    PDEBUG(DFULL,"set_dev_option(%s,%s)",name,val); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
		PDEBUG(DERR,"set_dev_option: Cannot open %s",fname);
		return -1;
    }
    snprintf(buf,BUF_SIZE,"%d",val);
    len = strlen(buf);
    cnt = write(fd,buf,len);
    PDEBUG(DFULL,"set_dev_option: write %d, written %d",len,cnt);
    close(fd);
    if( cnt != len )
		return -1;
    return 0;
}

int 
set_char_option(char *conf_path,char *name,char *str)
{
    int fd;
    int len,cnt;
    char fname[MAX_FNAME];
	
    PDEBUG(DFULL,"set_dev_option(%s,%s)",name,str); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
		PDEBUG(DERR,"set_dev_option: Cannot open %s",fname);
		return -1;
    }
    len = strlen(str)+1;
    cnt = write(fd,str,len);
    PDEBUG(DFULL,"set_dev_option: write %d, written %d",len,cnt);
    close(fd);
    if( cnt != len )
		return -1;
    return 0;
}

int
get_int_option(char *conf_path,char *name,int *val)
{
    int fd;
    int cnt;
    char fname[MAX_FNAME];
    char buf[BUF_SIZE];
    char *endp;
    int tmp=0;
    
    PDEBUG(DFULL,"get_dev_option(%s)",name); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);

    if( (fd = open(fname,O_RDONLY)) < 0 ){
		PDEBUG(DERR,"get_dev_option: Cannot open %s",fname);
		return -1;
    }

    cnt = read(fd,buf,BUF_SIZE);
    PDEBUG(DFULL,"get_dev_option: readed %d",cnt);

    close(fd);

    if( cnt < 0 || !cnt){
		return -1;
    }
    buf[cnt] = 0;

    *val = strtoul(buf,&endp,0);
    if (endp == buf )
		return -1;

    return 0;
}

int
get_char_option(char *conf_path,char *name,char **val)
{
    int fd;
    int cnt;
    char fname[MAX_FNAME];
    char buf[BUF_SIZE];
    char *endp;
    
    PDEBUG(DFULL,"get_dev_option(%s)",name); 
    snprintf(fname,MAX_FNAME,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_RDONLY)) < 0 ){
		PDEBUG(DERR,"get_dev_option: Cannot open %s",fname);
		return -1;
    }
    cnt = read(fd,buf,BUF_SIZE);
    PDEBUG(DFULL,"get_dev_option: readed %d",cnt);
    close(fd);
    if( cnt < 0 ){
		return -1;
    }
    buf[cnt] = 0;
    *val = strdup(buf);
    return 0;
}

int
apply_settings_net(char *conf_path,ifdescr_t *ifd)
{
    char conf[MAX_FNAME];
    char buf[BUF_SIZE];
    devsetup_t *set = &ifd->settings;
    
    snprintf(conf,MAX_FNAME,"%s/%s/%s",conf_path,ifd->name,mux_dirs[ifd->conf_ind]);
    
    if( set->type == continual_ts && set->mxrate>=0 ){
		if( set_int_option(conf,"mxrate",set->mxrate) )
			return -1;
    }else if( set->type == selective_ts ){
		slotmap2str(set->mx_slotmap,buf,0);
		if( set_char_option(conf,"mx_slotmap",buf) )
			return -1;
    }

    if( set->rline >=0 ){
		set_int_option(conf,"mx_rline",set->rline);
    }

    if( set->tline >=0 ){
		set_int_option(conf,"mx_tline",set->tline);
    }

    if( set->rfs >=0 ){
		set_int_option(conf,"mx_rxstart",set->rfs);
    }
    
    if( set->tfs >=0 ){
		set_int_option(conf,"mx_txstart",set->tfs);
    }

    if( set->clkm >=0 ){
		set_int_option(conf,"mx_clkm",set->clkm);
    }

    if( set->clkab >=0 ){
		set_int_option(conf,"mx_clkab",set->clkab);
    }

    if( set->clkr >=0 ){
		set_int_option(conf,"mx_clkr",set->clkr);
    }

    if( set->mxen >=0 ){
		set_int_option(conf,"mx_enable",set->mxen);
    }

}

int
accum_settings_net(char *conf_path,ifdescr_t *ifd)
{
    char conf[MAX_FNAME];
    char *buf;
    int size,err;
    devsetup_t *set = &ifd->settings;
    int tmp=0;
    
    snprintf(conf,MAX_FNAME,"%s/%s/%s",conf_path,ifd->name,mux_dirs[ifd->conf_ind]);
    
    // Read used timeslots information
    switch( set->type ){
    case continual_ts: 
		if( get_int_option(conf,"mxrate",&tmp) )
			return -1;
		set->mxrate = tmp;
		break;
    case selective_ts:
		if( get_char_option(conf,"mx_slotmap",&buf) )
			return -1;
		set->mx_slotmap = str2slotmap(buf,strlen(buf),&err);
		if( err && strlen(buf) )
			return -1;
		break;
    }
    
    // 
    if( get_int_option(conf,"mx_rline",&tmp) ){
		return -1;
    }
    set->rline = tmp;

    if( get_int_option(conf,"mx_tline",&tmp) ){
		return -1;
    }
    set->tline = tmp;
    
    if( get_int_option(conf,"mx_rxstart",&tmp) ){
		return -1;
    }
    set->rfs = tmp;
    
    if( get_int_option(conf,"mx_txstart",&tmp) ){
		return -1;
    }
    set->tfs = tmp;

    if( get_int_option(conf,"mx_clkm",&tmp) ){
		return -1;
    }
    set->clkm = tmp;

    if( get_int_option(conf,"mx_clkab",&tmp) ){
		return -1;
    }
    set->clkab= tmp;

    if( get_int_option(conf,"mx_clkr",&tmp) ){
		return -1;
    }
    set->clkr = tmp;

    if( get_int_option(conf,"mx_enable",&tmp) ){
		return -1;
    }
    set->mxen = tmp;
    return 0;
}

int
apply_settings_ser(ifdescr_t *ifd)
{
    char name[64];
    struct mxsettings dset;
    int fd, ret;
    devsetup_t *set;
        
    snprintf(name,64,"%s/%s",DEVBASEPATH,ifd->name);
    // Open device node
    if( (fd=open(name,O_NONBLOCK)) < 0){
        mxerror("No such device %s",ifd->name);
        return -1;
    }

    // Test mux ability and get current settings
    dset.magic = 0xAFAF;
    set = &ifd->settings;
    dset.mxrate = set->mxrate;
    dset.rline = set->rline;
    dset.tline = set->tline;
    dset.rfs = set->rfs;
    dset.tfs = set->tfs;
    dset.clkm = set->clkm;    
    dset.clkab = set->clkab;
    dset.clkr = set->clkr;
    dset.mxen = set->mxen;
    
    // apply changes
    if( ioctl(fd,TIOCSMX,(char*)&dset,sizeof(dset)) ){
        mxerror("Device %s does not support multiplexing",ifd->name);
        return -1;
    }
    return 0;
}
