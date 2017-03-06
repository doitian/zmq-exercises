#include <zmq.h>
#include <time.h>
#include <glib.h>

#include "zmqex.h"

int main(void) {
  void *context = zmq_ctx_new();
  void *sender = zmq_socket(context, ZMQ_PUSH);
  zmqex_assert(zmq_bind(sender, "tcp://*:5557"));

  void *sink = zmq_socket(context, ZMQ_PUSH);
  zmqex_assert(zmq_connect(sink, "tcp://localhost:5558"));

  printf("Press Enter when the workers are ready: ");
  getchar();
  printf("Sending tasks to workers...\n");

  zmq_send(sink, "0", 1, 0);

  GRand *r = g_rand_new_with_seed((guint32)g_get_real_time());

  int task_nbr = 0;
  int total_msec = 0;
  char buff[10];

  for (task_nbr = 0; task_nbr < 100; task_nbr++) {
    int32_t workload = g_rand_int_range(r, 1, 101);
    total_msec += workload;
    int32_t len = sprintf(buff, "%d", workload);
    zmq_send(sender, buff, len, 0);
  }

  g_rand_free(r);

  zmq_close(sender);
  zmq_close(sink);
  zmq_ctx_destroy(context);

  return 0;
}
