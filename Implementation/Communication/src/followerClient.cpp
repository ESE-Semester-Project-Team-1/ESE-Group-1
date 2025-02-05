#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
//#include "message.h"

#define PORT 8081
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

std::queue<message> message_receiver_queue;
std::queue<message> message_transmitter_queue;

void increaseDistance(){

    std::cout << "Distance has been increased\n";
    message msg = message("Distance increased", 0, 100, CAR, 1);
    
    //store data in message transmitter queue
    mu.lock();
    message_transmitter_queue.push(msg);
    mu.unlock();
}

int setupClientConnection(const char* ipAddress, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, ipAddress, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported\n";
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

    return sock;
}

void receiverModule(int sock){

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
            message_receiver_queue.push(msg1);
            mu.unlock();

        }
    }   
}

void transmitterModule(int sock){

    //checking the message queue for valid messages
    while (true){

        if(!message_transmitter_queue.empty()){

            //get data from message queue
            mu.lock();
            message msg = message_transmitter_queue.front();
            message_transmitter_queue.pop();
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

void behaviourModule(){

    while(true){

        // Process messages in the queue if there are any
        if (!message_receiver_queue.empty()){

            //get data from message queue
            mu.lock();
            message msg = message_receiver_queue.front();
            message_receiver_queue.pop();
            mu.unlock();

            //std::cout << "Received message: " << msg.messg << std::endl;
            
            if (msg.messg == "Increase Distance") {
                
                increaseDistance();

            } else {
                
                std::cout << "Invalid command\n";
            }
            
        }
    }
}

int main() {
    
    // Set up client connection
    int sock = setupClientConnection("127.0.0.1", PORT);
    if (sock < 0) {
        return -1; // Exit if connection setup failed
    }

    //recieverModule
    std::thread t1(receiverModule, sock);

    //behaviourModule
    std::thread t2(behaviourModule);

    //transmitterModule
    std::thread t3(transmitterModule, sock);

    t1.join();
    t2.join();
    t3.join();

    close(sock);

    return 0;
}
