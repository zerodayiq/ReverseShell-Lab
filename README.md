# 🌐 C++ TCP Socket Programming Lab

A cross-platform networking project built in C++ to demonstrate TCP client-server communication between Linux and Windows systems.

> 📚 Educational project focused on learning socket programming, network communication, and client-server architecture.

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
│     Linux Server    │ ◄──────────► │   Windows Client   │
│    (Listener)       │              │    (Connector)     │
└─────────────────────┘              └─────────────────────┘
```

---

## ✨ Features

* TCP communication
* IPv4 networking
* Cross-platform support
* Socket creation and management
* Error handling
* Message exchange
* Educational code structure

---

## 🔧 Installation & Compilation

### Linux Server

```bash
g++ ServerOnLinux.cpp -o ServerOnLinux
./ServerOnLinux
```

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

---

## 👨‍💻 Author

Mustafa

* C++
* Linux
* Cybersecurity
* Networking
