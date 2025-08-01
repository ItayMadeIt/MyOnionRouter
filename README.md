# MyOnionRouter

## Summary
This is "My Onion Router" - MOR. A simplified version of [TOR](https://www.torproject.org/) - The Onion Router.
It demonstrates how layered encryption and relay-based routing can anonymize internet traffic.

> MOR is intended strictly for educational purposes. It is *not secure enough* for production use.

The main features are:
- **Full Crypto Library** - All core cryptographic features (AES, Diffie-Hellman, SHA-256) are implemented specifically for this project.
- **Directory Server** - Tracks and shares available relay nodes
- **N Relays** - Traffic is routed through multiple hops, each decrypting one encryption layer
- **Raw Client Connection** - Sends raw data through the onion tunnel

This project is a low-level, lightweight version of TOR, aiming to create a simplified yet strong enough model to demonstrate how onion routing works, while maintaining a readable, modular, and educational codebase.

## Libraries
This project uses minimal external libraries. The libraries:
- `libc` - Standard C library, included on all Linux systems.
- [`GMP`](https://gmplib.org/) - A well-known library for big integer arithmetic.  
  Licensed under the GNU GPL or LGPL. See `licenses/` for full terms.

## Interface
Each file created can be interacted using a CLI.
The `server` and `relay` only support the `exit` command which terminates their application cleanly, while the client has more options.
The `client` has the commands:
- `connect` - Connects to a server and generates a `stream ID` (Identifier for that connection).
- `close` - Closes a connection using a `stream ID`.
- `ping` - Pings by sending a packet that traverses the circuit.
- `destroy` - Destroys the circuit and exists from all relays.
- `send` - Sends to a connection (`stream ID`) a specific number of bytes using char input.
- `recv` - Receives data from a connection (`stream ID`) a specific number of bytes.

## Dependencies
- `gcc` or `clang`
- `make`
- `libgmp-dev` (or installed GMP use `install_deps.sh`)
- Linux OS (tested on Ubuntu/Debian)

To install the added dependencies:
```bash
./install_deps.sh
```

## Build
To build the project:
```bash
make all
```
This will generate a `build/` directory that mirrors the source structure. It contains compiled object files and final binaries:
```bash
build/
├── client/
│   ├── client            # The compiled client binary
│   └── core/, net/, ...  # Object files for the client
├── relay/
│   ├── relay             # The compiled relay binary
│   └── core/, net/, ...  # Object files for the relay
├── server/
│   ├── server            # The compiled server binary
│   └── core/, net/, ...  # Object files for the server
├── encryptions/          # Crypto library object files
│   ├── aes/
│   ├── dh/
│   ├── sha/
│   └── source/
└── utils/                # Utility libraries object files
    ├── sock_utils/
    ├── server_cfg/
    ├── file_utils/
    └── string_utils/
```
You can then run each component (in separate terminals):
```bash
./build/server/server <server.cfg> 
./build/relay/relay <config_file> [-p <port> | --port <port>]
./build/client/client <config_file> [-r <relays> | --relays <relays>] 
```