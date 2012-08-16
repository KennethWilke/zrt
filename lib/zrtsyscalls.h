/*
 * syscallbacks.h
 * Syscallbacks used by zrt;
 *
 *  Created on: 6.07.2012
 *      Author: YaroslavLitvinov
 */

#ifndef ZRT_LIB_ZRTSYSCALLS_H
#define ZRT_LIB_ZRTSYSCALLS_H

#include "zvm.h"

#define DEV_STDIN  "/dev/stdin"
#define DEV_STDOUT "/dev/stdout"
#define DEV_STDERR "/dev/stderr"

#define FIRST_NON_RESERVED_INODE 11
#define INODE_FROM_HANDLE(handle) (FIRST_NON_RESERVED_INODE+handle)


#ifdef DEBUG
#define zrt_log(fmt, ...) \
        do {\
            char *buf; \
            int debug_handle = debug_handle_get_buf(&buf); \
            int len;\
            if(debug_handle < 0) break;\
            len = snprintf(buf, 0x1000, "%s; %s, %d: " fmt "\n", \
            __FILE__, __func__, __LINE__, __VA_ARGS__);\
            zvm_pwrite(debug_handle, buf, len, 0); \
        } while(0)

#define zrt_log_str(text) \
        do {\
            char *buf; \
            int debug_handle = debug_handle_get_buf(&buf); \
            int len;\
            if( debug_handle < 0) break;\
            len = snprintf(buf, 0x1000, "%s; %s, %d: %s\n", \
            __FILE__, __func__, __LINE__, text);\
            zvm_pwrite(debug_handle, buf, len, 0); \
        } while(0)

int debug_handle_get_buf(char **buf);
#endif


struct ZrtChannelRt{
    int     handle;
    int     open_mode;             /*For currently opened file contains mode, otherwise -1*/
    int     flags;                 /*For currently opened file contains flags*/
    int64_t sequential_access_pos; /*sequential read, sequential write*/
    int64_t random_access_pos;     /*random read, random write*/
    int64_t maxsize;               /*synthethic size. maximum position of channel for all I/O requests*/
};

enum PosAccess{ EPosSeek=0, EPosRead, EPosWrite };
enum PosWhence{ EPosGet=0, EPosSetAbsolute, EPosSetRelative };
/*@param pos_whence If EPosGet offset unused, otherwise check and set offset
 *@return -1 if bad offset, else offset result*/
int64_t channel_pos( int handle, int8_t whence, int8_t access, int64_t offset );

/*Assign own channels pointer to get it filled with channels at the syscallback*/
void zrt_setup( struct UserManifest* manifest );

#define USE_PADDING
struct nacl_abi_dirent {

    unsigned long long   d_ino;     /*offsets NaCl 0 */
//#ifdef USE_PADDING
//    unsigned long padding1;
//#endif

    unsigned long long  d_off;     /*offsets NaCl 8 */
//#ifdef USE_PADDING
//    unsigned long padding2;
//#endif

    uint16_t  d_reclen;  /*offsets NaCl 16 */
    char     d_name[];  /*offsets NaCl 18 */
};

/*
 * temporary fix for nacl. stat != nacl_abi_stat
 * also i had a weird error when tried to use "service_runtime/include/sys/stat.h"
 */
struct nacl_abi_stat
{
    /* must be renamed when ABI is exported */
    int64_t   nacl_abi_st_dev;       /* not implemented */
    uint64_t  nacl_abi_st_ino;       /* not implemented */
    uint32_t  nacl_abi_st_mode;      /* partially implemented. */
    uint32_t  nacl_abi_st_nlink;     /* link count */
    uint32_t  nacl_abi_st_uid;       /* not implemented */
    uint32_t  nacl_abi_st_gid;       /* not implemented */
    int64_t   nacl_abi_st_rdev;      /* not implemented */
    int64_t   nacl_abi_st_size;      /* object size */
    int32_t   nacl_abi_st_blksize;   /* not implemented */
    int32_t   nacl_abi_st_blocks;    /* not implemented */
    int64_t   nacl_abi_st_atime;     /* access time */
    int64_t   nacl_abi_st_atimensec; /* possibly just pad */
    int64_t   nacl_abi_st_mtime;     /* modification time */
    int64_t   nacl_abi_st_mtimensec; /* possibly just pad */
    int64_t   nacl_abi_st_ctime;     /* inode change time */
    int64_t   nacl_abi_st_ctimensec; /* only nacl; possibly just pad */
};


/* same here */
struct nacl_abi_timeval {
    int64_t   nacl_abi_tv_sec;
    int32_t   nacl_abi_tv_usec;
};

/*
 * FULL NACL SYSCALL LIST
 */
#define NACL_sys_null                    1 /* empty syscall. does nothing */
#define NACL_sys_nameservice             2
#define NACL_sys_dup                     8
#define NACL_sys_dup2                    9
#define NACL_sys_open                   10
#define NACL_sys_close                  11
#define NACL_sys_read                   12
#define NACL_sys_write                  13
#define NACL_sys_lseek                  14
#define NACL_sys_ioctl                  15
#define NACL_sys_stat                   16
#define NACL_sys_fstat                  17
#define NACL_sys_chmod                  18
#define NACL_sys_sysbrk                 20
#define NACL_sys_mmap                   21
#define NACL_sys_munmap                 22
#define NACL_sys_getdents               23
#define NACL_sys_exit                   30
#define NACL_sys_getpid                 31
#define NACL_sys_sched_yield            32
#define NACL_sys_sysconf                33
#define NACL_sys_gettimeofday           40
#define NACL_sys_clock                  41
#define NACL_sys_nanosleep              42
#define NACL_sys_imc_makeboundsock      60
#define NACL_sys_imc_accept             61
#define NACL_sys_imc_connect            62
#define NACL_sys_imc_sendmsg            63
#define NACL_sys_imc_recvmsg            64
#define NACL_sys_imc_mem_obj_create     65
#define NACL_sys_imc_socketpair         66
#define NACL_sys_mutex_create           70
#define NACL_sys_mutex_lock             71
#define NACL_sys_mutex_trylock          72
#define NACL_sys_mutex_unlock           73
#define NACL_sys_cond_create            74
#define NACL_sys_cond_wait              75
#define NACL_sys_cond_signal            76
#define NACL_sys_cond_broadcast         77
#define NACL_sys_cond_timed_wait_abs    79
#define NACL_sys_thread_create          80
#define NACL_sys_thread_exit            81
#define NACL_sys_tls_init               82
#define NACL_sys_thread_nice            83
#define NACL_sys_tls_get                84
#define NACL_sys_second_tls_set         85
#define NACL_sys_second_tls_get         86
#define NACL_sys_sem_create             100
#define NACL_sys_sem_wait               101
#define NACL_sys_sem_post               102
#define NACL_sys_sem_get_value          103
#define NACL_sys_dyncode_create         104
#define NACL_sys_dyncode_modify         105
#define NACL_sys_dyncode_delete         106
#define NACL_sys_test_infoleak          109

/*
 * DIRECT NACL SYSCALLS FUNCTIONS (VIA TRAMPOLINE)
 * should be fulfilled with all syscalls and moved to the header.
 */
/* mmap() -- nacl syscall via trampoline */
#define NaCl_mmap(start, length, prot, flags, d, offp) \
        ((int32_t (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)) \
                (NACL_sys_mmap * 0x20 + 0x10000))(start, length, prot, flags, d, offp)

/* munmap() -- nacl syscall via trampoline */
#define NaCl_munmap(start, length) ((int32_t (*)(uint32_t, uint32_t)) \
        (NACL_sys_munmap * 0x20 + 0x10000))(start, length)

/* sysbrk() -- nacl syscall via trampoline */
#define NaCl_sysbrk(new_break) ((int32_t (*)(uint32_t)) \
        (NACL_sys_sysbrk * 0x20 + 0x10000))(new_break)

/* tls_get() -- nacl syscall via trampoline */
#define NaCl_tls_get() ((int32_t (*)()) \
        (NACL_sys_tls_get * 0x20 + 0x10000))()

/* invalid syscall */
#define NaCl_invalid() ((int32_t (*)()) \
        (999 * 0x20 + 0x10000))()



#endif //ZRT_LIB_ZRTSYSCALLS_H
