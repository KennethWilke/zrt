#ifndef __ZCALLS_H__
#define __ZCALLS_H__

#include <time.h>   /*clock_t*/

#include "zrt.h"

/*forwards*/
struct stat;
struct dirent;
struct timeval;

struct MountsInterface;

/***************************************************************************
* Declaration of ZLIBC syscall implementations that used as main syscall 
* handlers while prolog initializing. Function with prefix zrt_zcall_prolog_
* should be used for initialization of struct zcalls_init_t in __query_zcalls.
***************************************************************************/

/******************* zcalls_init_t functions **************/
void zrt_zcall_prolog_init(void);
/* irt basic *************************/
void zrt_zcall_prolog_exit(int status);
int zrt_zcall_prolog_gettod(struct timeval *tv);
int zrt_zcall_prolog_clock(clock_t *ticks);
int zrt_zcall_prolog_nanosleep(const struct timespec *req, struct timespec *rem);
int zrt_zcall_prolog_sched_yield(void);
int zrt_zcall_prolog_sysconf(int name, int *value);
/* irt fdio *************************/
int zrt_zcall_prolog_close(int fd);
int zrt_zcall_prolog_dup(int fd, int *newfd);
int zrt_zcall_prolog_dup2(int fd, int newfd);
int zrt_zcall_prolog_read(int fd, void *buf, size_t count, size_t *nread);
int zrt_zcall_prolog_write(int fd, const void *buf, size_t count, size_t *nwrote);
int zrt_zcall_prolog_seek(int fd, off_t offset, int whence, off_t *new_offset);
int zrt_zcall_prolog_fstat(int fd, struct stat *);
int zrt_zcall_prolog_getdents(int fd, struct dirent *, size_t count, size_t *nread);
/* irt filename *************************/
int zrt_zcall_prolog_open(const char *pathname, int oflag, mode_t cmode, int *newfd);
int zrt_zcall_prolog_stat(const char *pathname, struct stat *);
/* irt memory *************************/
int zrt_zcall_prolog_sysbrk(void **newbrk);
int zrt_zcall_prolog_mmap(void **addr, size_t len, int prot, int flags, int fd, off_t off);
int zrt_zcall_prolog_munmap(void *addr, size_t len);
/* irt dyncode *************************/
int zrt_zcall_prolog_dyncode_create(void *dest, const void *src, size_t size);
int zrt_zcall_prolog_dyncode_modify(void *dest, const void *src, size_t size);
int zrt_zcall_prolog_dyncode_delete(void *dest, size_t size);
/* irt thread *************************/
int zrt_zcall_prolog_thread_create(void *start_user_address, void *stack, void *thread_ptr);
void zrt_zcall_prolog_thread_exit(int32_t *stack_flag);
int zrt_zcall_prolog_thread_nice(const int nice);
/* irt mutex *************************/
int zrt_zcall_prolog_mutex_create(int *mutex_handle);
int zrt_zcall_prolog_mutex_destroy(int mutex_handle);
int zrt_zcall_prolog_mutex_lock(int mutex_handle);
int zrt_zcall_prolog_mutex_unlock(int mutex_handle);
int zrt_zcall_prolog_mutex_trylock(int mutex_handle);
/* irt cond *************************/
int zrt_zcall_prolog_cond_create(int *cond_handle);
int zrt_zcall_prolog_cond_destroy(int cond_handle);
int zrt_zcall_prolog_cond_signal(int cond_handle);
int zrt_zcall_prolog_cond_broadcast(int cond_handle);
int zrt_zcall_prolog_cond_wait(int cond_handle, int mutex_handle);
int zrt_zcall_prolog_cond_timed_wait_abs(int cond_handle, int mutex_handle,
			   const struct timespec *abstime);
/* irt tls *************************/
int zrt_zcall_prolog_tls_init(void *thread_ptr);
void *zrt_zcall_prolog_tls_get(void);
/* irt resource open *************************/
int zrt_zcall_prolog_open_resource(const char *file, int *fd);
/* irt clock *************************/
int zrt_zcall_prolog_getres(clockid_t clk_id, struct timespec *res);
int zrt_zcall_prolog_gettime(clockid_t clk_id, struct timespec *tp);


/************************** zcalls_zrt_t functions **************/
void zrt_zcall_prolog_zrt_setup(void);


/***************************************************************************
* Declaration of ZLIBC syscall implementations that used as secondary syscall
* handlers with enhanced functionality, can be used only after prolog init done;
***************************************************************************/

/* irt basic *************************/
void zrt_zcall_enhanced_exit(int status);
int zrt_zcall_enhanced_gettod(struct timeval *tv);
/* irt fdio *************************/
int zrt_zcall_enhanced_close(int fd);
int zrt_zcall_enhanced_dup(int fd, int *newfd);
int zrt_zcall_enhanced_dup2(int fd, int newfd);
int zrt_zcall_enhanced_read(int fd, void *buf, size_t count, size_t *nread);
int zrt_zcall_enhanced_write(int fd, const void *buf, size_t count, size_t *nwrote);
int zrt_zcall_enhanced_seek(int fd, off_t offset, int whence, off_t *new_offset);
int zrt_zcall_enhanced_fstat(int fd, struct stat *);
int zrt_zcall_enhanced_getdents(int fd, struct dirent *, size_t count, size_t *nread);
/* irt filename *************************/
int zrt_zcall_enhanced_open(const char *pathname, int oflag, mode_t cmode, int *newfd);
int zrt_zcall_enhanced_stat(const char *pathname, struct stat *);
/* irt memory *************************/
int zrt_zcall_enhanced_sysbrk(void **newbrk);
int zrt_zcall_enhanced_mmap(void **addr, size_t len, int prot, int flags, int fd, off_t off);
int zrt_zcall_enhanced_munmap(void *addr, size_t len);

/************************** zcalls_zrt_t functions **************/
void zrt_zcall_enhanced_zrt_setup(void);



#endif //__ZCALLS_H__


