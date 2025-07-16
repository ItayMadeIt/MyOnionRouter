#include <encryptions/encryptions.h>
#include <protocol/server_net_structs.h>
#include <stdbool.h>

bool recv_server_msg(int sock_fd, msg_server_buffer_t* buffer);
bool recv_enc_server_msg(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key);

bool send_server_msg(int sock_fd, msg_server_buffer_t* buffer, void* data, uint64_t length);
bool send_enc_server_msg(int sock_fd, msg_server_buffer_t* buffer, void* data, uint64_t length, key_data_t* key);

bool send_server_handshake_req(int sock_fd, msg_server_buffer_t* buffer);
bool recv_server_handshake_res(int sock_fd, msg_server_buffer_t* buffer, server_handshake_response_t* response);

bool send_server_handshake_key(int sock_fd, msg_server_buffer_t* buffer, uint8_t key[ASYMMETRIC_KEY_BYTES]);
bool recv_server_handshake_confirmation(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, server_handshake_confirmation_t* confirmation);