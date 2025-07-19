#ifndef __SERVER_REGISTER_H__
#define __SERVER_REGISTER_H__

#include <relay.h>
#include <protocol/relay_data_structs.h>
#include <sys/socket.h>

relay_code_t signup_server();
relay_code_t signout_server();


#endif // __SERVER_REGISTER_H__