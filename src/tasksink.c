#include <zmq.h>
#include <time.h>

#include "utils.h"

int main (void) {
  //  Prepare our context and socket
  void *context = zmq_ctx_new();
  void *receiver = zmq_socket(context, ZMQ_PULL);
  zmq_bind(receiver, "tcp://*:5558");

  zmq_recv(receiver, NULL, 0, 0);

  //  Start our clock now
  unsigned long start_time = (unsigned long)time(NULL);

  //  Process 100 confirmations
  int task_nbr;
  char buff[256];
  for (task_nbr = 0; task_nbr < 100; task_nbr++) {
    int len = zmq_recv(receiver, buff, 255, 0);
    buff[len > 255 ? 255 : len] = 0;
    if ((task_nbr / 10) * 10 == task_nbr) {
      printf (":");
    } else {
      printf (".");
    }
    fflush (stdout);
  }
  //  Calculate and report duration of batch
  printf("Total elapsed time: %d seconds\n",
      (int) ((unsigned long)time(NULL) - start_time));

  zmq_close(receiver);
  zmq_ctx_destroy(context);
  return 0;
}

