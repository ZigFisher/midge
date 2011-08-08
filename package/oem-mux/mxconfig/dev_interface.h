#ifndef DEV_INTERFACE_H
#define DEV_INTERFACE_H

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/fcntl.h>

#include "ltypes.h"

#define MAX_FNAME 512
#define BUF_SIZE 256

// TTY device part
#ifndef DEVBASENAME
#define DEVBASENAME "ttyRS"
#endif
#define DEVBASEPATH "/dev"
#define TIOCGMX 0xA000 // Get mux info
#define TIOCSMX	0xA001 // set mux info


typedef enum { unknown_ts,continual_ts,selective_ts } iftype_t;
typedef enum { network,serial } device_type_t;

typedef struct {
    iftype_t type;
    u32 mx_slotmap;
    s32 mxrate;
    s8 rline,tline;
    s16 tfs,rfs;
    s8 clkm;
    s8 clkab;
    s8 clkr;
    s8 mxen;
} devsetup_t;

struct mxsettings{
    unsigned short magic;
    s32 mxrate;
    s8 rline,tline;
    s16 tfs,rfs;
    s8 clkm;
    s8 clkab;
    s8 clkr;
    s8 mxen;
};

typedef struct {
    char *name;
    u8 conf_ind;
    devsetup_t settings;
    device_type_t type;
} ifdescr_t;


int check_for_mxsupport(char *conf_path,ifdescr_t *ifd);
int fill_iflist(char *conf_path,ifdescr_t **iflist,int ifcnt);
int set_dev_option(char *conf_path,char *name,int val);
int get_dev_option(char *conf_path,char *name,int *val);
u32 str2slotmap(char *str,size_t size,int *err);
int slotmap2str(u32 smap, char *buf,int offset);
u8 slotmap2mxrate(u32 smap);
int apply_settings_net(char *conf_path,ifdescr_t *ifd);
int accum_settings_net(char *conf_path,ifdescr_t *ifd);
int apply_settings_ser(ifdescr_t *ifd);

#endif
