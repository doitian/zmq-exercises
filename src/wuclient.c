#include <assert.h>
#include <zmq.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_rng.h>

int main(int argc, char *argv[]) {
  printf("Collecting updates from weather server…\n");
  void *context = zmq_ctx_new();
  void *subscriber = zmq_socket(context, ZMQ_SUB);
  int rc = zmq_connect(subscriber, "tcp://localhost:5556");
  assert(rc == 0);

  //  Subscribe to zipcode, default is NYC, 10001
  char *filter = (argc > 1) ? argv[1]: "10001 ";
  rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen (filter)); 
  assert(rc == 0);

  int update_nbr;
  long total_temp = 0;
  char buff[256];
  for (update_nbr = 0; update_nbr < 100; update_nbr++) {
    int len = zmq_recv(subscriber, buff, 255, 0);
    if (len > 255) {
      len = 255;
    }
    buff[len] = 0;

    int zipcode, temperature, relhumidity;
    sscanf(buff, "%d %d %d", &zipcode, &temperature, &relhumidity);
    total_temp += temperature;
  }
  printf ("Average temperature for zipcode '%s' was %dF\n",
      filter, (int) (total_temp / update_nbr));

  zmq_close(subscriber);
  zmq_ctx_destroy(context);

  return 0;
}