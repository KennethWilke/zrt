/*
 * syscallbacks.c
 * Syscallbacks implementation used by zrt;
 *
 *  Created on: 6.07.2012
 *      Author: YaroslavLitvinov
 */

#include <sys/types.h> //off_t
#include <string.h> //memcpy
#include <stdio.h>  //SEEK_SET .. SEEK_END
#include <stdlib.h> //atoi
#include <stddef.h> //size_t
#include <errno.h>

#include "zvm.h"
#include "zrt.h"
#include "unistd.h" //STDIN_FILENO
#include "syscallbacks.h"



/* ******************************************************************************
 * Syscallbacks debug macros*/
#ifdef DEBUG
#define SHOWID {log_msg((char*)__func__); log_msg("() is called\n");}
#else
#define SHOWID
#endif

/* ******************************************************************************
 * Syscallback mocks helper macroses*/
#define JOIN(x,y) x##y
#define ZRT_FUNC(x) JOIN(zrt_, x)

/* mock. replacing real syscall handler */
#define SYSCALL_MOCK(name_wo_zrt_prefix, code) \
		static int32_t ZRT_FUNC(name_wo_zrt_prefix)(uint32_t *args)\
		{\
	SHOWID;\
	return code;\
		}


/*manifest should be initialized in zrt main*/
static struct UserManifest* s_manifest;
/* positions of zerovm channels. should be allocated before usage */
static size_t *s_pos_ptr;



#ifdef DEBUG
static int s_fd_debug_log = -1;
void log_msg(const char *msg)
{
	const char *fname= ZVM_STDERR;
	if ( s_fd_debug_log == -1 ){
		/* search for name through the channels */
		int handle;
		for(handle = 0; handle < s_manifest->channels_count; ++handle)
			if(strcmp(s_manifest->channels[handle].name, fname) == 0)
				s_fd_debug_log = handle;
	}

	if ( s_fd_debug_log != -1 ){
		int length = zvm_pwrite(s_fd_debug_log, msg, strlen(msg), s_pos_ptr[s_fd_debug_log]);
		if(length > 0) s_pos_ptr[s_fd_debug_log] += length;
	}
}

size_t strlen(const char *string) {
	const char *s;
	s = string;
	while (*s)
		s++;
	return s - string;
}

char *strrev(char *str) {
	char *p1, *p2;
	if (!str || !*str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

char *itoa(int n, char *s, int b) {
	static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	int i=0, sign;
	if ((sign = n) < 0)
		n = -n;
	do {
		s[i++] = digits[n % b];
	} while ((n /= b) > 0);
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	return strrev(s);
}

int log_int(int dec)
{
	char log_msg_text[30];
	itoa(dec, (char*)log_msg_text, 10);
	log_msg(log_msg_text);
	return 0;
}
#endif

/*
 * ZRT IMPLEMENTATION OF NACL SYSCALLS
 * each nacl syscall must be implemented or, at least, mocked. no exclusions!
 */

SYSCALL_MOCK(nan, 0)
SYSCALL_MOCK(null, 0)
SYSCALL_MOCK(nameservice, 0)
SYSCALL_MOCK(dup, -EPERM) /* duplicate the given file handle. n/a in the simple zrt version */
SYSCALL_MOCK(dup2, -EPERM) /* duplicate the given file handle. n/a in the simple zrt version */


/*
 * return channel handle by given name. if given flags
 * doesn't answer channel's type/limits
 */
static int32_t zrt_open(uint32_t *args)
{
	SHOWID;
	char* name = (char*)args[0];
	//  int flags = (int)args[1];
	//  int mode = (int)args[2];
	int handle;

	/* search for name through the channels */
	for(handle = 0; handle < s_manifest->channels_count; ++handle)
		if(strcmp(s_manifest->channels[handle].name, name) == 0)
			return handle;

	/* todo: check flags and mode against type and limits/counters */
	return -ENOENT;
}

/* do nothing but checks given handle */
static int32_t zrt_close(uint32_t *args)
{
	SHOWID;
	//  int handle = (int)args[0];
	int result = -EBADF;

	/*
	 * todo: did channel was "opened" with zrt_open() ?
	 * if so fix channel position and return OK
	 * otherwise - appropriate error
	 */

	return result;
}


/* read the file with the given handle number */
static int32_t zrt_read(uint32_t *args)
{
	SHOWID;
	int file = (int)args[0];
	void *buf = (void*)args[1];
	int64_t length = (int64_t)args[2];

	/**DEBUG*******************************************/
	log_msg( "read: length=" ); log_int( length ); log_msg( "\n" );
	/**DEBUG*******************************************/

	/* for the new zvm channels design */
	length = zvm_pread(file, buf, length, s_pos_ptr[file]);
	if(length > 0) s_pos_ptr[file] += length;

	/**DEBUG*******************************************/
	log_msg( "read: readed=" ); log_int( length ); log_msg( buf ); log_msg( "\n" );
	/**DEBUG*******************************************/

	return length;
}

/* example how to implement zrt syscall */
static int32_t zrt_write(uint32_t *args)
{
	SHOWID;
	int file = (int)args[0];
	void *buf = (void*)args[1];
	int64_t length = (int64_t)args[2];

	/* for the new zvm channels design */
	length = zvm_pwrite(file, buf, length, s_pos_ptr[file]);
	if(length > 0) s_pos_ptr[file] += length;

	return length;
}

/*
 * seek for the new zerovm channels design
 */
static int32_t zrt_lseek(uint32_t *args)
{
	SHOWID;
	struct ZVMChannel *channel;

#define CHECK_NEW_POS(offset)\
		if(offset < 0 || offset > 0x7fffffff)\
		{\
			errno = EPERM; /* in advanced version should be set to conventional value */\
		return -EPERM;\
		}

	int32_t handle = (int32_t)args[0];
	off_t offset = *((off_t*)args[1]);
	int whence = (int)args[2];
	off_t new_pos;

	/* check handle */
	if(handle < 0 || handle >= s_manifest->channels_count) return EBADF;

	/* select channel and make checks */
	channel = &s_manifest->channels[handle];
	if(channel->position != s_pos_ptr[handle]) return EIO;

	/* check if channel has random access */
	if(channel->type == SGetSPut) return ESPIPE;
	if(channel->type == Stdin) return ESPIPE;
	if(channel->type == Stdout) return ESPIPE;
	if(channel->type == Stderr) return ESPIPE;

	/*
	 * following check doesn't garantee absence of errors since
	 * it does not envolves counters update
	 * todo(d'b): try to solve it w/o syscalls
	 */
	/* sequential r/o channel */
	if(channel->type == SGetRPut && (channel->limits[PutsLimit] == 0
			|| channel->limits[PutSizeLimit] == 0)) return ESPIPE;
	/* sequential w/o channel */
	if(channel->type == RGetSPut && (channel->limits[GetsLimit] == 0
			|| channel->limits[GetSizeLimit] == 0)) return ESPIPE;

	switch(whence)
	{
	case SEEK_SET:
		CHECK_NEW_POS(offset);
		s_pos_ptr[handle] = offset;
		break;
	case SEEK_CUR:
		new_pos = s_pos_ptr[handle] + offset;
		CHECK_NEW_POS(new_pos);
		s_pos_ptr[handle] = new_pos;
		break;
	case SEEK_END:
		new_pos = channel->size + offset;
		CHECK_NEW_POS(new_pos);
		s_pos_ptr[handle] = new_pos;
		break;
	default:
		errno = EPERM; /* in advanced version should be set to conventional value */
		return -EPERM;
	}

	/*
	 * return current position in a special way since 64 bits
	 * doesn't fit to return code (32 bits)
	 */
	*(off_t *)args[1] = s_pos_ptr[handle];
	return 0;
}


SYSCALL_MOCK(ioctl, -EINVAL) /* not implemented in the simple version of zrtlib */

/*
 * return synthetic channel information
 * todo(d'b): the function needs update after the channels design will complete
 */
static int32_t zrt_stat(uint32_t *args)
{
  SHOWID;
//  char *prefixes[] = {STDIN, STDOUT, STDERR};
  const char *file = (const char*)args[0];
  struct nacl_abi_stat *sbuf = (struct nacl_abi_stat *)args[1];
//  struct ZVMChannel *channel;
  int handle;

  /**DEBUG*******************************************/
  log_msg( file ); log_msg( "\n" );
  /**DEBUG*******************************************/

  /* calculate handle number */
  if(file == NULL) return -EFAULT;
  for(handle = STDIN_FILENO; handle < s_manifest->channels_count; ++handle)
    if(!strcmp(file, s_manifest->channels[handle].name)) break;

  /* todo(d'b): make difference for random/sequential channels */

  /* return stat object */
  sbuf->nacl_abi_st_dev = 2049;     /* ID of device containing handle */
  sbuf->nacl_abi_st_ino = 1967;     /* inode number */
  sbuf->nacl_abi_st_mode = 33261;   /* protection */
  sbuf->nacl_abi_st_nlink = 1;      /* number of hard links */
  sbuf->nacl_abi_st_uid = 1000;     /* user ID of owner */
  sbuf->nacl_abi_st_gid = 1000;     /* group ID of owner */
  sbuf->nacl_abi_st_rdev = 0;       /* device ID (if special handle) */
  sbuf->nacl_abi_st_size = s_manifest->channels[handle].size; /* size in bytes */
  sbuf->nacl_abi_st_blksize = 4096; /* block size for file system I/O */
  sbuf->nacl_abi_st_blocks = /* number of 512B blocks allocated */
      ((sbuf->nacl_abi_st_size + sbuf->nacl_abi_st_blksize - 1)
      / sbuf->nacl_abi_st_blksize) * sbuf->nacl_abi_st_blksize / 512;

  /* files are not allowed to have real date/time */
  sbuf->nacl_abi_st_atime = 0;      /* time of the last access */
  sbuf->nacl_abi_st_mtime = 0;      /* time of the last modification */
  sbuf->nacl_abi_st_ctime = 0;      /* time of the last status change */
  sbuf->nacl_abi_st_atimensec = 0;
  sbuf->nacl_abi_st_mtimensec = 0;
  sbuf->nacl_abi_st_ctimensec = 0;

  return 0;
}

/* return synthetic channel information */
static int32_t zrt_fstat(uint32_t *args)
{
  SHOWID;
  int handle = (int)args[0];
  struct nacl_abi_stat *sbuf = (struct nacl_abi_stat *)args[1];
  struct ZVMChannel *channel;

  /**DEBUG*******************************************/
  log_msg( "fstat: " ); log_int( handle ); log_msg( "\n" );
  /**DEBUG*******************************************/

  /* check if user request contain the proper file handle */
  if(handle < STDIN_FILENO || handle >= s_manifest->channels_count) return -EBADF;
  channel = &s_manifest->channels[handle];

  /* return stat object */
  sbuf->nacl_abi_st_dev = 2049; /* ID of device containing handle */
  sbuf->nacl_abi_st_ino = 1967; /* inode number */
  sbuf->nacl_abi_st_mode = 33261; /* protection */
  sbuf->nacl_abi_st_nlink = 1; /* number of hard links */
  sbuf->nacl_abi_st_uid = 1000; /* user ID of owner */
  sbuf->nacl_abi_st_gid = 1000; /* group ID of owner */
  sbuf->nacl_abi_st_rdev = 0; /* device ID (if special handle) */
  sbuf->nacl_abi_st_size = channel->size; /* total size, in bytes */
  sbuf->nacl_abi_st_blksize = 4096; /* blocksize for file system I/O */
  sbuf->nacl_abi_st_blocks = /* number of 512B blocks allocated */
      ((sbuf->nacl_abi_st_size + sbuf->nacl_abi_st_blksize - 1)
      / sbuf->nacl_abi_st_blksize) * sbuf->nacl_abi_st_blksize / 512;

  /* files are not allowed to have real date/time */
  sbuf->nacl_abi_st_atime = 0; /* time of the last access */
  sbuf->nacl_abi_st_mtime = 0; /* time of the last modification */
  sbuf->nacl_abi_st_ctime = 0; /* time of the last status change */
  sbuf->nacl_abi_st_atimensec = 0;
  sbuf->nacl_abi_st_mtimensec = 0;
  sbuf->nacl_abi_st_ctimensec = 0;

  return 0;
}

SYSCALL_MOCK(chmod, -EPERM) /* in a simple version of zrt chmod is not allowed */

/*
 * following 3 functions (sysbrk, mmap, munmap) is the part of the
 * new memory engine. the new allocate specified amount of ram before
 * user code start and then user can only obtain that allocated memory.
 * zrt lib will help user to do it transparently.
 */

/* change space allocation */
static int32_t zrt_sysbrk(uint32_t *args)
{
  SHOWID;
  int32_t retcode;

  zvm_syscallback(0); /* uninstall syscallback */
  retcode = NaCl_sysbrk(args[0]); /* invoke syscall directly */
  zvm_syscallback((intptr_t)syscall_director); /* reinstall syscallback */

  return retcode;
}

/* map region of memory */
static int32_t zrt_mmap(uint32_t *args)
{
  SHOWID;
  int32_t retcode;

  zvm_syscallback(0); /* uninstall syscallback */
  retcode = NaCl_mmap(args[0], args[1], args[2], args[3], args[4], args[5]);
  zvm_syscallback((intptr_t)syscall_director); /* reinstall syscallback */

  return retcode;
}

/*
 * unmap region of memory
 * note: zerovm doesn't use it in memory management.
 * instead of munmap it use mmap with protection 0
 */
static int32_t zrt_munmap(uint32_t *args)
{
  SHOWID;
  int32_t retcode;

  zvm_syscallback(0); /* uninstall syscallback */
  retcode = NaCl_munmap(args[0], args[1]);
  zvm_syscallback((intptr_t)syscall_director); /* reinstall syscallback */

  return retcode;
}

SYSCALL_MOCK(getdents, 0)

/*
 * exit. most important syscall. without it the user program
 * cannot terminate correctly.
 */
static int32_t zrt_exit(uint32_t *args)
{
  /* no need to check args for NULL. it is always set by syscall_manager */
  SHOWID;
  zvm_exit(args[0]);
  return 0; /* unreachable */
}

SYSCALL_MOCK(getpid, 0)
SYSCALL_MOCK(sched_yield, 0)
SYSCALL_MOCK(sysconf, 0)

/* if given in manifest let user to have it */
#define TIMESTAMP_STR "TimeStamp="
#define TIMESTAMP_STRLEN strlen("TimeStamp=")
static int32_t zrt_gettimeofday(uint32_t *args)
{
  SHOWID;
  struct nacl_abi_timeval  *tv = (struct nacl_abi_timeval *)args[0];
  char *stamp = NULL;
  int i;

  /* get time stamp from the environment */
  for(i = 0; s_manifest->envp[i] != NULL; ++i)
  {
    if(strncmp(s_manifest->envp[i], TIMESTAMP_STR, TIMESTAMP_STRLEN) != 0)
      continue;

    stamp = s_manifest->envp[i] + TIMESTAMP_STRLEN;
    break;
  }

  /* check if timestampr is set */
  if(stamp == NULL || !*stamp) return -EPERM;

  /* check given arguments validity */
  if(!tv) return -EFAULT;

  tv->nacl_abi_tv_usec = 0; /* to simplify code. yet we can't get msec from nacl code */
  tv->nacl_abi_tv_sec = atoi(stamp); /* manifest always contain decimal values */

  return 0;
}

/*
 * should never be implemented if we want deterministic behaviour
 * note: but we can allow to return each time synthetic value
 * warning! after checking i found that nacl is not using it, so this
 *          function is useless for current nacl sdk version.
 */
SYSCALL_MOCK(clock, -EPERM)
SYSCALL_MOCK(nanosleep, 0)
SYSCALL_MOCK(imc_makeboundsock, 0)
SYSCALL_MOCK(imc_accept, 0)
SYSCALL_MOCK(imc_connect, 0)
SYSCALL_MOCK(imc_sendmsg, 0)
SYSCALL_MOCK(imc_recvmsg, 0)
SYSCALL_MOCK(imc_mem_obj_create, 0)
SYSCALL_MOCK(imc_socketpair, 0)
SYSCALL_MOCK(mutex_create, 0)
SYSCALL_MOCK(mutex_lock, 0)
SYSCALL_MOCK(mutex_trylock, 0)
SYSCALL_MOCK(mutex_unlock, 0)
SYSCALL_MOCK(cond_create, 0)
SYSCALL_MOCK(cond_wait, 0)
SYSCALL_MOCK(cond_signal, 0)
SYSCALL_MOCK(cond_broadcast, 0)
SYSCALL_MOCK(cond_timed_wait_abs, 0)
SYSCALL_MOCK(thread_create, 0)
SYSCALL_MOCK(thread_exit, 0)
SYSCALL_MOCK(tls_init, 0)
SYSCALL_MOCK(thread_nice, 0)

/*
 * get tls from zerovm. can be replaced with untrusted version
 * after "blob library" loader will be ready
 */
static int32_t zrt_tls_get(uint32_t *args)
{
  SHOWID;
  int32_t retcode;

  zvm_syscallback(0); /* uninstall syscallback */
  retcode = NaCl_tls_get(); /* invoke syscall directly */
  zvm_syscallback((intptr_t)syscall_director); /* reinstall syscallback */

  return retcode;
}

SYSCALL_MOCK(second_tls_set, 0)

/*
 * get second tls.
 * since we only have single thread use instead of 2nd tls 1st one
 */
static int32_t zrt_second_tls_get(uint32_t *args)
{
  SHOWID;
  return zrt_tls_get(NULL);
}

SYSCALL_MOCK(sem_create, 0)
SYSCALL_MOCK(sem_wait, 0)
SYSCALL_MOCK(sem_post, 0)
SYSCALL_MOCK(sem_get_value, 0)
SYSCALL_MOCK(dyncode_create, 0)
SYSCALL_MOCK(dyncode_modify, 0)
SYSCALL_MOCK(dyncode_delete, 0)
SYSCALL_MOCK(test_infoleak, 0)

/*
 * array of the pointers to zrt syscalls
 *
 * each zrt function (syscall) has uniform prototype: int32_t syscall(uint32_t *args)
 * where "args" pointer to syscalls' arguments. number and types of arguments
 * can be found in the nacl "nacl_syscall_handlers.c" file.
 */
int32_t (*zrt_syscalls[])(uint32_t*) = {
    zrt_nan,                   /* 0 -- not implemented syscall */
    zrt_null,                  /* 1 -- empty syscall. does nothing */
    zrt_nameservice,           /* 2 */
    zrt_nan,                   /* 3 -- not implemented syscall */
    zrt_nan,                   /* 4 -- not implemented syscall */
    zrt_nan,                   /* 5 -- not implemented syscall */
    zrt_nan,                   /* 6 -- not implemented syscall */
    zrt_nan,                   /* 7 -- not implemented syscall */
    zrt_dup,                   /* 8 */
    zrt_dup2,                  /* 9 */
    zrt_open,                  /* 10 */
    zrt_close,                 /* 11 */
    zrt_read,                  /* 12 */
    zrt_write,                 /* 13 */
    zrt_lseek,                 /* 14 */
    zrt_ioctl,                 /* 15 */
    zrt_stat,                  /* 16 */
    zrt_fstat,                 /* 17 */
    zrt_chmod,                 /* 18 */
    zrt_nan,                   /* 19 -- not implemented syscall */
    zrt_sysbrk,                /* 20 */
    zrt_mmap,                  /* 21 */
    zrt_munmap,                /* 22 */
    zrt_getdents,              /* 23 */
    zrt_nan,                   /* 24 -- not implemented syscall */
    zrt_nan,                   /* 25 -- not implemented syscall */
    zrt_nan,                   /* 26 -- not implemented syscall */
    zrt_nan,                   /* 27 -- not implemented syscall */
    zrt_nan,                   /* 28 -- not implemented syscall */
    zrt_nan,                   /* 29 -- not implemented syscall */
    zrt_exit,                  /* 30 -- must use trap:exit() */
    zrt_getpid,                /* 31 */
    zrt_sched_yield,           /* 32 */
    zrt_sysconf,               /* 33 */
    zrt_nan,                   /* 34 -- not implemented syscall */
    zrt_nan,                   /* 35 -- not implemented syscall */
    zrt_nan,                   /* 36 -- not implemented syscall */
    zrt_nan,                   /* 37 -- not implemented syscall */
    zrt_nan,                   /* 38 -- not implemented syscall */
    zrt_nan,                   /* 39 -- not implemented syscall */
    zrt_gettimeofday,          /* 40 */
    zrt_clock,                 /* 41 */
    zrt_nanosleep,             /* 42 */
    zrt_nan,                   /* 43 -- not implemented syscall */
    zrt_nan,                   /* 44 -- not implemented syscall */
    zrt_nan,                   /* 45 -- not implemented syscall */
    zrt_nan,                   /* 46 -- not implemented syscall */
    zrt_nan,                   /* 47 -- not implemented syscall */
    zrt_nan,                   /* 48 -- not implemented syscall */
    zrt_nan,                   /* 49 -- not implemented syscall */
    zrt_nan,                   /* 50 -- not implemented syscall */
    zrt_nan,                   /* 51 -- not implemented syscall */
    zrt_nan,                   /* 52 -- not implemented syscall */
    zrt_nan,                   /* 53 -- not implemented syscall */
    zrt_nan,                   /* 54 -- not implemented syscall */
    zrt_nan,                   /* 55 -- not implemented syscall */
    zrt_nan,                   /* 56 -- not implemented syscall */
    zrt_nan,                   /* 57 -- not implemented syscall */
    zrt_nan,                   /* 58 -- not implemented syscall */
    zrt_nan,                   /* 59 -- not implemented syscall */
    zrt_imc_makeboundsock,     /* 60 */
    zrt_imc_accept,            /* 61 */
    zrt_imc_connect,           /* 62 */
    zrt_imc_sendmsg,           /* 63 */
    zrt_imc_recvmsg,           /* 64 */
    zrt_imc_mem_obj_create,    /* 65 */
    zrt_imc_socketpair,        /* 66 */
    zrt_nan,                   /* 67 -- not implemented syscall */
    zrt_nan,                   /* 68 -- not implemented syscall */
    zrt_nan,                   /* 69 -- not implemented syscall */
    zrt_mutex_create,          /* 70 */
    zrt_mutex_lock,            /* 71 */
    zrt_mutex_trylock,         /* 72 */
    zrt_mutex_unlock,          /* 73 */
    zrt_cond_create,           /* 74 */
    zrt_cond_wait,             /* 75 */
    zrt_cond_signal,           /* 76 */
    zrt_cond_broadcast,        /* 77 */
    zrt_nan,                   /* 78 -- not implemented syscall */
    zrt_cond_timed_wait_abs,   /* 79 */
    zrt_thread_create,         /* 80 */
    zrt_thread_exit,           /* 81 */
    zrt_tls_init,              /* 82 */
    zrt_thread_nice,           /* 83 */
    zrt_tls_get,               /* 84 */
    zrt_second_tls_set,        /* 85 */
    zrt_second_tls_get,        /* 86 */
    zrt_nan,                   /* 87 -- not implemented syscall */
    zrt_nan,                   /* 88 -- not implemented syscall */
    zrt_nan,                   /* 89 -- not implemented syscall */
    zrt_nan,                   /* 90 -- not implemented syscall */
    zrt_nan,                   /* 91 -- not implemented syscall */
    zrt_nan,                   /* 92 -- not implemented syscall */
    zrt_nan,                   /* 93 -- not implemented syscall */
    zrt_nan,                   /* 94 -- not implemented syscall */
    zrt_nan,                   /* 95 -- not implemented syscall */
    zrt_nan,                   /* 96 -- not implemented syscall */
    zrt_nan,                   /* 97 -- not implemented syscall */
    zrt_nan,                   /* 98 -- not implemented syscall */
    zrt_nan,                   /* 99 -- not implemented syscall */
    zrt_sem_create,            /* 100 */
    zrt_sem_wait,              /* 101 */
    zrt_sem_post,              /* 102 */
    zrt_sem_get_value,         /* 103 */
    zrt_dyncode_create,        /* 104 */
    zrt_dyncode_modify,        /* 105 */
    zrt_dyncode_delete,        /* 106 */
    zrt_nan,                   /* 107 -- not implemented syscall */
    zrt_nan,                   /* 108 -- not implemented syscall */
    zrt_test_infoleak          /* 109 */
};


void zrt_setup( struct UserManifest* manifest ){
	s_manifest = manifest;
	/* set up internals */
	s_pos_ptr = calloc(manifest->channels_count, sizeof(manifest->channels_count));
}


