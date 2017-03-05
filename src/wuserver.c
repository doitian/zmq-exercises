#include <assert.h>
#include <zmq.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_rng.h>

int main(void) {
  void *context = zmq_ctx_new();
  void *publisher = zmq_socket(context, ZMQ_PUB);
  int rc = zmq_bind(publisher, "tcp://*:5556");
  assert(rc == 0);

  gsl_rng *r = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set(r, (unsigned long)time(NULL));

  while (1) {
    //  Get values that will fool the boss
    unsigned long zipcode, temperature, relhumidity;
    zipcode     = gsl_rng_uniform_int(r, 100000);
    temperature = gsl_rng_uniform_int(r, 215) - 80;
    relhumidity = gsl_rng_uniform_int(r, 50) + 10;

    //  Send message to all subscribers
    char update[20];
    sprintf(update, "%05ld %ld %ld", zipcode, temperature, relhumidity);
    zmq_send(publisher, update, strlen(update), 0);
  }

  gsl_rng_free(r);

  zmq_close(publisher);
  zmq_ctx_destroy(context);

  return 0;
}
