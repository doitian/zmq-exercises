#ifndef _UTILS_H
#define _UTILS_H

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

static inline int utils_zmq_assert(int errnum) {
  if (errnum != 0) {
    fprintf(stderr, "zmq: %s\n", zmq_strerror(errnum));
    exit(errnum);
  }

  return errnum;
}

static inline int utils_zmq_assert_errno(int len) {
  if (len == -1) {
    int save_errno = zmq_errno();
    fprintf(stderr, "zmq: %s\n", strerror(save_errno));
    exit(save_errno);
  }

  return len;
}

static inline void utils_sleep_ms(int ms)
{
#ifdef LINUX
  usleep(ms * 1000);
#endif
#ifdef WINDOWS
  Sleep(ms);
#endif
}

#endif // _UTILS_H
