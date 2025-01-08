#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int socket_desc; //socket descriptor for UDP to communicate with backend servers
int socket_desc_tcp, client_sock_tcp, client_size_tcp; //socket descriptor for TCP to communicate with client

//Member struct to store member info
struct Member{
    char uName[50];
    char pass[50];
};

//signal handler to catch ctrl+c and close sockets before exiting program
void sigint_handler(int signum) {
    // Close UDP socket
    if (close(client_sock_tcp) == -1) {
        //perror("Error closing TCP socket");
    }

    if (close(socket_desc_tcp) == -1) {
        //perror("Error closing TCP socket");
    }

    // Close TCP socket
    if (close(socket_desc) == -1) {
        //perror("Error closing UDP socket");
    }

    // Exit the program
    exit(EXIT_SUCCESS);
}


//main
int main (void){
    //instantiate signal handler
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return EXIT_FAILURE;
    }

    //printf("Main serverM booting up.");
    char singleRooms[1024][4]; //array to store single rooms info
    char doubleRooms[1024][4]; //array to store double rooms info
    char suites[1024][4]; //array to store suites info
    
    struct Member members[100]; //array to store members
    FILE *fp; //file reader
    char s[1024];
    fp = fopen("member.txt", "r"); //read members.txt file
    int linecount = 0; //count number of members
    while (fgets(s, sizeof s, fp) != NULL){ 
        //printf("%d: %s", ++linecount, s); 
        ++linecount;
        //printf("\n%s\n", scopy);
    }
    fclose(fp);
    fp = fopen("member.txt", "r");
    //store members in members struct array
    for(int i = 0; i<linecount; i++){
        fgets(s, sizeof s, fp);
        char * token = strtok(s, " ");
        while(token != NULL){
            strcpy(members[i].uName, token);
            int len = strlen(members[i].uName);
            members[i].uName[len-1] = '\0';
            token = strtok(NULL, " ");
            strcpy(members[i].pass, token);
            token = strtok(NULL, " ");
        } 
    }
    fclose(fp);

    int flags = 0; //keep track of number of backend servers message is received from
    int flags_tcp = 0; //keep track of tcp clients
    struct sockaddr_in server_addr, client_addr, client_addr2, client_addr3; //store main server addr and three backend server address in seperate addr structures
    char server_message[2000], client_message[2000]; //message buffers
    int client_struct_length = sizeof(client_addr);
    int client_struct_length2 = sizeof(client_addr2);
    int client_struct_length3 = sizeof(client_addr3);

    struct sockaddr_in server_addr_tcp, client_addr_tcp; //socket info for client tcp communication
    char server_message_tcp[2000], client_message_tcp[2000];

    memset(server_message_tcp, '\0', sizeof(server_message_tcp));
    memset(client_message_tcp, '\0', sizeof(client_message_tcp));


    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    
    // Create UDP socket:
   // close(socket_desc);
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    socket_desc_tcp = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    if(socket_desc_tcp < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    //printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(44278); //main server port 
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_port = htons(45278); //client server port
    server_addr_tcp.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }

    if(bind(socket_desc_tcp, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }

    printf("The main server is up and running.\n");
    //printf("Done with binding\n");

    //printf("Listening for incoming messages...\n\n");

    //receive room info from backend servers and receive room info from all three first
    while(flags < 3) {
        // Receive client's message:
        if(flags == 0){ //recv from Server S (singles)
            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
            (struct sockaddr*)&client_addr, &client_struct_length) < 0){
            //printf("Couldn't receive\n");
            return -1;
            }
            //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        } else if (flags == 1){ //recv from Server D (doubles)
            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
            (struct sockaddr*)&client_addr2, &client_struct_length2) < 0){
            //printf("Couldn't receive\n");
            return -1;
            }
            //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr2.sin_addr), ntohs(client_addr2.sin_port));
        } else if (flags == 2){ //recv from Server U (suites)
            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
            (struct sockaddr*)&client_addr3, &client_struct_length3) < 0){
            //printf("Couldn't receive\n");
            return -1;
            }
            //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr3.sin_addr), ntohs(client_addr3.sin_port));

        }
        // strcpy(server_message, "This is a test message 1");

        // if (sendto(socket_desc, &server_message, strlen(server_message), 0,
        //     (struct sockaddr*)&client_addr, client_struct_length) < 0){
        //     printf("Can't send\n");
        //     return -1;
        // }

        

        //printf("Msg from client: %s\n", client_message);
        if(strchr(client_message, 'S')){ //print message for recv from Server S and update single rooms array
            flags++;
            printf("\nThe main server has received the room status from Server S using UDP over port 44278.\n");
            for(int i = 0; i<strlen(client_message); i++){
            singleRooms[i][0] = client_message[i*4];
            singleRooms[i][1] = client_message[i*4+1];
            singleRooms[i][2] = client_message[i*4+2];
            singleRooms[i][3] = client_message[i*4+3];
            if(strchr(singleRooms[i], 'S')){
                //printf("Recv Code: %s\n", singleRooms[i]);
            } else {
                break;
            }

         }
          
        } else if (strchr(client_message, 'D')){ //print message for recv from Server D and update double rooms array
                flags++;
                printf("The main server has received the room status from Server D using UDP over port 44278.\n");
                for(int i = 0; i<strlen(client_message); i++){
                doubleRooms[i][0] = client_message[i*4];
                doubleRooms[i][1] = client_message[i*4+1];
                doubleRooms[i][2] = client_message[i*4+2];
                doubleRooms[i][3] = client_message[i*4+3];
                if(strchr(doubleRooms[i], 'D')){
                    //printf("Recv Code: %s\n", doubleRooms[i]);
                } else {
                    break;
                }
            }
        } else if (strchr(client_message, 'U')){//print message for recv from Server U and update suites array
            flags++;
            printf("The main server has received the room status from Server U using UDP over port 44278.\n");
            for(int i = 0; i<strlen(client_message); i++){
                suites[i][0] = client_message[i*4];
                suites[i][1] = client_message[i*4+1];
                suites[i][2] = client_message[i*4+2];
                suites[i][3] = client_message[i*4+3];
                if(strchr(suites[i], 'U')){
                    //printf("Recv Code: %s\n", suites[i]);
                } else {
                    break;
                }
            }
        }

        //strcpy(singleRooms, client_message);
        // Respond to client:
        
        // strcpy(server_message, "This is a test message");

        // if (sendto(socket_desc, server_message, strlen(server_message), 0,
        //     (struct sockaddr*)&client_addr, client_struct_length) < 0){
        //     printf("Can't send\n");
        //     return -1;
        // }

        
    }

    //listen for clients
    if(listen(socket_desc_tcp, 2) < 0){ //upto two clients
            printf("Error while listening\n");
            return -1;
        }
        //printf("\nListening for incoming connections.....\n");

        

/*
    while(flags_tcp < 2){
        if(listen(socket_desc_tcp, 1) < 0){
            printf("Error while listening\n");
            return -1;
        }
        printf("\nListening for incoming connections.....\n");

        client_size_tcp = sizeof(client_addr_tcp);
        client_sock_tcp = accept(socket_desc_tcp, (struct sockaddr*)&client_addr_tcp, &client_size_tcp);

        if (client_sock_tcp < 0){
            printf("Can't accept\n");
            return -1;
        }

        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port));
        flags_tcp++;
    }
    */

    while(1){
        memset(client_message_tcp, '\0', sizeof(client_message_tcp));
        client_size_tcp = sizeof(client_addr_tcp);
        client_sock_tcp = accept(socket_desc_tcp, (struct sockaddr*)&client_addr_tcp, &client_size_tcp);
        if(flags_tcp < 2) {
            flags_tcp++;
        } else if (client_sock_tcp < 0){
            //printf("Can't accept\n");
            return -1;
        }

        pid_t pid = fork(); //fork (not to use for eating something)
        if(pid < 0){
            perror("Error forking process");
            return -1;
        }

        if(pid == 0){
            close(socket_desc_tcp); 
            //printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port));
        
            int checker = 0;
            while(1){
                //recv from clients
                memset(client_message_tcp, '\0', sizeof(client_message_tcp));
                ssize_t bytes_received = recv(client_sock_tcp, client_message_tcp, sizeof(client_message_tcp), 0);
                if (bytes_received < 0) {
                    //perror("Error receiving data");
                    // Handle error, possibly close the connection or return -1
                    break; // Exit the loop on error
                } else if (bytes_received == 0) {
                    // Client has closed the connection
                    printf("Client disconnected\n");
                    break; // Exit the loop when the client disconnects
                }

                //printf("Msg from client: %s\n", client_message_tcp);
                //printf("Msg from client: %s\n", client_message_tcp);
                //printf("Msg from client: %s\n", client_message_tcp);
                char curr_user[50]; //keep track of current user
                char uName_toCheck[50]; //buffer 1 (not used for just username; I was just too lazy to change variable names)
                char pass_toCheck[50]; //buffer 2

                // if(strstr(client_message_tcp, "Availability") != NULL){
                //     printf("reached here...\n");
                //     //char *tok = strtok(client_message_tcp, "\n");
                //     int l = strlen(client_message_tcp);
                //     printf("\n%s; length: %d\n", strchr(client_message_tcp, ' '), l);

                // } else{
                char *tok = strtok(client_message_tcp, " "); //strtok used for parsing string
                while(tok != NULL){
                    strcpy(uName_toCheck, tok);
                    int lastChar = strlen(uName_toCheck);
                    if(strcmp(uName_toCheck, "Availability") && strcmp(uName_toCheck, "Reservation")){
                        uName_toCheck[lastChar-1] = '\0';
                    }
                    //printf("\n%s\n", uName_toCheck);
                    tok = strtok(NULL, " ");
                    strcpy(pass_toCheck, tok);
                    tok = strtok(NULL, " ");
                }
                // }
        
                //printf("Word 1: %s, Word 2: %s\n", uName_toCheck, pass_toCheck);
                if(!strcmp(uName_toCheck, "Availability")){
                    printf("The main server has received the availability request on Room %s from %s using TCP over port 45278\n", pass_toCheck, curr_user);
                    //printf("first char: %c\n", pass_toCheck[0]);
                    if (pass_toCheck[strlen(pass_toCheck) - 1] == '\n') {
                        pass_toCheck[strlen(pass_toCheck) - 1] = '\0'; // Replace '\n' with '\0'
                    }
                    if(strlen(pass_toCheck) > 4){
                        pass_toCheck[strlen(pass_toCheck)-1] = '\0';
                    }
                    char first = (pass_toCheck[0]);
                    if((first == 'S')){
                        strcpy(server_message, pass_toCheck);
                        //printf("server mess: %s\n", server_message);

                        if (sendto(socket_desc, &server_message, strlen(server_message), 0,
                            (struct sockaddr*)&client_addr, client_struct_length) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent a request to Server S.\n");

                        //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        memset(client_message, 0, sizeof(client_message));
                        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                            //printf("Couldn't receive\n");
                            return -1;
                        }
                        //printf("Room avail: %s\n", client_message);
                        printf("The main server received the response from Server S using UDP over port 44278.\n");
                        memset(server_message_tcp, 0, sizeof(server_message_tcp));
                        strcat(server_message_tcp, client_message);
                        if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent the availability information to the client.\n");

                    } else if((first == 'D')){
                        strcpy(server_message, pass_toCheck);
                        //printf("server mess: %s\n", server_message);
                        if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr2, client_struct_length2) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent a request to Server D.\n");
                        memset(client_message, 0, sizeof(client_message));
                        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr2, &client_struct_length2) < 0){
                            printf("Couldn't receive\n");
                            return -1;
                        }
                        //printf("Room avail: %s\n", client_message);
                        printf("The main server received the response from Server D using UDP over port 44278.\n");
                        memset(server_message_tcp, 0, sizeof(server_message_tcp));
                        strcat(server_message_tcp, client_message);
                        if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent the availability information to the client.\n");

                    }else if((first == 'U')){
                
                        strcpy(server_message, pass_toCheck);
                        if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr3, client_struct_length3) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent a request to Server U.\n");
                        memset(client_message, 0, sizeof(client_message));
                        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr3, &client_struct_length3) < 0){
                            printf("Couldn't receive\n");
                            return -1;
                        }
                        //printf("Room avail: %s\n", client_message);
                        printf("The main server received the response from Server D using UDP over port 44278.\n");
                        memset(server_message_tcp, 0, sizeof(server_message_tcp));
                        strcat(server_message_tcp, client_message);
                        if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent the availability information to the client.\n");

                    }else{
                        //to doo
                        memset(server_message_tcp, 0, sizeof(server_message_tcp));
                        strcat(server_message_tcp, "invalid");
                        if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server found invalid room type request.\n");

                    }
                    // if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
                    // (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                    //     printf("Couldn't receive\n");
                    //     return -1;
                    // }
                    // printf("Room avail: %s", client_message);
                } else if(!strcmp(uName_toCheck, "Reservation")) {
                    printf("The main server has received the reservation request on Room %s from %s using TCP over port 45278\n", pass_toCheck, curr_user);
                    if(checker == 0){
                        printf("%s cannot make a reservation.\n", curr_user);
                        memset(server_message_tcp, 0, sizeof(server_message_tcp));
                        strcat(server_message_tcp, "Permission denied: Guest cannot make a reservation.");
                        if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        printf("The main server sent the error message to the client.\n");

                    } else {
                        //printf("first char: %c\n", pass_toCheck[0]);
                        if (pass_toCheck[strlen(pass_toCheck) - 1] == '\n') {
                            pass_toCheck[strlen(pass_toCheck) - 1] = '\0'; // Replace '\n' with '\0'
                        }
                        if(strlen(pass_toCheck) > 4){
                            pass_toCheck[strlen(pass_toCheck)-1] = '\0';
                        }
                        char first = (pass_toCheck[0]);
                        if((first == 'S')){
                            strcpy(server_message, "R");
                            strcat(server_message, pass_toCheck);
                            //printf("server mess: %s\n", server_message);

                            if (sendto(socket_desc, &server_message, strlen(server_message), 0,
                                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server sent a request to Server S.\n");

                            //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                            memset(client_message, 0, sizeof(client_message));
                            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                                printf("Couldn't receive\n");
                                return -1;
                            }
                            //printf("Room avail: %s\n", client_message);
                            printf("The main server received the response from Server S using UDP over port 44278.\n");
                            if(strstr(client_message, "none") || (client_message[0] == '0')){
                                printf("The main server received the response from server S using UDP over port 44278.\n");
                            } else {
                                printf("The main server received the response and updated the room number from server S using UDP over port 44278.\n");
                            }
                            memset(server_message_tcp, 0, sizeof(server_message_tcp));
                            strcat(server_message_tcp, client_message);
                            if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server sent the availability information to the client.\n");

                        } else if((first == 'D')){
                            strcpy(server_message, "R");
                            strcat(server_message, pass_toCheck);
                            printf("server mess: %s\n", server_message);
                            if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr2, client_struct_length2) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server sent a request to Server D.\n");
                            memset(client_message, 0, sizeof(client_message));
                            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr2, &client_struct_length2) < 0){
                                printf("Couldn't receive\n");
                                return -1;
                            }
                            //printf("Room avail: %s\n", client_message);
                            printf("The main server received the response from Server D using UDP over port 44278.\n");
                            if(strstr(client_message, "none") || (client_message[0] == '0')){
                                printf("The main server received the response from server D using UDP over port 44278.\n");
                            } else {
                                printf("The main server received the response and updated the room number from server D using UDP over port 44278.\n");
                            }
                            memset(server_message_tcp, 0, sizeof(server_message_tcp));
                            strcat(server_message_tcp, client_message);
                            if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server sent the reservation information to the client.\n");

                        }else if((first == 'U')){
                            strcpy(server_message, "R");
                            strcat(server_message, pass_toCheck);
                            if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr3, client_struct_length3) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server sent a request to Server U.\n");
                            memset(client_message, 0, sizeof(client_message));
                            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr3, &client_struct_length3) < 0){
                                printf("Couldn't receive\n");
                                return -1;
                            }
                            //printf("Room avail: %s\n", client_message);
                            printf("The main server received the response from Server U using UDP over port 44278.\n");
                            if(strstr(client_message, "none") || (client_message[0] == '0')){
                                printf("The main server received the response from Server U using UDP over port 44278.\n");
                            } else {
                                printf("The main server received the response and updated the room number from server U using UDP over port 44278.\n");
                            }
                            memset(server_message_tcp, 0, sizeof(server_message_tcp));
                            strcat(server_message_tcp, client_message);
                            if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server sent the availability information to the client.\n");

                        }else{
                            //to doo
                            memset(server_message_tcp, 0, sizeof(server_message_tcp));
                            strcat(server_message_tcp, "invalid");
                            if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("The main server found invalid room type request.\n");

                        }

                    }
            

                } else {
                    strcpy(curr_user, uName_toCheck);
        
                    for(int i = 0; i<linecount; i++){
                        int asdf = strlen(pass_toCheck);
                        //printf("pass_toCheck: %s; length: %d\n", pass_toCheck, (pass_toCheck[0] == '\n'));

                        if(!strcmp(uName_toCheck, members[i].uName)){
                            if(!strcmp(pass_toCheck, members[i].pass)){
                                strcpy(server_message_tcp, "member");
                                checker = 1;
                                printf("The main server received the authentication for %s using TCP over port 45278.\n", uName_toCheck);
                                break;
                            } else {
                                if((pass_toCheck[0] == '\n') || !strcmp(pass_toCheck, "\0") || !strcmp(pass_toCheck, "\n")){
                                    printf("The main server received the guest request for %s using TCP over port 45278.\n", uName_toCheck);
                                    strcpy(server_message_tcp, "guest");
                                    checker = 0;
                                } else {
                                    strcpy(server_message_tcp, "wrong");
                                    checker = 1;
                                }
                                break;
                            }
                        } else {
                            strcpy(server_message_tcp, "guest");
                            if((i == linecount-1)){
                                if(pass_toCheck[0] != '\n'){
                                    checker = 1;
                                    strcpy(server_message_tcp, "existnot");
                                    printf("The main server received the authentication for %s using TCP over port 45278.\n", uName_toCheck);
                                } else {
                                    checker = 0;
                                    printf("The main server received the guest request for %s using TCP over port 45278.\n", uName_toCheck);
                                }
                                
                            }
                        }
                    }

                    //strcpy(server_message_tcp, "This is the server's message.");
                    if (send(client_sock_tcp, server_message_tcp, strlen(server_message_tcp), 0) < 0){
                        printf("Can't send\n");
                        return -1;
                    }

                    if(checker == 1){
                        printf("The main server sent the authentication result to the client.\n");
                    } else {
                        printf("The main server sent the guest response to the client.\n");
                    }
                }
            }
        
        
            close(client_sock_tcp);

        } else {
            close(client_sock_tcp);
        }

    } 
}