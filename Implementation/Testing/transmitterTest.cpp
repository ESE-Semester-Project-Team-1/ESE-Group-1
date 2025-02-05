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
};

std::queue<message> message_transmitter_queue;
std::mutex mu;

void transmitterModule(int sock) {
    while (true) {
        if (!message_transmitter_queue.empty()) {
            mu.lock();
            message msg = message_transmitter_queue.front();
            message_transmitter_queue.pop();
            mu.unlock();

            size_t buffer_size = sizeof(size_t) + msg.messg.length();
            char* buffer = new char[buffer_size];
            msg.serialize(buffer);

            if (send(sock, buffer, buffer_size, 0) < 0) {
                std::cerr << "Failed to send the message!" << std::endl;
            }
            delete[] buffer;
        }
    }
}

void test_transmitterModule() {
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    message test_msg;
    test_msg.messg = "Hello, World!";
    
    mu.lock();
    message_transmitter_queue.push(test_msg);
    mu.unlock();

    std::thread transmitter_thread(transmitterModule, sockets[0]);
    sleep(1);

    char buffer[1024];
    ssize_t received = recv(sockets[1], buffer, sizeof(buffer), 0);
    if (received > 0) {
        size_t msg_len;
        memcpy(&msg_len, buffer, sizeof(size_t));
        std::string received_msg(buffer + sizeof(size_t), msg_len);
        std::cout << "Received message: " << received_msg << std::endl;
        std::cout << "Test successfull " << std::endl;
    } else {
        std::cerr << "Failed to receive message" << std::endl;
    }

    close(sockets[0]);
    close(sockets[1]);
    transmitter_thread.detach();
}

int main() {
    test_transmitterModule();
    return 0;
}
