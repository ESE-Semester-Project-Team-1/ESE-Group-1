#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
//#include "message.h"

#define PORT_V2X 8080
#define PORT_V2P 8081
#define FOLLOWER 1
#define CAR 0


std::mutex mu;

//class message
class message {
public:
    std::string messg;
    int id;
    int timeStamp;
    int receiverPort;
    int valid;

    message() : messg(""), id(0), timeStamp(0), receiverPort(0), valid(0) {}

    message(std::string new_msg, int new_id, int new_masa, int new_receiver_port, int valid_new) {
        messg = new_msg;
        id = new_id;
        timeStamp = new_masa;
        receiverPort = new_receiver_port;
        valid = valid_new;

    }

    // Serialize object into a byte array
    void serialize(char* buffer) const {
        // Serialize the integer fields
        memcpy(buffer, &id, sizeof(id));
        memcpy(buffer + sizeof(id), &timeStamp, sizeof(timeStamp));
        memcpy(buffer + sizeof(id) + sizeof(timeStamp), &receiverPort, sizeof(receiverPort));
        memcpy(buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort), &valid, sizeof(valid));

        // Serialize the string
        size_t messg_length = messg.length();
        memcpy(buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid), &messg_length, sizeof(size_t)); // Add length of the string
        memcpy(buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid) + sizeof(size_t), messg.c_str(), messg_length); // Add string data
    }

    // Deserialize byte array into object
    void deserialize(const char* buffer) {
        // Deserialize the integer fields
        memcpy(&id, buffer, sizeof(id));
        memcpy(&timeStamp, buffer + sizeof(id), sizeof(timeStamp));
        memcpy(&receiverPort, buffer + sizeof(id) + sizeof(timeStamp), sizeof(receiverPort));
        memcpy(&valid, buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort), sizeof(valid));

        // Deserialize the string
        size_t messg_length;
        memcpy(&messg_length, buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid), sizeof(size_t));

        char messg_buffer[messg_length + 1]; // Temporary buffer for string
        memcpy(messg_buffer, buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid) + sizeof(size_t), messg_length);
        messg_buffer[messg_length] = '\0'; // Null-terminate the string
        messg = std::string(messg_buffer);
    }

    // Print the message for debugging
    void print() const {
        std::cout << "Message: " << messg << ", ID: " << id << ", Timestamp: " << timeStamp
                  << ", ReceiverPort: " << receiverPort << ", Valid: " << valid << std::endl;
    }
};

std::queue<message> message_receiver_queue_V2X;
std::queue<message> message_receiver_queue_V2P;
std::queue<message> message_transmitter_queue_V2X;
std::queue<message> message_transmitter_queue_V2P;

// Function to set up the server and return the server socket descriptor
int setupServerV2X() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

 /*   // Configure socket options to allow reuse of the address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
*/
    // Define the server's address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT_V2X);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is set up and listening on port V2X " << PORT_V2X << "...\n";
    return server_fd; // Return the server socket descriptor
}

int setupServerV2P() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

 /*   // Configure socket options to allow reuse of the address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
*/
    // Define the server's address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT_V2P);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is set up and listening on port V2P " << PORT_V2P << "...\n";
    return server_fd; // Return the server socket descriptor
}

void receiverModuleV2X(int sock){

    while(true){
        // Receive data
        char buffer[1024] = {0};
        int valread = recv(sock, buffer, 1024, 0);
    
        if (valread < 0) {
            std::cerr << "Failed to receive data!" << std::endl;
            close(sock);
            return;
        }
        else if (valread == 0){
            std::cerr << "Connection closed by server!" << std::endl;
            close(sock);
            return;
        }
        else{

            // Deserialize the received data
            message msg1;
            msg1.deserialize(buffer);

            //debugging
            std::cout << "Received message: " << msg1.messg << std::endl;

            //store data in message queue
            mu.lock();
            message_receiver_queue_V2X.push(msg1);
            mu.unlock();

            //const message& item = message_receiver_queue.front();
            //item.print();

        }
    }   
}

void receiverModuleV2P(int sock){

    while(true){
        // Receive data
        char buffer[1024] = {0};
        int valread = recv(sock, buffer, 1024, 0);
    
        if (valread < 0) {
            std::cerr << "Failed to receive data!" << std::endl;
            close(sock);
            return;
        }
        else if (valread == 0){
            std::cerr << "Connection closed by server!" << std::endl;
            close(sock);
            return;
        }
        else{

            // Deserialize the received data
            message msg1;
            msg1.deserialize(buffer);

            //debugging
            std::cout << "Received message: " << msg1.messg << std::endl;

            //store data in message queue
            mu.lock();
            //std::cout << "selesai mutex lock"<< std::endl;
            message_receiver_queue_V2P.push(msg1);
            mu.unlock();

            //const message& item = message_receiver_queue.front();
            //item.print();

        }
    }   
}

void transmitterModuleV2X(int sock){

    //checking the message queue for valid messages
    while (true){

        if(!message_transmitter_queue_V2X.empty()){

            //get data from message queue
            mu.lock();
            message msg = message_transmitter_queue_V2X.front();
            message_transmitter_queue_V2X.pop();
            mu.unlock();

            //sending process

            // Calculate buffer size
            size_t buffer_size = sizeof(int) * 4 + sizeof(size_t) + msg.messg.length();
            char* buffer = new char[buffer_size];

            msg.serialize(buffer);
            
            // Send the serialized data
            if (send(sock, buffer, buffer_size, 0) < 0) {
                std::cerr << "Failed to send the message!" << std::endl;
            } else {
                std::cout << "Message sent successfully!" << std::endl;
            }
            
            delete[] buffer; 
        }
    }

}

void transmitterModuleV2P(int sock){

    //checking the message queue for valid messages
    while (true){

        if(!message_transmitter_queue_V2P.empty()){

            //get data from message queue
            mu.lock();
            message msg = message_transmitter_queue_V2P.front();
            message_transmitter_queue_V2P.pop();
            mu.unlock();

            //sending process

            // Calculate buffer size
            size_t buffer_size = sizeof(int) * 4 + sizeof(size_t) + msg.messg.length();
            char* buffer = new char[buffer_size];

            msg.serialize(buffer);
            
            // Send the serialized data
            if (send(sock, buffer, buffer_size, 0) < 0) {
                std::cerr << "Failed to send the message!" << std::endl;
            } else {
                std::cout << "Message sent successfully!" << std::endl;
            }
            
            delete[] buffer; 
        }
    }

}

void handlerV2X(int server_fd, struct sockaddr_in address, int addrlen){
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "New client connected in V2X Port\n";

        // Create a thread to handle the client
        std::thread t1(receiverModuleV2X, new_socket);
        std::thread t2(transmitterModuleV2X, new_socket);

        // Detach the thread to allow it to run independently
        t1.detach();
        t2.detach();

    }
}

void handlerV2P(int server_fd, struct sockaddr_in address, int addrlen){
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "New client connected in V2P Port\n";

        // Create a thread to handle the client
        std::thread t1(receiverModuleV2P, new_socket);
        std::thread t2(transmitterModuleV2P, new_socket);

        // Detach the thread to allow it to run independently
        t1.detach();
        t2.detach();

    }
}

void behaviourModule(){
    
    while(true){

        // Process messages in the V2X queue if there are any
        if (!message_receiver_queue_V2X.empty()){

            //get data from message queue
            mu.lock();
            message msg = message_receiver_queue_V2X.front();
            message_receiver_queue_V2X.pop();
            mu.unlock();

            //std::cout << "Received message: " << msg.messg << std::endl;
            
            if (msg.messg == "Request to pass") {
                
                message msgFol = message("Increase Distance", 0, 100, FOLLOWER, 1);
                mu.lock();
                message_transmitter_queue_V2P.push(msgFol);
                mu.unlock();

            } else {
                std::cout << "Invalid message received: " << msg.messg << std::endl;
            }
        }

        // Process messages in the V2P queue if there are any
        if (!message_receiver_queue_V2P.empty()){

            //get data from message queue
            mu.lock();
            message msg = message_receiver_queue_V2P.front();
            message_receiver_queue_V2P.pop();
            mu.unlock();

            //std::cout << "Received message: " << msg.messg << std::endl;
            
            if (msg.messg == "Distance increased") {
                
                message msgFol = message("Safe to pass", 0, 100, FOLLOWER, 1);
                mu.lock();
                message_transmitter_queue_V2X.push(msgFol);
                mu.unlock();

            } else {
                std::cout << "Invalid message received: " << msg.messg << std::endl;
            }
        }
    }
}

int main() {
    // Step 1: Set up the serverV2X
    int server_fd_V2X = setupServerV2X();
    struct sockaddr_in addressV2X;
    int addrlenV2X = sizeof(addressV2X);

    // Step 2: Set up the serverV2P
    int server_fd_V2P = setupServerV2P();
    struct sockaddr_in addressV2P;
    int addrlenV2P = sizeof(addressV2P);

    //behaviour module
    std::thread behaviourThread(behaviourModule);

    // Create a thread to handle the V2X port
    std::thread t1(handlerV2X, server_fd_V2X, addressV2X, addrlenV2X);

    // Create a thread to handle the V2P port
    std::thread t2(handlerV2P, server_fd_V2P, addressV2P, addrlenV2P);

    // Join the threads before exiting
    behaviourThread.join();
    t1.join();
    t2.join();

    // Close the server socket before exiting
    close(server_fd_V2X);
    close(server_fd_V2P);
    std::cout << "Server shut down successfully.\n";
    return 0;
}
