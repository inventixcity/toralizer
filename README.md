# Toralizer

## Overview

**Toralizer** is a lightweight, low-level network utility written in C designed to proxy TCP connections through a SOCKS server (specifically targeting the Tor network). 

It features a dual-architecture design:
*   **Linux**: Operates as a **transparent proxy injector** (using `LD_PRELOAD`). It intercepts socket calls from arbitrary applications, forcing them to tunnel through the configured proxy without modifying the application's source code.
*   **Windows**: Operates as a **standalone diagnostic client** to verify SOCKS4 connectivity, handshake integrity, and upstream data transfer.

## Features

*   **SOCKS4 Protocol Integration**: Native implementation of the SOCKS4 handshake protocol.
*   **Cross-Platform Support**: Unified codebase supporting both Linux (GCC) and Windows (MinGW).
*   **Transparent Interception (Linux)**: Uses dynamic library preloading to wrap the `connect()` system call.
*   **Hardened Compilation**: Built with modern security flags (`-fstack-protector-strong`, `_FORTIFY_SOURCE=2`, `-fPIC`) to resist common exploit techniques.

## Prerequisites

*   **Tor Proxy**: A running instance of the Tor service (Tor Expert Bundle or Tor Browser).
    *   Default address: `127.0.0.1`
    *   Default port: `9050`
*   **Build Tools**:
    *   Linux: `gcc`, `make`
    *   Windows: `MinGW-w64` (gcc), `make`

## Installation & Compilation

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/your-username/toralizer.git
    cd toralizer
    ```

2.  **Compile**:
    The project includes a smart `Makefile` that automatically detects the operating system.

    *   **On Linux**:
        ```bash
        make
        ```
        *Output*: `toralize.so` (Shared Object Library)

    *   **On Windows**:
        ```powershell
        make
        ```
        *Output*: `toralize.exe` (Executable Binary)

## Usage

### Linux (Transparent Wrapper)

To "toralize" an application (force it to use the proxy), set the `LD_PRELOAD` environment variable to the path of the compiled shared object.

```bash
# Syntax
export LD_PRELOAD=/path/to/toralize.so
[command] [arguments]

# Example: Tunnelling curl
export LD_PRELOAD=$PWD/toralize.so
curl http://check.torproject.org
```

**Note**: This technique intercepts standard TCP `connect()` calls. It does not automatically intercept DNS resolution functions (`getaddrinfo`, etc.), so DNS requests may still be resolved locally by the OS (DNS Leak).

### Windows (Diagnostic Client)

On Windows, Toralizer serves as a connectivity tester. It establishes a connection to the proxy, requests a tunnel to a target, and performs a simple HTTP HEAD request.

```powershell
# Syntax
.\toralize.exe <target_ip> <target_port>

# Example: Connect to example.com via Tor
.\toralize.exe 93.184.216.34 80
```

**Output**:
*   `Connected to proxy`: Successful local link to Tor.
*   `Proxy connection established`: Successful SOCKS4 handshake with the Tor network.
*   `Received data`: Confirmation of data flow from the remote target.

## Configuration

To change the default proxy address or port, edit `toralize.h` before compiling:

```c
#define PROXY "127.0.0.1"  // Proxy IP
#define PROXYPORT 9050     // Proxy Port (9050 for Tor Expert, 9150 for Tor Browser)
```

## Security & Limitations

*   **Protocol**: Currently supports SOCKS4. SOCKS4a (remote DNS resolution) or SOCKS5 are required for complete protection against DNS leaks when using domain names.
*   **Encryption**: The connection between your machine and the Tor entry node is encrypted by Tor. However, Toralizer itself does not add encryption to the traffic inside the tunnel (unless the application uses SSL/TLS).
*   **Education Object**: This project is intended for educational purposes in network programming and security research. It is not a replacement for full-featured privacy tools like `torsocks`.

## License

[MIT License](LICENSE)
