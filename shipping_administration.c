#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <errno.h>

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

int main(){
	key_t key = ftok("cargo_ship.c", 'A');
	if (key == -1) {
        printf("error in creating unique key\n");
        exit(1);
    }
    int msg_id;
    bool running_state = true;
    int ship_count = 0;
	message m;

    // Create a message queue
    if ((msg_id = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    int no_of_ports;
    printf("Enter the number of ports to be handled/managed:");
    scanf("%d", &no_of_ports);
    int active_ports[no_of_ports + 1];
    for (int i = 0; i <= no_of_ports; i++) {
    	active_ports[i] = 0;
    }
 
    FILE *file;
    char mess[100]; // Buffer to store the message

    // Open the file in append mode
    file = fopen("ShippingAdministration.txt", "a");
    if (file == NULL) {
        fprintf(stderr, "Error opening file.\n");
        exit(EXIT_FAILURE);
    }
    char shipID[20];
    int departurePort, arrivalPort;
    bool check_for_closing = false;
    m.data.ship.ready_for_termination = false;
    
    while (1) {
        // msgrcv to receive message
		if (msgrcv(msg_id, &m, sizeof(message), 100, IPC_NOWAIT) == -1) {
		    if (errno != ENOMSG) {
		        // No message of type i, continue to the next type
		        perror("msgrcv");
		        exit(EXIT_FAILURE);
		    } 
		} else {
		 	running_state = false;
		 	check_for_closing = true;
		}
	    for (long i = 11; i <= 30; i++) {
		    if (msgrcv(msg_id, &m, sizeof(message), i, IPC_NOWAIT) == -1) {
		        if (errno == ENOMSG) {
		            // No message of type i, continue to the next type
		            continue;
		        } else {
		            // Other error occurred
		            perror("msgrcv");
		            exit(EXIT_FAILURE);
		        }
		    } else {
		        // Message received successfully
		        if (m.mtype <= 20) { 
					if (m.data.ship.for_init) {
						ship_count++;
					} else {
					    if (check_for_closing) {
					    	m.data.ship.ready_for_termination = true;
					    	m.mtype = m.data.ship.ship_id + 30;
					    	if (msgsnd(msg_id, &m, sizeof(message), 0) == -1) {
								printf("error in sending message");
								exit(1);
							}
							ship_count--;
					    } else {
							active_ports[m.data.ship.departure_port]++;
							
							if (m.data.ship.for_departure == true) m.mtype = m.data.ship.departure_port;
							else m.mtype = m.data.ship.arrival_port;
								
							if (msgsnd(msg_id, &m, sizeof(message), 0) == -1) {
								printf("error in sending message");
								exit(1);
							}
					    }
					} 
		        } else if (m.mtype <= 30) {
		           	if (m.data.ship.for_departure == true) {   
                     	m.mtype = m.data.ship.arrival_port;
                        m.data.ship.for_departure = false;
                        active_ports[m.data.ship.departure_port]--;
                        active_ports[m.data.ship.arrival_port]++;
                        sprintf(mess, "Ship %ld has departed from Port %ld and will arrive at Port %ld.\n", m.data.ship.ship_id, m.data.ship.departure_port, m.data.ship.arrival_port);
                        fprintf(file, "%s", mess);
                        fflush(file);
                        if (msgsnd(msg_id, &m, sizeof(message), 0) == -1) {
                            printf("error in sending message");
                            exit(1);
                        }
                     } else {
                     	m.mtype = m.data.ship.ship_id + 30;
                        if (msgsnd(msg_id, &m, sizeof(message), 0) == -1) {
		                    printf("error in sending message");
		                    exit(1);
                     	}
                     	active_ports[m.data.ship.arrival_port]--;
                     	ship_count--;
				    }
				}
			}
		}
		if (ship_count == 0 && check_for_closing)
			break;
	}
	
	m.data.ship.ready_for_termination = true;
	for (int i = 1; i <= no_of_ports; i++) {
		m.mtype = i;
		if (msgsnd(msg_id, &m, sizeof(message), 0) == -1) {
			printf("error in sending message");
			exit(1);
        }
	}
	for (int i = 0; i < 30; i++) i++;
	if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
	return 0;
}
