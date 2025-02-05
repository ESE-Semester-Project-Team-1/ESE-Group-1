#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <cassert>
#include <unistd.h>

struct message {
    std::string messg;
    int id;
    int priority;
    int receiver;
    bool valid;

    message(std::string msg, int i, int p, int r, bool v) 
        : messg(msg), id(i), priority(p), receiver(r), valid(v) {}
};

std::queue<message> message_receiver_queue_V2X;
std::queue<message> message_receiver_queue_V2P;
std::queue<message> message_transmitter_queue_V2P;
std::queue<message> message_transmitter_queue_V2X;
std::mutex mu;

void behaviourModule() {
    while (true) {
        if (!message_receiver_queue_V2X.empty()) {
            mu.lock();
            message msg = message_receiver_queue_V2X.front();
            message_receiver_queue_V2X.pop();
            mu.unlock();

            if (msg.messg == "Request to pass") {
                message msgFol = message("Increase Distance", 0, 100, 1, true);
                mu.lock();
                message_transmitter_queue_V2P.push(msgFol);
                mu.unlock();
                break;
            } else {
                std::cout << "Invalid message received: " << msg.messg << std::endl;
                break;
            }
        }

        if (!message_receiver_queue_V2P.empty()) {
            mu.lock();
            message msg = message_receiver_queue_V2P.front();
            message_receiver_queue_V2P.pop();
            mu.unlock();

            if (msg.messg == "Distance increased") {
                message msgFol = message("Safe to pass", 0, 100, 1, true);
                mu.lock();
                message_transmitter_queue_V2X.push(msgFol);
                mu.unlock();
                break;
            } else {
                std::cout << "Invalid message received: " << msg.messg << std::endl;
                break;
            }
        }
    }
}

void test_behaviourModule() {
    message test_msg("Request to pass", 0, 100, 1, true);
    mu.lock();
    message_receiver_queue_V2X.push(test_msg);
    mu.unlock();

    std::thread behaviour_thread(behaviourModule);
    sleep(1);
    behaviour_thread.join();

    mu.lock();
    assert(!message_transmitter_queue_V2P.empty());
    message sent_msg = message_transmitter_queue_V2P.front();
    message_transmitter_queue_V2P.pop();
    mu.unlock();
    
    assert(sent_msg.messg == "Increase Distance");
    std::cout << "Leader behaviour test passed!" << std::endl;
}

int main() {
    test_behaviourModule();
    return 0;
}
