# DormitoryReservation_SocketProgramming
A socket programming project in C language featuring a dormitory reservation and search system. 

This project is programmed in C and utilizes socket programming for a dormitory reservation system for students. 

Users, or clients, can initiate a communication by opening a TCP socket with the main backend server. There are three types of rooms available with codes S (single), D (double), and U (suites). Each room type has a designated backend server which can only communicate with the main backend server. Thus, the client cannot connect with the three subsidiary backend servers. 
When a client makes a request to book or check the availability of a specific room type (single, double, or suite), the main server initiates a UDP connection with the respective backend server and updates the room status or retrieves availability information. If a room is requested to be booked but is not available, an appropriate response message is delivered to the client by the main server. 

Code files:
   serverM.c --> main server which communicates with the clients (TCP) and 
                 backend servers (UDP)
   serverS.c --> Backend Server S, which represents the single rooms 
                 available and responds to client requests forwarded by
                 main server (UDP)
   serverD.c --> Backend Server D, which represents the double rooms 
                 available and responds to client requests forwarded by
                 main server (UDP)
   serverU.c --> Backend Server U, which represents the suites available
                 and responds to client requests forwarded by main server
                 (UDP)
   client.c  --> Client code which communicates with main server (TCP) and 
                 checks availability or rooms and requests reservations,
                 if logged in and authenticated by main server

***above updates as of April 2024***

Future Directions/Feature Implementations:
- implement front-end interface

***above updates as of December 2024***
