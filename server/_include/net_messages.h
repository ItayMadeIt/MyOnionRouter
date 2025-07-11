#include <encryptions/encryptions.h>
#include <protocol/server_net_structs.h>

void recv_handshake_request(int sock_fd, server_handshake_request_t* request);
void send_handshake_response(int sock_fd, server_handshake_response_t* response);

void recv_handshake_client_key(int sock_fd, server_handshake_client_key_t* request);
// (confirmation is encrypted)
void send_handshake_confirmation(int sock_fd, key_data_t* key, server_handshake_confirmation_t* response); 

void recv_enc_buffer(int sock_fd, msg_buffer_t* buffer);
void send_enc_buffer(int sock_fd, msg_buffer_t* buffer);

void send_relay_map(int sock_fd, server_client_request_map_t* map);
