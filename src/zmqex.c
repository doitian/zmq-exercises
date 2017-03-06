#include "zmqex.h"

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int zmqex_assert(int retcode) {
  if (retcode == -1) {
    int err = zmq_errno();
    fprintf(stderr, "zmq: %s\n", zmq_strerror(err));
    exit(err);
  }

  return retcode;
}

int zmqex_dump(void *socket) {
  int rc;
  int more = 0;

  do {
    zmq_msg_t msg;
    zmq_msg_init(&msg);

    rc = zmq_msg_recv(&msg, socket, 0);
    if (rc == -1) {
      zmq_msg_close(&msg);
      return rc;
    }

    int len = zmq_msg_size(&msg);
    unsigned char* data = zmq_msg_data(&msg);
    int istext = 1;
    for (int i = 0; i < len; ++i) {
      if (data[i] < 32 || data[i] > 126) {
        istext = 0;
        break;
      }
    }
    printf("[%d] ", len);
    if (istext) {
      for (int i = 0; i < len; ++i) {
        printf("%c", data[i]);
      }
    } else {
      for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
      }
    }
    printf("\n");

    more = zmq_msg_more(&msg);
    zmq_msg_close(&msg);
  } while(more);
  printf("------------------------------\n");
}
