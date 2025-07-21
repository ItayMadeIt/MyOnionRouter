#include <encryptions/encryptions.h>

bool handle_tls_sender   (int sock_fd, key_data_t* this);
bool handle_tls_recviever(int sock_fd, key_data_t* this);