# Lightweight BP Chat Application

This project aims to build a **lightweight messaging application** with the functionality and feasibility of mainstream peer-to-peer (P2P) chat applications. It leverages the `bpsocket` interface to demonstrate communication over the Bundle Protocol (BP). 

**Start Date:** 12/14  
**Current Status:** Ongoing Development  

## Overview

The BP Chat application offers a streamlined approach to testing and discussing the `bpsocket` API for real-world use cases. By building a more interactive and practical chat tool, the application identifies areas where the API can be improved to better support modern P2P messaging features.

## Features

- **P2P Communication**: Exchange messages between peers using `bpsocket`.
- **Interactive Terminal UI**: Easy-to-use text-based interface for sending and receiving messages.
- **Lightweight Design**: Focused on minimal dependencies and efficient performance.
- **Real-Time Messaging**: Supports simultaneous message sending and receiving.

## Current Progress

As of now, the app is functional with:
1. User-friendly prompts for username and destination EID.
2. Real-time messaging with message display in the terminal.
3. Integration with the `bpsocket` API for seamless communication over BP.

Future updates will enhance usability and extend functionality to match modern P2P applications.

## Prerequisites

To run this application, you need:
1. The [bp-sockets repository](https://github.com/juanfraire/bp-sockets) properly installed and configured.
2. The BP Sockets kernel module and daemon running on the system.
3. A C compiler like GCC or Clang with support for `bpsocket`.

## How to Use

1. **Clone the Repository**
   ```bash
   git clone https://github.com/<your-username>/bp-chat-app.git
   cd bp-chat-app
