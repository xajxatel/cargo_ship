#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_BUFFER_SIZE sizeof(message)

typedef struct ship_info {
    // ship_info structure definition
} ship_info;

typedef struct port_details {
    // port_details structure definition
} port_details;

typedef struct ship_detail {
    ship_info ship;
    port_details port;
} ship_detail;

typedef struct message {
    long mtype;
    ship_detail data;
} message;

int main() {
    message m;
    key_t key = ftok("cargo_ship.c", 'A');
	if (key == -1) {
        printf("error in creating unique key\n");
        exit(1);
    }
    int msg_id = msgget(key, 0666); // Creating message queue if not exists
    if (msg_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    char yes;
    while (1) {
        printf("Do you want the Shipping Administration System to terminate? (Y for Yes and N for No)\n");
        scanf(" %c", &yes); // Adding a space before %c to consume any whitespace characters

        if (yes == 'N') {
            continue;
        } else if (yes == 'Y') {
            m.mtype = 100;
            if (msgsnd(msg_id, &m, sizeof(message), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            break;
        } else {
            printf("Invalid input. Please enter Y or N.\n");
        }
    }

    return 0;
}
