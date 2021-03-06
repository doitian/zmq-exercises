#include <zmq.h>
#include <time.h>
#include <stdlib.h>
#include <glib.h>

#include "zmqex.h"

int main(void) {
  void *context = zmq_ctx_new();
  void *receiver = zmq_socket(context, ZMQ_PULL);
  zmqex_assert(zmq_connect(receiver, "tcp://localhost:5557"));

  void *sender = zmq_socket(context, ZMQ_PUSH);
  zmqex_assert(zmq_connect(sender, "tcp://localhost:5558"));

  char buff[256];
  while (1) {
    int len = zmqex_assert(
        zmq_recv(receiver, buff, 255, 0)
        );
    if (len > 255) {
      len = 255;
    }
    buff[len] = 0;

    printf("%s.", buff);
    fflush(stdout);

    g_usleep(atoi(buff) * 1000);
    zmq_send(sender, NULL, 0, 0);
  }

  zmq_close(receiver);
  zmq_close(sender);
  zmq_ctx_destroy(context);

  return 0;
}
