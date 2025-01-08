#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>

//socket descriptor
int socket_desc;

//to handle ctrl+c exit and make sure sockets close before exiting program
void sigint_handler(int signum) {
    // Close TCP socket
    if (close(socket_desc) == -1) {
        perror("Error closing TCP socket");
    }
    exit(EXIT_SUCCESS);
}

//main
int main (void){
    //set up signal handler
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Error setting signal handler");
        return EXIT_FAILURE;
    }


    printf("Client is up and running.\n");
    int guest_flag = 0; //flag for indicating a guest
    struct sockaddr_in server_addr; //client address stored here
    char server_message[2000], client_message[2000]; 
    socklen_t addrLen = sizeof(server_addr);

    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    //create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }

    //printf("Socket created successfully\n");

    //set socket addr; USC ID last 3 digits: 278
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(45278);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    //printf("Connected with server successfully\n");

    if (getsockname(socket_desc, (struct sockaddr *)&server_addr, (socklen_t *)&addrLen) < 0) {
        perror("Error getting socket name");
        
    }
    //printf("Connected to server. Local port: %d\n", ntohs(server_addr.sin_port));


    char username[50]; //input buffer for username
    char username2[50]; //store copy of original (unencrypted) username
    char password[50] = ""; //input buffer for password
    printf("Please enter the username: ");
    fgets(username, 50, stdin);
    printf("Please enter the password: ");
    fgets(password, 50, stdin);
    strcpy(username2, username); //store copy of original username
    
    //encrypt username
    for(int i=0; i<strlen(username); i++){
        int direction = islower(username[i]) ? 'a' : 'A';
        username[i] = ((username[i] - direction + 3) % 26) + direction;
    }
    //identify if user is a guest or not
    if(password[0] == '\n'){
        guest_flag = 1;
    }
    //encrypt password
    for(int j=0; j<strlen(password); j++){
        if(isdigit(password[j])){
            if(password[j] == '7'){
                password[j] = '0';
            } else if (password[j] == '8'){
                password[j] = '1';
            } else if (password[j] == '9'){
                password[j] = '2';
            } else {
                password[j] = password[j]+3;
            }
        } else if(isalpha(password[j])){
            int direction_pass = islower(password[j]) ? 'a' : 'A';
            password[j] = ((password[j] - direction_pass + 3) % 26) + direction_pass;
        }
    }

    
    //store encrypted username and password to send to main server for authentication
    strcat(client_message, username);
    strcat(client_message, " ");
    strcat(client_message, password);

    //send to server
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    if(guest_flag == 1){
        printf("%s sent a guest request to the main server using TCP over port %d.\n", username2, ntohs(server_addr.sin_port));
    } else {
        printf("%s sent an authentication request to the main server.\n", username2);
    }

    //recv from main server
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    //printf("Server's response: %s\n",server_message);

    //determine user fate based on recved response (master server)
    int wrong_pass = 0;
    if(guest_flag == 1){
        printf("Welcome guest %s\n", username2);
        wrong_pass = 0;
    } else if(!strcmp(server_message, "member")){
        printf("Welcome member %s\n", username2);
        wrong_pass = 0;
    } else if(!strcmp(server_message, "wrong")){
        printf("Failed login: Password does not match.\n");
        wrong_pass = 1;
    } else if(!strcmp(server_message, "existnot")){
        printf("Failed login: Username does not exist.\n");
        wrong_pass = 1;
    }else if(!strcmp(server_message, "guest")){
        printf("Failed login: Username does not exist.\n");
        wrong_pass = 0;
    }

    //until wrong_pass (unauthenticated user and non-guest) status is valid,
    //keep asking for username and password
    while (wrong_pass == 1){
        memset(server_message,'\0',sizeof(server_message));
        printf("Please enter the username: ");
        fgets(username, 50, stdin);
        printf("Please enter the password: ");
        fgets(password, 50, stdin);
        strcpy(username2, username);
        for(int i=0; i<strlen(username); i++){
            int direction = islower(username[i]) ? 'a' : 'A';
            username[i] = ((username[i] - direction + 3) % 26) + direction;
        }
        if(password[0] == '\n'){
            guest_flag = 1;
        }
        for(int j=0; j<strlen(password); j++){
            if(isdigit(password[j])){
                if(password[j] == '7'){
                    password[j] = '0';
                } else if (password[j] == '8'){
                    password[j] = '1';
                } else if (password[j] == '9'){
                    password[j] = '2';
                } else {
                    password[j] = password[j]+3;
                }
            } else if(isalpha(password[j])){
                int direction_pass = islower(password[j]) ? 'a' : 'A';
                password[j] = ((password[j] - direction_pass + 3) % 26) + direction_pass;
            }
        }

        
        memset(client_message,'\0',sizeof(client_message));
        strcat(client_message, username);
        strcat(client_message, " ");
        strcat(client_message, password);

        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }

        if(guest_flag == 1){
            printf("%s sent a guest request to the main server using TCP over port %d.\n", username2, ntohs(server_addr.sin_port));
        } else {
            printf("%s sent an authentication request to the main server.\n", username2);
        }
        
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }

        //printf("Server's response: %s\n",server_message);
        if(guest_flag == 1){
            printf("Welcome guest %s\n", username2);
            wrong_pass = 0;
        } else if(!strcmp(server_message, "member")){
            printf("Welcome member %s\n", username2);
            wrong_pass = 0;
        } else if(!strcmp(server_message, "wrong")){
            printf("Failed login: Password does not match.\n");
            wrong_pass = 1;
        } else if(!strcmp(server_message, "existnot")) {
            printf("Failed login: Username does not exist.\n");
            wrong_pass = 1;
        } else if(!strcmp(server_message, "guest")){
            printf("Failed login: Username does not exist.\n");
            wrong_pass = 0;
        }
        memset(server_message,'\0',sizeof(server_message));
    }


    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    char roomcode[50]; //buffer for inputted roomcode
    char action[50]; //buffer for desired action (inputed)

    char *newline_pos; //will be used to modify the input buffers to ensure proper formatting
    char *newline_pos2;

    //main while loop
    while(1){
        int invalidComm = 0; //checker for invalid command input
        printf("Please enter the room code: ");
        fgets(roomcode, 50, stdin);
        newline_pos = strchr(roomcode, '\n');
        if (newline_pos != NULL) {
            *newline_pos = '\0'; 
        }
        //roomcode[strcspn(roomcode, '\n')] = '\0';
        // if(!strcmp(roomcode[0], 'S') || !strcmp(roomcode[0], 'D') || !strcmp(roomcode[0], 'U')){
        //     printf("Invalid roomcode\n");
        // }
        printf("Would you like to search for the availability or make a reservation? (Enter \"Availability\" to search for the availability or Enter \"Reservation\" to make a reservation): ");
        fgets(action, 50, stdin);
        newline_pos2 = strchr(action, '\n');
        if (newline_pos2 != NULL) {
            *newline_pos2 = '\0'; 
        }
        //printf("Action: %s\n", action);
        if((strcmp(action, "Availability") != 0) && (strcmp(action, "Reservation") != 0)){
            invalidComm = 1;
            printf("Invalid command. Please type either \"Availability\" or \"Reservation\"\n");
        } else {
            invalidComm = 0;
        }

        while(invalidComm){ //keep asking for action until a valid command is entered
            printf("Would you like to search for the availability or make a reservation? (Enter \"Availability\" to search for the availability or Enter \"Reservation\" to make a reservation): ");
            fgets(action, 50, stdin);
            newline_pos2 = strchr(action, '\n');
            if (newline_pos2 != NULL) {
                *newline_pos2 = '\0'; 
            }
            //printf("Action: %s\n", action);
            if((strcmp(action, "Availability") != 0) && (strcmp(action, "Reservation") != 0)){
                invalidComm = 1;
                printf("Invalid command. Please type either \"Availability\" or \"Reservation\"\n");
            } else {
                invalidComm = 0;
            }
        }

        //prepare to send action and roomcode to main server
        strcpy(client_message, action);
        strcat(client_message, " ");
        strcat(client_message, roomcode);
        //printf("Mess to send: %s", client_message);
        //strcat(client_message, roomcode);
        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }
        if(strstr(action, "Availability")){
            printf("%s sent an availability request to main server.\n", username2);
        } else if(strstr(action, "Reservation")){
            printf("%s sent a reservation request to main server.\n", username2);
        }
        memset(server_message,'\0',sizeof(server_message));
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){

        }  
        //recv response from main server and print valid message  
        if(!strcmp(action, "Availability")){
            //printf("Availability info received: %s\n", server_message);
            if(strstr(server_message, "none")){
                printf("The client received the response from main server using TCP over port 45278.\nNot able to find the room layout.\n\n-----Start a new request-----\n");
            } else if (strstr(server_message, "invalid")){
                printf("The client received the response from main server using TCP over port 45278.\nInvalid room type.\n\n-----Start a new request-----\n");
            }else if(!strcmp(server_message, "0")){
                printf("The client received the response from main server using TCP over port 45278.\nThe requested room is not available.\n\n-----Start a new request-----\n");
            } else {
                printf("The client received the response from main server using TCP over port 45278.\nThe requested room is available.\n\n-----Start a new request-----\n");
            }

        } else if(!strcmp(action, "Reservation")){
            if(guest_flag == 1){
                printf("%s\n\n-----Start a new request-----\n", server_message);
            } else {
                if(strstr(server_message, "none")){
                    printf("The client received the response from main server using TCP over port 45278.\nOops! Not able to find the room.\n\n-----Start a new request-----\n");
                } else if(!strcmp(server_message, "0")){
                    printf("The client received the response from main server using TCP over port 45278.\nSorry! The requested room is not available.\n\n-----Start a new request-----\n");
                } else {
                    printf("The client received the response from main server using TCP over port 45278.\nCongratulation! The reservation for %s has been made.\n\n-----Start a new request-----\n", roomcode);
                }
            }
        }
        
    }
    
}