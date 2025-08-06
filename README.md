# 🧪 OS Project 2 — Molecule Warehouse Server

A multi-protocol server written in C for managing atomic inventory and generating molecules based on client requests via TCP, UDP, and UDS.

## 🚀 Features

### Stage 1: Atom Warehouse (TCP)
- Listens for TCP connections from `atom_supplier` clients
- Accepts commands like:
  - `ADD CARBON n`
  - `ADD OXYGEN n`
  - `ADD HYDROGEN n`
- Tracks atom counts and prints the updated inventory after each command

### Stage 2: Molecule Supplier (UDP)
- Accepts UDP packets from clients requesting molecule deliveries:
  - `DELIVER WATER n`
  - `DELIVER CARBON DIOXIDE n`
  - `DELIVER ALCOHOL n`
  - `DELIVER GLUCOSE n`
- Verifies if enough atoms exist to assemble requested molecules
- Replies with success or error

### Stage 3: Drinks Bar Console
- Adds CLI control via keyboard input
- Commands like:
  - `GEN SOFT DRINK`
  - `GEN VODKA`
  - `GEN CHAMPAGNE`
- Uses recipes to calculate how many drinks can be generated

### Stage 4: Startup Options
- Supports atom initialization via CLI flags:
  - `--carbon`, `--oxygen`, `--hydrogen`
- Adds `--timeout`, `--tcp-port`, `--udp-port`

### Stage 5: UDS Support
- Adds Unix Domain Socket (UDS) support for:
  - Stream-based
  - Datagram-based communication

### Stage 6: Shared Memory & File Mapping
- Supports multiple concurrent `drinks_bar` processes
- Uses `mmap` to store and persist inventory across runs
- Inventory file path provided via `--save-file`

## 📡 Protocols Supported
- TCP
- UDP
- UDS (stream and datagram)
- CLI / stdin input

## 🛠️ Tools & Concepts
- `select()`, `getaddrinfo()`, `alarm()`, `getopt()`
- POSIX shared memory (`mmap`)
- File locking
- Multi-client architecture

## 🧪 Testing
- Tested with multiple clients running in parallel
- Validated proper molecule assembly and inventory consistency

## 📁 Structure
- `atom_supplier.c`
- `molecule_supplier.c`
- `molecule_requester.c`
- `drinks_bar.c`
- `warehouse_server.c`
- `Makefile`, `README.md`

## 📌 Notes
- Follows all assignment requirements from OS Course — Semester B 2025
- Code is modular, memory-checked, and supports multiple interaction protocols

## 👥 Contributors
Adir Ofir
