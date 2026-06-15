## 📖 Overview

This project implements a **reverse shell** communication channel between a **Linux-based C2 server** (operator) and a **Windows target client** (agent). The target initiates the connection back to the server, allowing remote command execution behind firewalls or NAT.

### How It Works

1. **Server** listens on TCP port `4444` (all interfaces)
2. **Client** connects back to the server's IP address
3. Operator sends commands via the server console
4. Client executes them via `cmd.exe` and returns output

---

## 📋 Table of Contents

1. Overview
2. Architecture
3. Features
4. Installation & Compilation
5. Usage
6. Project Structure
7. Learning Objectives
8. License

---

## 📖 Overview

This project demonstrates the fundamentals of TCP/IP networking using C++ sockets.

The repository contains:

* A Linux TCP server
* A Windows TCP client
* Examples of sending and receiving messages
* Basic connection management and error handling

---

## 🏗️ Architecture

```text
┌─────────────────────┐      TCP      ┌─────────────────────┐
│     Linux Server    │ ◄──────────►  │   Windows Client    │
│      (Listener)     │               │    (Connector)      │
└─────────────────────┘               └─────────────────────┘
```

---

## ✨ Features

### ✅ Current Features
| Feature | Description |
|---------|-------------|
| **Reverse TCP Shell** | Target connects back to operator |
| **Remote Command Execution** | Any Windows `cmd.exe` command |
| **Directory Navigation** | `cd` with full path support |
| **File Listing** | `dir` with any path (e.g., `dir C:\`) |
| **File Deletion** | `rm`, `del`, `rmdir` support |
| **View File Contents** | `type` / `cat` to read remote files |
| **Cross-Platform** | Server on Linux, Client on Windows |
| **Error Output** | Captures `stderr` via `2>&1` |
| **Port Reuse** | `SO_REUSEADDR` for clean restarts |

### 🔜 Planned Features
- [ ] AES-256 encryption for C2 traffic
- [ ] Multi-client session management
- [ ] File upload/download
- [ ] Keylogger & clipboard capture
- [ ] Screenshot capture
- [ ] PowerShell execution engine
- [ ] DNS-over-HTTPS tunneling

---

## 🔧 Installation & Compilation

### Prerequisites

#### Linux (Server):
```bash
# Arch
sudo pacman -S gcc

# Debian/Kali/Ubuntu
sudo apt install g++ make

# Fedora
sudo dnf install gcc-c++

### Windows Client

```powershell
g++ ClientOnWindows.cpp -o client.exe -lws2_32
```

---

## 🚀 Usage

### Start the Server

```bash
./ServerOnLinux
```

### Run the Client

```powershell
.\client.exe
```

### Test Communication

Once connected, messages can be exchanged between the client and server.

---

## 📁 Project Structure

```text
.
├── ClientOnWindows.cpp
├── ServerOnLinux.cpp
├── README.md
└── LICENSE
```

---

## 🎯 Learning Objectives

This project helps developers understand:

* TCP/IP fundamentals
* Socket programming
* Client-server architecture
* Cross-platform development
* Network debugging
* Error handling

---

## 🛠 Technologies

* C++
* GCC / G++
* Winsock2
* POSIX Sockets
* Linux
* Windows

---

## 📄 License
MIT License

Copyright (c) 2026 zerodayiq

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

