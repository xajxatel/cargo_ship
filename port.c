#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#define ADMINISTRATION_tag 2
#define ship_tag 3
#define MAX_SIZE 100
#define MSG_BUFFER_SIZE sizeof(message)

int num_docks;

long id_variable = -1;
long port_num;

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

// Structure for the FCFS semaphore
typedef struct {
    sem_t sem;
    pthread_mutex_t mutex;
    int waiting;
} FCFS_Semaphore;

FCFS_Semaphore fcfs_sem; // Global FCFS semaphore

// Initialize the FCFS semaphore
void initializeFCFSSemaphore(FCFS_Semaphore* sem) {
    sem_init(&sem->sem, 0, 1); // Initialize semaphore to 1
    pthread_mutex_init(&sem->mutex, NULL); // Initialize mutex
    sem->waiting = 0; // Initialize waiting count
}

// Wait on the FCFS semaphore
void waitFCFS(FCFS_Semaphore* sem) {
    pthread_mutex_lock(&sem->mutex); // Lock the mutex
    sem->waiting++; // Increment the waiting count
    pthread_mutex_unlock(&sem->mutex); // Unlock the mutex
    sem_wait(&sem->sem); // Wait on the semaphore
    pthread_mutex_lock(&sem->mutex); // Lock the mutex
    sem->waiting--; // Decrement the waiting count
    pthread_mutex_unlock(&sem->mutex); // Unlock the mutex
}

// Signal the FCFS semaphore
void signalFCFS(FCFS_Semaphore* sem) {
    sem_post(&sem->sem); // Signal the semaphore
}
void destroyFCFSSemaphore(FCFS_Semaphore* sem) {
    sem_destroy(&sem->sem); // Destroy the semaphore
    pthread_mutex_destroy(&sem->mutex); // Destroy the mutex
}
void *ship_runner(void *arg) {
    ship_detail *detail = (ship_detail *)arg;
    key_t key = ftok("cargo_ship.c", 'A');
	if (key == -1) {
        printf("error in creating unique key\n");
        exit(1);
    }
    int msg_id = msgget(key, 0666);
    if (msg_id == -1) {
        perror("msgget");
        exit(1);
    }

    int best_fit = -1;
    int best_fit_cap = 15000;
    message m;
    m.data.ship = detail->ship;
    
    m.mtype = 20 + detail->port.port_num;
    
    for (int i = 0; i < detail->port.num_docks; i++) {
        if (detail->ship.total_weight < detail->port.load_cap[i] && best_fit_cap > detail->port.load_cap[i]) {
            best_fit = i;
            best_fit_cap = detail->port.load_cap[i];
        }
    }
   
    if (best_fit == -1) best_fit = detail->port.num_docks;
    
    if (m.data.ship.for_departure == false) sleep(30);
    
    waitFCFS(&fcfs_sem); // Wait on the FCFS semaphore
    
    sleep(5);

    if (m.data.ship.for_departure) {
        printf("Ship %ld has completed loading and departed from Dock No. %d of Port No. %ld. \n", m.data.ship.ship_id, best_fit + 1, m.data.ship.departure_port);
    } else {
        printf("Ship %ld has arrived at Dock No. %d of Port No. %ld and has completed unloading.\n", m.data.ship.ship_id, best_fit + 1, m.data.ship.arrival_port);
    }
    if (m.data.ship.for_departure) m.mtype = m.data.ship.departure_port + 20;
    else m.mtype = m.data.ship.arrival_port + 20;
    
    if (msgsnd(msg_id, (void *)&m, sizeof(message), 0) == -1) {
        printf("error in sending message");
        exit(1);
    }

    signalFCFS(&fcfs_sem); // Signal the FCFS semaphore

    pthread_exit(NULL);
}

int main() {
    
    printf("Enter Port Number:");
    scanf("%ld", &port_num);
    printf("Enter number of Docks:");
    scanf("%d", &num_docks);

    int load_cap[num_docks + 1];

    printf("Enter loadCapacity of Docks (give as a space separated list in a single line):");
    for (int i = 0; i < num_docks; i++)
        scanf("%d", &load_cap[i]);

    load_cap[num_docks] = 15000;

    initializeFCFSSemaphore(&fcfs_sem); // Initialize the FCFS semaphore

    key_t key = ftok("cargo_ship.c", 'A');
	if (key == -1) {
        printf("error in creating unique key\n");
        exit(1);
    }

	pthread_t ship_threads[MAX_SIZE];
    int thread_count = 0;
    int msg_id = msgget(key, 0666);
    if (msg_id == -1) {
        perror("msgget");
        exit(1);
    }

    message msg;

    while (true) {
        if (msgrcv(msg_id, &msg, sizeof(message), port_num, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        if (msg.data.ship.ready_for_termination == true) break;

        ship_detail detail;
        detail.ship = msg.data.ship;
        detail.port.port_num = port_num;
        detail.port.num_docks = num_docks;
        detail.port.load_cap = load_cap;

        pthread_t ship_id;
        int status = pthread_create(&ship_id, NULL, ship_runner, &detail);
        ship_threads[thread_count++] = ship_id;
        if (status != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    
    destroyFCFSSemaphore(&fcfs_sem);
    return 0;
}
