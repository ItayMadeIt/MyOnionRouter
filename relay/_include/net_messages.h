#ifndef __NET_MESSAGES_H__
#define __NET_MESSAGES_H__

#include <encryptions/encryptions.h>
#include <protocol/server_net_structs.h>
#include <stdbool.h>
#include <protocol/tor_structs.h>

bool recv_tls_msg(int sock_fd, tls_key_buffer_t* data);
bool send_tls_msg(int sock_fd, tls_key_buffer_t* data);

bool recv_tor_buffer(int sock_fd, msg_tor_buffer_t* data, key_data_t* tls_key, key_data_t* onion_key, bool from_client);
bool send_tor_buffer(int sock_fd, msg_tor_buffer_t* data, key_data_t* tls_key, key_data_t* onion_key, bool to_client);

bool recv_server_msg(int sock_fd, msg_server_buffer_t* buffer);
bool recv_enc_server_msg(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key);

bool send_server_msg(int sock_fd, msg_server_buffer_t* buffer, void* data, uint64_t length);
bool send_enc_server_msg(int sock_fd, msg_server_buffer_t* buffer, void* data, uint64_t length, key_data_t* key);

bool send_server_handshake_req(int sock_fd, msg_server_buffer_t* buffer);
bool recv_server_handshake_res(int sock_fd, msg_server_buffer_t* buffer, server_handshake_response_t* response);

bool send_server_handshake_key(int sock_fd, msg_server_buffer_t* buffer, uint8_t key[ASYMMETRIC_KEY_BYTES]);
bool recv_server_handshake_confirmation(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, server_handshake_confirmation_t* confirmation);

bool send_server_signup_req(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, sock_addr_t* sock_addr);
bool recv_server_signup_res(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, uint32_t* id);

bool send_server_signout_req(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, uint32_t id);
bool recv_server_signout_res(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key);

bool send_server_exit_req(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key);
bool recv_server_exit_res(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key);

#endif // __NET_MESSAGES_H__