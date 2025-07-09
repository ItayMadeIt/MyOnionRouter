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

This project is a low-level, lightweight version of TOR, aiming to create a simplified yet strong enough model to demonstrate how onion routing works, while keeping the codebase readable, modular, and educational.

## Libraries
This project uses minimal external libraries. The libaries:
- `libc` – Standard C library, included on all Linux systems.
- [`GMP`](https://gmplib.org/) – A well-known library for big integer arithmetic.  
  Licensed under the GNU GPL or LGPL. See `licenses/` for full terms.