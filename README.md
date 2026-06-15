# 🚀 Reverse Shell C2 Framework

A lightweight, cross-platform **Command & Control (C2)** framework for authorized penetration testing and security assessments. Built in C++ with raw socket programming.

> ⚠️ **LEGAL DISCLAIMER**  
> This tool is intended **ONLY** for authorized security testing, educational purposes, and CTF challenges. Unauthorized access to computer systems is illegal. The author assumes no liability for misuse.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Features](#features)
4. [Installation & Compilation](#installation--compilation)
5. [Usage Guide](#usage-guide)
6. [Available Commands](#available-commands)
7. [Network Setup (Bridged Mode)](#network-setup-bridged-mode)
8. [Persistence & Evasion](#persistence--evasion)
9. [Project Structure](#project-structure)
10. [Testing Environment](#testing-environment)
11. [Roadmap](#roadmap)
12. [Contributing](#contributing)
13. [License](#license)

---

## 📖 Overview

This project implements a **reverse shell** communication channel between a **Linux-based C2 server** (operator) and a **Windows target client** (agent). The target initiates the connection back to the server, allowing remote command execution behind firewalls or NAT.

### How It Works

1. **Server** listens on TCP port `4444` (all interfaces)
2. **Client** connects back to the server's IP address
3. Operator sends commands via the server console
4. Client executes them via `cmd.exe` and returns output

---

## 🏗️ Architecture
