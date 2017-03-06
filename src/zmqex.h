#ifndef _ZMQEX_H
#define _ZMQEX_H

int zmqex_assert(int retcode);
void zmqex_sleep_ms(int ms);

int zmqex_dump(void *socket);

#endif // _ZMQEX_H
