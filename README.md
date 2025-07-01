# MyOnionRouter

## Summary
This is "My Onion Router" - MOR. A simplified version of TOR, (The Onion Router).
It demonstrates how layered encryption and relay-based routing can anonymize internet traffic.
>It is made primarly for educational purposes, not as a secure enough or a production ready tool.

The main features are:
- Directory Server - Keeps track of available relays
- N relays - Traffic is routed through multiple hops
- Client raw connection - Sends raw data through the onion tunnel

This project is a low-level lightweight version of TOR.

## Libraries
This project uses minimal external libraries. The only one currently included is:
- [`mini-gmp`](https://gmplib.org/) – a minimalistic subset of GMP for big integer arithmetic.  
  Licensed under the GNU GPL or LGPL. See `licenses/` for full terms.
