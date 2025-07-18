#include <client.h>

#include <server_query.h>
#include <stdio.h>

client_vars_t client_vars;

client_code_t run_client()
{
    init_encryption();

    gather_relay_map(&client_vars.relays);

    printf("%d < %d", client_vars.relays.relay_amount, client_vars.config->relays);

    free_encryption();

    return client_success;
}