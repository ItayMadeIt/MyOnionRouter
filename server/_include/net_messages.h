#include <encryptions/encryptions.h>
#include <protocol/server_net_structs.h>
#include <stdbool.h>

bool recv_server_msg(int sock_fd, msg_buffer_t* buffer);
bool recv_enc_server_msg(int sock_fd, msg_buffer_t* buffer, key_data_t* key);

bool send_server_msg(int sock_fd, msg_buffer_t* buffer, void* data, uint64_t length);
bool send_enc_server_msg(int sock_fd, msg_buffer_t* buffer, void* data, uint64_t length, key_data_t* key);

bool recv_handshake_request(int sock_fd, msg_buffer_t* buffer, server_handshake_request_t* request);
bool send_handshake_response(int sock_fd, msg_buffer_t* buffer);

bool recv_handshake_client_key(int sock_fd, msg_buffer_t* buffer, server_handshake_client_key_t* request);
// (confirmation is encrypted)
bool send_handshake_confirmation(int sock_fd, msg_buffer_t* buffer, key_data_t* key, uint32_t id);

bool send_enc_relay_signup_response(int sock_fd, key_data_t* key, msg_buffer_t *buffer, server_responses_t response_type, uint32_t id);
bool send_enc_relay_signout_response(int sock_fd, key_data_t* key, msg_buffer_t *buffer, server_responses_t response_type);
bool send_enc_relay_exit_response(int sock_fd, key_data_t* key, msg_buffer_t *buffer, server_responses_t response_type);

bool send_enc_client_relay_map(int sock_fd, key_data_t *key, msg_buffer_t *buffer, server_relay_list_t* list, server_responses_t response_type);
bool send_enc_client_exit(int sock_fd, key_data_t *key, msg_buffer_t *buffer, server_responses_t response_type);
