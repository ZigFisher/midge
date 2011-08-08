#include "tailor.h"
#include "gzip.h"
#include "lzw.h"
#include "revision.h"
#include "getopt.h"

/* global buffers */
DECLARE(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
DECLARE(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
//DECLARE(ush, d_buf,  DIST_BUFSIZE);
DECLARE(uch, window, 2L*WSIZE);
//#ifndef MAXSEG_64K
//    DECLARE(ush, tab_prefix, 1L<<BITS);
//#else
//    DECLARE(ush, tab_prefix0, 1L<<(BITS-1));
//    DECLARE(ush, tab_prefix1, 1L<<(BITS-1));
//#endif

                /* local variables */
long header_bytes;
int ascii = 0;        /* convert end-of-lines to local OS conventions */
int to_stdout = 0;    /* output to stdout (-c) */
int decompress = 0;   /* decompress (-d) */
int force = 0;        /* don't ask questions, compress links (-f) */
int no_name = -1;     /* don't save or restore the original file name */
int no_time = -1;     /* don't save or restore the original file time */
int recursive = 0;    /* recurse through directories (-r) */
int list = 0;         /* list the file contents (-l) */
int verbose = 0;      /* be verbose (-v) */
int quiet = 0;        /* be very quiet (-q) */
int do_lzw = 0;       /* generate output compatible with old compress (-Z) */
int gzip_test = 0;         /* test .gz file integrity */
int foreground;       /* set if program run in foreground */
char *progname;       /* program name */
int maxbits = BITS;   /* max bits per code for LZW */
int method = DEFLATED;/* compression method */
int level = 6;        /* compression level */
int exit_code = OK;   /* program exit code */
int save_orig_name;   /* set if original name must be saved */
int last_member;      /* set for .zip and .Z files */
int part_nb;          /* number of parts in .gz file */
long time_stamp;      /* original time stamp (modification time) */
long ifile_size;      /* input file size, -1 for devices (debug only) */
char *env;            /* contents of GZIP env variable */
char **args = NULL;   /* argv pointer if GZIP env variable defined */
char z_suffix[MAX_SUFFIX+1]; /* default suffix (can be set with --suffix) */
int  z_len;           /* strlen(z_suffix) */

long bytes_in;             /* number of input bytes */
long bytes_out;            /* number of output bytes */
long total_in = 0;         /* input bytes for all files */
long total_out = 0;        /* output bytes for all files */
int  remove_ofname = 0;    /* remove output file on error */
flash_file ofd;             /* input file descriptor */
flash_file ifd;             /* output file descriptor */
unsigned insize;           /* valid bytes in inbuf */
unsigned inptr;            /* index of next byte to be processed in inbuf */
unsigned outcnt;           /* bytes in output buffer */


/* local functions */
local int treat_file();
local int  get_method();



int gzip_decompress (flash_file *in, flash_file *out)
{
    /******************************************************************/
    /* Cache cannot work with interrupt, since interrupt will causes  */
    /* cache miss becase interrupt usually is far code. When caches   */
    /* missed, burst access is invoked but the flash does not support */
    /* burst access. So, we disable interrupt while we use cache.     */
    /******************************************************************/

    //SysIcacheInit();

    decompress = 1;

    ifd.size = in->size;
    ifd.buffer = in->buffer;
    ifd.read_index = 0;
    ofd.buffer = out->buffer;
    ofd.write_index = 0;
 
    /* By default, save name and timestamp on compression but do not
     * restore them on decompression.
     */
    if (no_time < 0) no_time = decompress;
    if (no_name < 0) no_name = decompress;

    decompress = 1;
    exit_code = 0;
    exit_code = treat_file();
    out->size = ofd.write_index;

    //SysIcacheInhibit();

    return exit_code; /* just to avoid lint warning */
}


/* ========================================================================
 * Compress or decompress the given file
 */
local int treat_file()
{  
    ifile_size = ifd.size;
    time_stamp = 0;

    /* Generate output file name */
    
    /* Open the input file and determine compression method. */

    clear_bufs(); /* clear input and output buffers */
    part_nb = 0;

    method = get_method(); /* updates ofname if original given */
    if (method < 0) 
    {
        return -1;               /* error message already emitted */
    }

    /* Actually do the compression/decompression. Loop over zipped members.
     */
    for (;;) {
        if (unzip() != OK) 
        {
            method = -1; /* force cleanup */
        }
        if (!decompress || last_member || inptr == insize) break;
        /* end of file */

        method = get_method();
        if (method < 0) break;    /* error message already emitted */
        bytes_out = 0;            /* required for length check */
    }

    if (method == -1) {
        return -1;
    }
    return 0;
}



/* ========================================================================
 * Check the magic number of the input file and update ofname if an
 * original name was given and to_stdout is not set.
 * Return the compression method, -1 for error, -2 for warning.
 * Set inptr to the offset of the next byte to be processed.
 * Updates time_stamp if there is one and --no-time is not used.
 * This function may be called repeatedly for an input file consisting
 * of several contiguous gzip'ed members.
 * IN assertions: there is at least one remaining compressed member.
 *   If the member is a zip file, it must be the only one.
 */
local int get_method()
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    ulg stamp;     /* time stamp */


    magic[0] = (char)get_byte();
    magic[1] = (char)get_byte();
    method = -1;                 /* unknown yet */
    part_nb++;                   /* number of parts in gzip file */
    header_bytes = 0;
    last_member = RECORD_IO;
    /* assume multiple members in gzip file except for record oriented I/O */

    if (memcmp(magic, GZIP_MAGIC, 2) == 0
        || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0) 
    {
        method = (int)get_byte();
        if (method != DEFLATED) 
        {
            printf("\n unknown method %d -- ", method);
            return -1;
        }
        flags  = (uch)get_byte();

        if ((flags & ENCRYPTED) != 0) {
            printf("\n input file is encrypted -- ");
            return -1;
        }
        if ((flags & CONTINUATION) != 0) {
            printf("\n is a a multi-part gzip file --");
            return -1;
        }
        if ((flags & RESERVED) != 0) {
            printf("\n has flags 0x%x -- ", flags);
            return -1;
        }
        stamp  = (ulg)get_byte();
        stamp |= ((ulg)get_byte()) << 8;
        stamp |= ((ulg)get_byte()) << 16;
        stamp |= ((ulg)get_byte()) << 24;
        if (stamp != 0 && !no_time) time_stamp = stamp;

        (void)get_byte();  /* Ignore extra flags for the moment */
        (void)get_byte();  /* Ignore OS type for the moment */

        if ((flags & CONTINUATION) != 0) {
            unsigned part = (unsigned)get_byte();
            part |= ((unsigned)get_byte())<<8;
        }
        if ((flags & EXTRA_FIELD) != 0) {
            unsigned len = (unsigned)get_byte();
            len |= ((unsigned)get_byte())<<8;
            printf("\n extra field of %u bytes ignored\n", len);
            while (len--) (void)get_byte();
        }

        /* Get original file name if it was truncated */
        if ((flags & ORIG_NAME) != 0) {
            /* Discard the old name */
            char c; /* dummy used for NeXTstep 3.0 cc optimizer bug */
            do {c=get_byte();} while (c != 0);
        } /* ORIG_NAME */

        /* Discard file comment if any */
        if ((flags & COMMENT) != 0) {
            while (get_char() != 0) /* null */ ;
        }
        if (part_nb == 1) {
            header_bytes = inptr + 2*sizeof(long); /* include crc and size */
        }
    } 

    if (method >= 0) return method;

    if (part_nb == 1) {
//        printf("\n not in gzip format\n");
        return -1;
    } 
    else 
    {
        //printf("\n decompression OK, trailing garbage ignored\n");
        return -2;
    }
}


