#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int socket_desc;

void sigint_handler(int signum) {
    // Close UDP socket
    if (close(socket_desc) == -1) {
        perror("Error closing UDP socket");
    }

    // Exit the program
    exit(EXIT_SUCCESS);
}

struct SingleRoom{
    char code[5];
    int availability;
};

int main (void){
    //printf("Backend serverS booting up.\n");
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr, main_server_addr;
    int server_struct_length = sizeof(server_addr);
    int main_server_struct_length = sizeof(main_server_addr);

    FILE *fp;
    char s[1024];
    char scopy[5];
    int linecount = 0;
    fp = fopen("single.txt", "r");
    while (fgets(s, sizeof s, fp) != NULL){
        //printf("%d: %s", ++linecount, s);
        ++linecount;
    }
    //printf("\n\n%d: total lines\n", linecount);
    fclose(fp);
    char server_message[2000], client_message[2000];
    fp = fopen("single.txt", "r");
    struct SingleRoom singleRooms[linecount];
    char codes[(linecount)*4+1];
   // printf("\nLen: %d\n", ());
    for(int i=0; i<(linecount); i++){
        fgets(s, sizeof s, fp);
        scopy[0] = s[0];
        scopy[1] = s[1];
        scopy[2] = s[2];
        scopy[3] = s[3];
        scopy[4] = '\0';
        codes[i*4] = s[0];
        codes[i*4+1] = s[1];
        codes[i*4+2] = s[2];
        codes[i*4+3] = s[3];

        strcpy(singleRooms[i].code, scopy);
        singleRooms[i].availability = s[6] - '0';
        //printf("code: %c; avail: %d\n", s[6], singleRooms[i].availability);
        //printf("Found room; availability: %c\n", singleRooms[i].code[4]);
        
        //printf("Availability: %d", singleRooms[i].availability);
    }
    //printf("Len: %d", strlen(codes));
    codes[24]='\0';
    //printf("\n%s\n", singleRooms[0].code);
    //printf("\n%s\n", codes);
    //printf("\nLast char: %s\n", codes[0]);
    fclose(fp);

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    //printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(41278);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
    }

    printf("The Server S is up and running using UDP on port 41278\n");

    main_server_addr.sin_family = AF_INET;
    main_server_addr.sin_port = htons(44278);
    main_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


    strcpy(client_message, codes);
     // Send the message to server:
    if(sendto(socket_desc, client_message, strlen(client_message), 0,
         (struct sockaddr*)&main_server_addr, main_server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    printf("\nThe Server S has sent the room status to the main server.\n");

    if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        //printf("Unable to receive message\n");
    }
    //printf("Server messg: %s", server_message);
    
    while(1){
        // Receive the server's response:
    //     if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    //         printf("Unable to receive message\n");
    //     }
    // printf("Server messg: %s", server_message);
    
    //     ssize_t bytes_received = recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    //     int byt = bytes_received;
    //     if (bytes_received < 0) {
    //         perror("Error while receiving server's msg");
    //         continue;
    //     // Handle error, possibly close the connection or return -1
    //         //break; // Exit the loop on error
    //     } else if (bytes_received == 0) {
    //     // No data received, continue waiting
    //         continue;
    //     }
        // while(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
        //     (struct sockaddr*)&server_addr, &main_server_struct_length) < 0){
        //     printf("Error while receiving server's msg\n");
        //     //return -1;
        // } else {
            //server_message[sizeof(server_message) - 1] = '\0';
            //printf("byt received: %d", byt);
            //printf("server_mess: %s; first char: %c", server_message, server_message[0]);
        
        if((server_message[0] == 'S')){
            printf("The Server S received an availability request from the main server.\n");
            for(int i = 0; i<linecount; i++){
                //printf("Entered for loop\n");
                //printf("Room code in check: %s\n", singleRooms[i].code);

                if(strstr(singleRooms[i].code, server_message)){
                    //printf("Found room; availability: %d\n", singleRooms[i].availability);
                    if(singleRooms[i].availability > 0){
                        printf("Room %s is available.\n", server_message);
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", singleRooms[i].availability);
                    
                    } else {
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", 0);
                        printf("Room %s is not available.\n", server_message);
                    }
                    break;
                } else {
                    strcpy(client_message, "none");
                    printf("Not able to find room layout.\n");
                }
            }
            //printf("sending mess: %s\n", client_message);
            if(sendto(socket_desc, client_message, strlen(client_message), 0, (struct sockaddr*)&main_server_addr, main_server_struct_length) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            printf("The Server S finished sending the response to the main server.\n");

            memset(server_message, '\0', sizeof(server_message));
            if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
            //printf("Unable to receive message\n");
            }
            //printf("Server messg: %s", server_message);

        } else if ((server_message[0] == 'R') && (server_message[1] == 'S')){
            printf("The Server S received a reservation request from main server.\n");
            int changed = 0;
            for(int i = 0; i<linecount; i++){
                //printf("Entered for loop\n");
                //printf("Room code in check: %s; Check stats: %s\n", singleRooms[i].code, strstr(server_message, singleRooms[i].code));
                if(strstr(server_message, singleRooms[i].code)){
                    //printf("Found room; availability: %d\n", singleRooms[i].availability);
                    if(singleRooms[i].availability > 0){
                        changed = 1;
                        int old_avail = singleRooms[i].availability;
                        singleRooms[i].availability = singleRooms[i].availability-1;
                        printf("Successful reservation. The count of %s is now %d.\n", singleRooms[i].code, singleRooms[i].availability);
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", old_avail);
                    
                    } else {
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", 0);
                        printf("Cannot make a reservation. Room %s is not available.\n", singleRooms[i].code);
                    }
                    break;
                } else {
                    strcpy(client_message, "none");
                    if(i == (linecount-1)){
                        printf("Cannot make a reservation. Not able to find room layout.\n");
                    }
                    
                }
            }
            //printf("sending mess: %s\n", client_message);
            if(sendto(socket_desc, client_message, strlen(client_message), 0, (struct sockaddr*)&main_server_addr, main_server_struct_length) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            if(changed == 1){
                printf("The Server S finished sending the response and updated room status to the main server.\n");
            } else {
                printf("The Server S finished sending the response to the main server.\n");
            }
            changed = 0;
            

            memset(server_message, '\0', sizeof(server_message));
            if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
            //printf("Unable to receive message\n");
            }
            //printf("Server messg: %s", server_message);
        }
        //printf("Server's response: %s\n", server_message);
    }
    //printf("Server's response: %s\n", server_message);

    // Close the socket:
    close(socket_desc);

};
