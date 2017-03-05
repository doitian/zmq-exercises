#include <zmq.h>
#include <gsl/gsl_rng.h>
#include <time.h>

#include "utils.h"

int main(void) {
  void *context = zmq_ctx_new();
  void *sender = zmq_socket(context, ZMQ_PUSH);
  utils_zmq_assert(zmq_bind(sender, "tcp://*:5557"));

  void *sink = zmq_socket(context, ZMQ_PUSH);
  utils_zmq_assert(zmq_connect(sink, "tcp://localhost:5558"));

  printf("Press Enter when the workers are ready: ");
  getchar();
  printf("Sending tasks to workers...\n");

  zmq_send(sink, "0", 1, 0);

  gsl_rng *r = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set(r, (unsigned long)time(NULL));

  int task_nbr = 0;
  int total_msec = 0;
  char buff[10];

  for (task_nbr = 0; task_nbr < 100; task_nbr++) {
    int workload = (int)(gsl_rng_uniform_int(r, 100) + 1);
    total_msec += workload;
    int len = sprintf(buff, "%d", workload);
    zmq_send(sender, buff, len, 0);
  }

  gsl_rng_free(r);

  zmq_close(sender);
  zmq_close(sink);
  zmq_ctx_destroy(context);

  return 0;
}
