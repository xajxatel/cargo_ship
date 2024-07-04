#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>

#define MAX_CONTAINERS 100
#define MAX_CARGO_WEIGHT 1000
#define MSG_KEY 12345 // Unique key for message queue

typedef struct ship_info {
    long ship_id;
    int is_container_ship;
    int num_containers; 
    int num_bulk_items; 
    int total_weight; 
    long departure_port; 
    long arrival_port;
    bool for_departure; 
    bool ready_for_termination;
    bool for_init;
} ship_info;

typedef struct port_details {
    long port_num;
    int num_docks;
    int *load_cap;
} port_details;

typedef struct ship_detail {
    ship_info ship;
    port_details port;
} ship_detail;

typedef struct message {
    long mtype;
    ship_detail data;
} message;

sem_t mutex; // Semaphore for synchronization

void createCargoProcesses(int num_containers, int pipefd[][2]);

int main() {
    ship_info ship;
    ship.total_weight = 0;
    ship.for_departure = true;
    
    sem_init(&mutex, 0, 1); // Initialize semaphore

    key_t key = ftok("cargo_ship.c", 'A');
    if (key == -1) {
        printf("error in creating unique key\n");
        exit(1);
    }
    
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
    
    message init;
    init.mtype = 13;
    init.data.ship.for_init = true;
    
    if (msgsnd(msgid, &init, sizeof(message), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    
    // Ask user to enter unique ship ID
    printf("Enter Ship ID: ");
    scanf("%ld", &ship.ship_id);
    // Ask user to enter type of ship (container or bulk)
    printf("Enter Type of Ship (1 for Container, 0 for Bulk): ");
    scanf("%d", &ship.is_container_ship);

    // Print ship ID and type
    if (ship.is_container_ship) {
        printf("Enter Number of Containers (1-100): ");
        scanf("%d", &ship.num_containers);
        
        if (ship.num_containers < 1 || ship.num_containers > 100) {
            fprintf(stderr, "Invalid number of containers. Must be between 1 and 100.\n");
            exit(EXIT_FAILURE);
        }
        
        int pipefd[MAX_CONTAINERS][2]; // Array to store pipe file descriptors

        // Create pipes for each container
        for (int i = 0; i < ship.num_containers; i++) {
            if (pipe(pipefd[i]) == -1) {
                perror("Pipe creation failed");
                exit(EXIT_FAILURE);
            }
        }
		
        // Create cargo processes
        createCargoProcesses(ship.num_containers, pipefd);

        // Close pipe file descriptors in parent process
        for (int i = 0; i < ship.num_containers; i++) {
            close(pipefd[i][1]); // Close write end
        }

        // Calculate total weight of cargo
        int cargo_weight = 0;

        // Calculate total weight of cargo
        for (int i = 0; i < ship.num_containers; i++) {
            int weight; // Variable to store weight
            read(pipefd[i][0], &weight, sizeof(weight));
            cargo_weight += weight;
        }

        // Calculate total weight including crew members
        ship.total_weight = cargo_weight + (NUM_CREW_MEMBERS * AVERAGE_CREW_WEIGHT);
        for (int i = 0; i < ship.num_containers; i++) {
            close(pipefd[i][0]); // Close read end
        }
        
    } else {
        printf("Enter Number of Bulk Items: ");
        scanf("%d", &ship.num_bulk_items);
        
        if (ship.num_bulk_items < 1 || ship.num_bulk_items > 1000) {
            fprintf(stderr, "Invalid number of bulk items. Must be between 1 and 1000.\n");
            exit(EXIT_FAILURE);
        }
        
        printf("Enter Average Weight of Bulk Items: ");
        scanf("%d", &ship.total_weight);
        if (ship.total_weight < 1 || ship.total_weight > 1000) {
            fprintf(stderr, "Invalid average weight of bulk items. Must be between 1 and 1000.\n");
            exit(EXIT_FAILURE);
        }
        
        ship.total_weight = (ship.num_bulk_items * ship.total_weight) + (2 * AVERAGE_CREW_WEIGHT);
    }

    // Ask user to enter port number for departure
    printf("Enter Port Number for Departure (1-10): ");
    scanf("%ld", &ship.departure_port);
    if (ship.departure_port < 1 || ship.departure_port > 10) {
        fprintf(stderr, "Invalid port number for departure. Must be between 1 and 10.\n");
        exit(EXIT_FAILURE);
    }

    // Ask user to enter port number for arrival
    printf("Enter Port Number for Arrival (1-10): ");
    scanf("%ld", &ship.arrival_port);
    if (ship.arrival_port < 1 || ship.arrival_port > 10 || ship.arrival_port == ship.departure_port) {
        fprintf(stderr, "Invalid port number for arrival. Must be between 1 and 10 and not equal to departure port.\n");
        exit(EXIT_FAILURE);
    }

    // Send message containing ship details to shipping administration
    message m;
    m.mtype = 10 + ship.ship_id; // Message type
    m.data.ship = ship; // Ship details
    m.data.ship.for_departure = true;
    m.data.ship.ready_for_termination = false;
    m.data.ship.for_init = false;
	
    if (msgsnd(msgid, &m, sizeof(message), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    // Receive message from administration indicating completion of voyage or shutdown of administration
    if (msgrcv(msgid, (void *)&m, sizeof(message), ship.ship_id + 30, 0) == -1) {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }
    if (m.data.ship.ready_for_termination) {
    	printf("Shipping Administration is Shut Down\n");
        if (sem_destroy(&mutex) == -1) {
            perror("Semaphore destruction failed");
            exit(EXIT_FAILURE);
        }
    	return 0;
    }
    
    // Display message indicating successful voyage
    printf("Ship %ld has successfully traveled from Port %ld to Port %ld!\n",
           m.data.ship.ship_id, m.data.ship.departure_port, m.data.ship.arrival_port);
    if (sem_destroy(&mutex) == -1) {
        perror("Semaphore destruction failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Function to create cargo processes
void createCargoProcesses(int num_containers, int pipefd[][2]) {
    pid_t pid;
    int cargo_weight;
    
    for (int i = 0; i < num_containers; i++) {
        sem_wait(&mutex); // Wait for access
        
        pid = fork();
        if (pid < 0) {
            // Error occurred
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            close(pipefd[i][0]);
            // Prompt user to enter cargo weight
            printf("Enter Weight of Cargo Item: ");
            scanf("%d", &cargo_weight);
            if (cargo_weight < 0 || cargo_weight > MAX_CARGO_WEIGHT) {
                fprintf(stderr, "Invalid cargo weight. Must be between 0 and %d.\n", MAX_CARGO_WEIGHT);
                exit(EXIT_FAILURE);
            }
            // Write cargo weight to pipe
            write(pipefd[i][1], &cargo_weight, sizeof(cargo_weight));
            close(pipefd[i][1]);
            exit(EXIT_SUCCESS);
        }
        
        sem_post(&mutex); // Release access
        wait(NULL);
    }
}
