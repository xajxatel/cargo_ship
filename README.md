# Cargo Ship Management System

## Overview
The Cargo Ship Management System simulates the coordination and management of cargo ships traveling between ports. This system is built using C and demonstrates the use of Inter-Process Communication (IPC), multi-threading, and synchronization mechanisms.

## Components
The project consists of four main components:
1. `cargo_ship.c` - Simulates a cargo ship process.
2. `port.c` - Simulates a port process handling multiple ships.
3. `shipping_administration.c` - Simulates the central shipping administration.
4. `docking.c` - Manages the termination of the system.

## Files

### cargo_ship.c
This file simulates a cargo ship process. Each instance of the program represents a different cargo ship (container ship or bulk carrier).

#### Key Features:
- Reads input for ship ID, type (container or bulk), number of containers or cargo items, weights, departure, and arrival ports.
- Creates child processes for cargo handling and uses pipes for IPC between the ship and cargo processes.
- Calculates total weight and communicates with Shipping Administration using message queues.
- Handles termination and synchronization using semaphores.

### port.c
This file simulates a port process, capable of handling multiple cargo ships arriving or departing.

#### Key Features:
- Reads input for port ID, number of docks, and their capacities.
- Uses threads to handle multiple ship arrivals and departures simultaneously.
- Implements FCFS semaphore for synchronization.
- Selects the best-fit dock based on ship weight and handles loading and unloading processes.
- Communicates with Shipping Administration using message queues.

### shipping_administration.c
This file simulates the Shipping Administration process, which coordinates the movements of cargo ships between ports.

#### Key Features:
- Handles multiple ports and coordinates ship departures and arrivals.
- Uses a single message queue for all communications.
- Logs ship movements to `ShippingAdministration.txt`.
- Handles system termination requests from the Docking process and ensures all ships have been docked before shutting down.

### docking.c
This file simulates the Docking process, responsible for initiating the termination of the Shipping Administration system.

#### Key Features:
- Continuously prompts the user for termination of the system.
- Sends termination message to Shipping Administration when the user decides to terminate.
- Ensures that the Shipping Administration process only shuts down after all ships have been docked.

## Usage

### Compile the Programs
```sh
gcc cargo_ship.c -o cargo_ship -lpthread -lrt
gcc port.c -o port -lpthread -lrt
gcc shipping_administration.c -o shipping_administration -lpthread -lrt
gcc docking.c -o docking -lpthread -lrt
