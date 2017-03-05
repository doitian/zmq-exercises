#include <zmq.h>
#include <gsl/gsl_rng.h>
#include <time.h>
#include <stdlib.h>

#include "utils.h"

int main(void) {
  void *context = zmq_ctx_new();
  void *receiver = zmq_socket(context, ZMQ_PULL);
  utils_zmq_assert(zmq_connect(receiver, "tcp://localhost:5557"));

  void *sender = zmq_socket(context, ZMQ_PUSH);
  utils_zmq_assert(zmq_connect(sender, "tcp://localhost:5558"));

  char buff[256];
  while (1) {
    int len = utils_zmq_assert_errno(
        zmq_recv(receiver, buff, 255, 0)
        );
    if (len > 255) {
      len = 255;
    }
    buff[len] = 0;

    printf("%s.", buff);
    fflush(stdout);

    utils_sleep_ms(atoi(buff));
    zmq_send(sender, NULL, 0, 0);
  }

  zmq_close(receiver);
  zmq_close(sender);
  zmq_ctx_destroy(context);

  return 0;
}
