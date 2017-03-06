#include <assert.h>
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

int main(void) {
  void *context = zmq_ctx_new();
  void *publisher = zmq_socket(context, ZMQ_PUB);
  int rc = zmq_bind(publisher, "tcp://*:5556");
  assert(rc == 0);

  GRand *r = g_rand_new_with_seed((guint32)g_get_real_time());

  while (1) {
    //  Get values that will fool the boss
    int32_t zipcode, temperature, relhumidity;
    zipcode     = g_rand_int_range(r, 0, 100000);
    temperature = g_rand_int_range(r, -80, 135);
    relhumidity = g_rand_int_range(r, 10, 60);

    //  Send message to all subscribers
    char update[20];
    sprintf(update, "%05d %d %d", zipcode, temperature, relhumidity);
    zmq_send(publisher, update, strlen(update), 0);
  }

  g_rand_free(r);

  return 0;
}
