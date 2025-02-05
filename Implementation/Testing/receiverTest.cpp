#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

struct message {
    std::string messg;
    
    void serialize(char* buffer) const {
        size_t len = messg.length();
        memcpy(buffer, &len, sizeof(size_t));
        memcpy(buffer + sizeof(size_t), messg.c_str(), len);
    }
    
    void deserialize(const char* buffer) {
        size_t len;
        memcpy(&len, buffer, sizeof(size_t));
        messg = std::string(buffer + sizeof(size_t), len);
    }
};

std::queue<message> message_receiver_queue;
std::mutex mu;

void receiverModule(int sock) {
    while (true) {
        char buffer[1024] = {0};
        int valread = recv(sock, buffer, 1024, 0);
    
        if (valread < 0) {
            std::cerr << "Failed to receive data!" << std::endl;
            close(sock);
            return;
        } else if (valread == 0) {
            std::cerr << "Connection closed by server!" << std::endl;
            close(sock);
            return;
        } else {
            message msg1;
            msg1.deserialize(buffer);
            std::cout << "Received message: " << msg1.messg << std::endl;

            mu.lock();
            message_receiver_queue.push(msg1);
            mu.unlock();

            // Exit after receiving one message
            break;
        }
    }
    close(sock);
}


void test_receiverModule() {
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    std::thread receiver_thread(receiverModule, sockets[1]);
    sleep(1);

    message test_msg;
    test_msg.messg = "Test Message!";
    char buffer[1024];
    test_msg.serialize(buffer);
    send(sockets[0], buffer, sizeof(size_t) + test_msg.messg.length(), 0);

    sleep(1);

    mu.lock();
    if (!message_receiver_queue.empty()) {
        message received_msg = message_receiver_queue.front();
        message_receiver_queue.pop();
        std::cout << "Test received message: " << received_msg.messg << std::endl;
    } else {
        std::cerr << "No message received!" << std::endl;
    }
    mu.unlock();

    close(sockets[0]);

    // Ensure receiver thread joins properly
    if (receiver_thread.joinable()) {
        receiver_thread.join();
    }
}


int main() {
    test_receiverModule();
    return 0;
}
