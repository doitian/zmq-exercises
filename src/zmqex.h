#ifndef _ZMQEX_H
#define _ZMQEX_H

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

static inline int zmqex_assert(int retcode) {
  if (retcode == -1) {
    int err = zmq_errno();
    fprintf(stderr, "zmq: %s\n", zmq_strerror(err));
    exit(err);
  }

  return retcode;
}

static inline void zmqex_sleep_ms(int ms) {
#ifdef WINDOWS
  Sleep(ms);
#else
  usleep(ms * 1000);
#endif
}

#endif // _ZMQEX_H
