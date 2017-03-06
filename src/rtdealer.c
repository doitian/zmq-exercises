#include <zmq.h>
#include <glib.h>
#include <string.h>

#include "zmqex.h"

static volatile gint worker_id = 1;

static void * worker(void *context) {
  GRand *r = g_rand_new_with_seed((guint32)g_get_real_time());
  void *dispatcher = zmqex_assert_ptr(zmq_socket(context, ZMQ_DEALER));

  char identity[16];
  int len = sprintf(identity, "worker-%d", g_atomic_int_add(&worker_id, 1));
  zmqex_assert(zmq_setsockopt(dispatcher, ZMQ_IDENTITY, identity, len));

  zmqex_assert(zmq_connect(dispatcher, "inproc://dispatcher"));

  int total = 0;
  while (1) {
    zmqex_assert(zmq_send(dispatcher, NULL, 0, 0));

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmqex_assert(zmq_msg_recv(&msg, dispatcher, 0));

    if (zmq_msg_size(&msg) == 4 && 0 == memcmp(zmq_msg_data(&msg), "QUIT", 4)) {
      printf("%s completed %d tasks\n", identity, total);
      break;
    }

    zmq_msg_close(&msg);
    g_usleep(g_rand_int_range(r, 1000, 100000));
    total += 1;
  }

  zmq_close(dispatcher);
  return NULL;
}

int main(void) {
  void *context = zmqex_assert_ptr(zmq_ctx_new());
  void *dispatcher = zmqex_assert_ptr(zmq_socket(context, ZMQ_ROUTER));

  zmqex_assert(zmq_bind(dispatcher, "inproc://dispatcher"));

  GThread *threads[5];
  int nthreads = sizeof(threads)/sizeof(GThread *);
  for (int i = 0; i < nthreads; ++i) {
    threads[i] = g_thread_new("worker", worker, context);
  }

  GTimer *timer = g_timer_new();
  int completed_threads = 0;
  while (1) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmqex_assert(zmq_msg_recv(&msg, dispatcher, 0));
    zmqex_assert(zmq_msg_send(&msg, dispatcher, ZMQ_SNDMORE));
    zmq_msg_close(&msg);

    zmqex_assert(zmq_recv(dispatcher, NULL, 0, 0));

    if (g_timer_elapsed(timer, NULL) > 5.0) {
      zmqex_assert(zmq_send(dispatcher, "QUIT", 4, 0));
      ++completed_threads;
      if (completed_threads >= nthreads) {
        break;
      }
    } else {
      zmqex_assert(zmq_send(dispatcher, NULL, 0, 0));
    }
  }
  g_timer_destroy(timer);

  for (int i = 0; i < nthreads; ++i) {
    g_thread_join(threads[i]);
  }
  zmq_close(dispatcher);
  zmq_ctx_destroy(context);
  return 0;
}
