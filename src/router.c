#include <zmq.h>

#include "zmqex.h"

int main(void) {
  void *context = zmq_ctx_new();

  void *router = zmq_socket(context, ZMQ_ROUTER);
  zmq_bind(router, "inproc://router");

  void *sender = zmq_socket(context, ZMQ_REQ);
  zmq_setsockopt(sender, ZMQ_IDENTITY, "REQ", 3);
  zmq_connect(sender, "inproc://router");

  zmq_send(sender, "hello", 5, 0);

  zmq_recv(router, NULL, 0, 0);
  zmq_recv(router, NULL, 0, 0);
  zmqex_assert(zmq_recv(router, NULL, 0, 0));

  zmq_send(router, "REQ", 3, ZMQ_SNDMORE);
  zmq_send(router, NULL, 0, ZMQ_SNDMORE);
  zmq_send(router, "world", 5, 0);

  zmqex_dump(sender);

  zmq_close(sender);
  zmq_close(router);
  zmq_ctx_destroy(context);
}
