#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <glib.h>

#include "zmqex.h"

#define FRONTEND_PORT "5555"
#define BACKEND_PORT "5556"

void dump_msg_t(zmq_msg_t* msg) {
  unsigned char* data = zmq_msg_data(msg);
  int len = zmq_msg_size(msg);

  printf("[%d] ", len);
  for (int i = 0; i < len; ++i) {
    printf("%02x", data[i]);
  }

  printf("\n");
}

void start_threads(const char* name, void *(thread_func)(void *), void *context, int num_threads) {
  GThread* threads[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    threads[i] = g_thread_new(name, thread_func, context);
  }

  for (int i = 0; i < num_threads; ++i) {
    g_thread_join(threads[i]);
  }
}


void broker(void *context) {
  GSList *workers = NULL;
  void *frontend = zmqex_assert_ptr(zmq_socket(context, ZMQ_ROUTER));
  void *backend = zmqex_assert_ptr(zmq_socket(context, ZMQ_ROUTER));

  zmqex_assert(zmq_bind(frontend, "tcp://*:" FRONTEND_PORT));
  zmqex_assert(zmq_bind(backend, "tcp://*:" BACKEND_PORT));

  while (1) {
    zmq_pollitem_t items[2] = {
      { backend, 0, ZMQ_POLLIN, 0 },
      { frontend, 0, ZMQ_POLLIN, 0 }
    };

    zmqex_assert(zmq_poll(items, workers == NULL ? 1 : 2, -1));

    if (items[0].revents & ZMQ_POLLIN) {
      zmq_msg_t* worker_id = g_malloc0(sizeof(zmq_msg_t));
      zmq_msg_init(worker_id);
      zmqex_assert(zmq_msg_recv(worker_id, backend, 0));
      zmqex_assert(zmq_recv(backend, NULL, 0, 0));

      printf("register worker_id: ");
      dump_msg_t(worker_id);

      zmq_msg_t client_id_or_ready;
      zmq_msg_init(&client_id_or_ready);
      zmqex_assert(zmq_msg_recv(&client_id_or_ready, backend, 0));

      if (zmq_msg_more(&client_id_or_ready)) {
        // it is client_id, send reply to frontend
        zmqex_assert(zmq_msg_send(&client_id_or_ready, frontend, ZMQ_SNDMORE));
        zmqex_assert(zmq_send(frontend, NULL, 0, ZMQ_SNDMORE));

        zmq_msg_t payload;
        zmq_msg_init(&payload);
        zmqex_assert(zmq_recv(backend, NULL, 0, 0));
        zmqex_assert(zmq_msg_recv(&payload, backend, 0));
        zmqex_assert(zmq_msg_send(&payload, frontend, 0));
      } else {
        zmq_msg_close(&client_id_or_ready);
      }

      workers = g_slist_prepend(workers, worker_id);
    }

    if (workers != NULL && (items[1].revents & ZMQ_POLLIN)) {
      GSList *top = workers;
      workers = workers->next;
      zmqex_assert(zmq_msg_send(top->data, backend, ZMQ_SNDMORE));
      zmq_msg_close(top->data);
      g_free(top->data);
      g_slist_free_1(top);
      zmqex_assert(zmq_send(backend, NULL, 0, ZMQ_SNDMORE));

      zmq_msg_t client_id;
      zmq_msg_t payload;
      zmq_msg_init(&client_id);
      zmq_msg_init(&payload);

      zmqex_assert(zmq_msg_recv(&client_id, frontend, 0));
      zmqex_assert(zmq_recv(frontend, NULL, 0, 0));
      zmqex_assert(zmq_msg_recv(&payload, frontend, 0));
      zmqex_assert(zmq_msg_send(&client_id, backend, ZMQ_SNDMORE));
      zmqex_assert(zmq_send(backend, NULL, 0, ZMQ_SNDMORE));
      zmqex_assert(zmq_msg_send(&payload, backend, 0));
    }
  }

  zmq_close(backend);
  zmq_close(frontend);
  g_slist_free_full(workers, g_free);
}

void * worker(void *context) {
  GRand *r = g_rand_new_with_seed((guint32)g_get_real_time());

  void *req = zmqex_assert_ptr(zmq_socket(context, ZMQ_REQ));
  zmqex_assert(zmq_connect(req, "tcp://localhost:" BACKEND_PORT));

  zmqex_assert(zmq_send(req, "READY", 5, 0));

  while (1) {
    zmq_msg_t client_id;
    zmq_msg_t payload;
    zmq_msg_init(&client_id);
    zmq_msg_init(&payload);

    zmqex_assert(zmq_msg_recv(&client_id, req, 0));
    zmqex_assert(zmq_recv(req, NULL, 0, 0));
    zmqex_assert(zmq_msg_recv(&payload, req, 0));

    zmqex_assert(zmq_msg_send(&client_id, req, ZMQ_SNDMORE));
    zmqex_assert(zmq_send(req, NULL, 0, ZMQ_SNDMORE));

    char* text = g_strndup(zmq_msg_data(&payload), zmq_msg_size(&payload));

    printf("Got task: %s\n", text);
    int resp = g_rand_int_range(r, 10000, 100000);
    g_usleep(resp);
    printf("Completed task: %s\n", text);

    g_free(text);
    zmq_msg_close(&payload);

    char resp_buffer[256];
    int resp_len = snprintf(resp_buffer, 256, "%d", resp);
    zmqex_assert(zmq_send(req, resp_buffer, resp_len, 0));
  }

  zmq_close(req);

  g_rand_free(r);

  return NULL;
}


void * client(void *context) {
  GRand *r = g_rand_new_with_seed((guint32)g_get_real_time());
  void *req = zmqex_assert_ptr(zmq_socket(context, ZMQ_REQ));

  zmqex_assert(zmq_connect(req, "tcp://localhost:" FRONTEND_PORT));

  char task_name[256];

  while (1) {
    int len = snprintf(task_name, 256, "task%d", g_rand_int_range(r, 0, 100000));
    zmqex_assert(zmq_send(req, task_name, len, 0));

    zmq_msg_t resp;
    zmq_msg_init(&resp);
    zmqex_assert(zmq_msg_recv(&resp, req, 0));

    char* text = g_strndup(zmq_msg_data(&resp), zmq_msg_size(&resp));

    printf("Got response: %s\n", text);

    g_free(text);
    zmq_msg_close(&resp);
  }

  zmq_close(req);
  g_rand_free(r);
  return NULL;
}

int main (int argc, char *argv[]) {
  void *context = zmqex_assert_ptr(zmq_ctx_new());

  int num_threads = 1;
  if (argc > 2) {
    num_threads = atoi(argv[2]);
  }

  if (argc > 1 && 0 == strcmp("client", argv[1])) {
    printf("---> client x %d\n", num_threads);
    start_threads("client", client, context, num_threads);
  } else if (argc > 1 && 0 == strcmp("worker", argv[1])) {
    printf("---> worker x %d\n", num_threads);
    start_threads("worker", worker, context, num_threads);
  } else {
    printf("---> broker\n");
    broker(context);
  }

  zmq_ctx_destroy(context);
  return 0;
}
