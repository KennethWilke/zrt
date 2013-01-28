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

#include <sys/stat.h> //mode_t

/*reserved channels list*/
#define DEV_STDIN  "/dev/stdin"
#define DEV_STDOUT "/dev/stdout"
#define DEV_STDERR "/dev/stderr"
#define DEV_FSTAB  "/dev/fstab"

#define UMASK_ENV "UMASK"

struct MountsInterface;

/*Assign own channels pointer to get it filled with channels at the syscallback*/
void zrt_setup( struct UserManifest* manifest );

void zrt_setup_finally();

/*get static object from zrtsyscalls.c*/
struct MountsInterface* transparent_mount();
/*move it from here*/
mode_t get_umask();
/*move it from here*/
mode_t apply_umask(mode_t mode);
/*move it from here*/
void debug_mes_stat(struct stat *stat);

/*
 * FULL NACL SYSCALL LIST
 */
#define NACL_sys_null                    1 /* empty syscall. does nothing */
#define NACL_sys_nameservice             2
#define NACL_3                           3 /*unknown*/
#define NACL_4                           4 /*unknown*/
#define NACL_5                           5 /*unknown*/
#define NACL_6                           6 /*unknown*/
#define NACL_7                           7 /*unknown*/
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
