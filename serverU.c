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

struct Suite{
    char code[5];
    int availability;
};

int main (void){
    //printf("Backend serverU booting up.");
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
    fp = fopen("suite.txt", "r");
    while (fgets(s, sizeof s, fp) != NULL){
        //printf("%d: %s", ++linecount, s);
        ++linecount;
        //printf("\n%s\n", scopy);
    }
    fclose(fp);
    char server_message[4*linecount], client_message[4*linecount];
    fp = fopen("suite.txt", "r");
    struct Suite suites[linecount];
    char codes[(linecount)*4+1];
    for(int i=0; i<linecount; i++){
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
        strcpy(suites[i].code, scopy);
        suites[i].availability = s[6]-'0';
    }
    codes[24]="\n";
    //printf("\n%s\n", suites[0].code);
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
    server_addr.sin_port = htons(43278);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
    }

    printf("The Server U is up and running using UDP on port 43278\n");

    main_server_addr.sin_family = AF_INET;
    main_server_addr.sin_port = htons(44278);
    main_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    strcpy(client_message, codes);

    // Send the message to server:
    if(sendto(socket_desc, client_message, strlen(client_message), 0, (struct sockaddr*)&main_server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    printf("\nThe Server U has sent the room status to the main server.\n");

    if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        //printf("Unable to receive message\n");
    }
    //printf("Server messg: %s", server_message);

    while(1){
        // Receive the server's response:
        //printf("server_mess: %s; first char: %c", server_message, server_message[0]);
        
        if((server_message[0] == 'U')){
            printf("The Server U received an availability request from the main server.\n");
            for(int i = 0; i<linecount; i++){
                //printf("Entered for loop\n");
                //printf("Room code in check: %s\n", singleRooms[i].code);

                if(strstr(suites[i].code, server_message)){
                    //printf("Found room; availability: %d\n", singleRooms[i].availability);
                    if(suites[i].availability > 0){
                        printf("Room %s is available.\n", server_message);
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", suites[i].availability);
                    
                    } else {
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", 0);
                        printf("Room %s is not available.\n", server_message);
                    }
                    break;
                } else {
                    strcpy(client_message, "none");
                    if(i == (linecount - 1)){
                        printf("Not able to find room layout.\n");
                    }
                }
            }
            //printf("sending mess: %s\n", client_message);
            if(sendto(socket_desc, client_message, strlen(client_message), 0, (struct sockaddr*)&main_server_addr, main_server_struct_length) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            printf("The Server U finished sending the response to the main server.\n");

            memset(server_message, '\0', sizeof(server_message));
            if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
            //printf("Unable to receive message\n");
            }
            //printf("Server messg: %s", server_message);

        } else if ((server_message[0] == 'R') && (server_message[1] == 'U')) {
            printf("The Server U received a reservation request from main server.\n");
            int changed = 0;
            for(int i = 0; i<linecount; i++){
                //printf("Entered for loop\n");
                //printf("Room code in check: %s; Check stats: %s\n", suites[i].code, strstr(server_message, suites[i].code));
                if(strstr(server_message, suites[i].code)){
                    //printf("Found room; availability: %d\n", suites[i].availability);
                    if(suites[i].availability > 0){
                        changed = 1;
                        int old_avail = suites[i].availability;
                        suites[i].availability = suites[i].availability-1;
                        printf("Successful reservation. The count of %s is now %d.\n", suites[i].code, suites[i].availability);
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", old_avail);
                    
                    } else {
                        memset(client_message, '\0', sizeof(client_message));
                        snprintf(client_message, sizeof(client_message), "%d", 0);
                        printf("Cannot make a reservation. Room %s is not available.\n", suites[i].code);
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
                printf("The Server U finished sending the response and updated room status to the main server.\n");
            } else {
                printf("The Server U finished sending the response to the main server.\n");
            }
            changed = 0;
            

            memset(server_message, '\0', sizeof(server_message));
            if(recvfrom(socket_desc, server_message, sizeof(server_message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
            //printf("Unable to receive message\n");
            }
            //printf("Server messg: %s", server_message);
        }
    }
    //printf("Server's response: %s\n", server_message);
    // Close the socket:
    close(socket_desc);
}