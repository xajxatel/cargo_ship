# Cargo Ship Management System

## Overview
The Cargo Ship Management System simulates the coordination and management of cargo ships traveling between ports. This system is built using C and demonstrates the use of Inter-Process Communication (IPC), multi-threading, and synchronization mechanisms.
![—Pngtree—port cargo ship transport logistics_4459488](https://github.com/xajxatel/cargo_ship/assets/137952206/07cf427d-d82f-4ed1-bb49-441fa0868228)

## Components
The project consists of four main components:
1. `cargo_ship.c` - Simulates a cargo ship process.
2. `port.c` - Simulates a port process handling multiple ships.
3. `shipping_administration.c` - Simulates the central shipping administration.
4. `docking.c` - Manages the termination of the system.
![cargo_ship_output_1](https://github.com/xajxatel/cargo_ship/assets/137952206/5991234b-7f86-43df-aa1c-fd11dd95ce7e)
![step6](https://github.com/xajxatel/cargo_ship/assets/137952206/cb341b71-484d-4f3d-a256-5390168c1ce3)
![step5](https://github.com/xajxatel/cargo_ship/assets/137952206/0cdd9d65-3d74-44d7-ba89-5e2eca8d1df9)
![step4](https://github.com/xajxatel/cargo_ship/assets/137952206/f5db1645-d7dd-4df8-85b0-9cc7c5f1692e)
![step3](https://github.com/xajxatel/cargo_ship/assets/137952206/fe83b723-22d0-4985-9f10-6a3e760d1fc9)
![step2](https://github.com/xajxatel/cargo_ship/assets/137952206/f13af2f4-49cd-4c36-a702-50b39164998c)
![step1](https://github.com/xajxatel/cargo_ship/assets/137952206/07665816-f8c8-408b-b224-0bff7bc98569)
![shipping_administration_output](https://github.com/xajxatel/cargo_ship/assets/137952206/8a228079-0910-492c-aca1-de9df0207715)
![port_output_port2](https://github.com/xajxatel/cargo_ship/assets/137952206/b3e08fae-2da0-4cf4-9ddd-8634e959e4c5)
![port_output_port1](https://github.com/xajxatel/cargo_ship/assets/137952206/644d9402-c22d-4546-bdfe-06e43fef0f89)
![docking_process_output](https://github.com/xajxatel/cargo_ship/assets/137952206/0b3b972f-118e-4432-997c-7da64714633e)
![cargo_ship_output2](https://github.com/xajxatel/cargo_ship/assets/137952206/8b8a415e-19f0-480e-92f7-0eca488645a2)

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
