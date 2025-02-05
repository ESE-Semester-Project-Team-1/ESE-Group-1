#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <cassert>
#include <unistd.h>

struct message {
    std::string messg;
};

std::queue<message> message_receiver_queue;
std::mutex mu;

void passPlatoon() {
    std::cout << "Platoon passing initiated." << std::endl;
}

void behaviourModuleCar() {
    while (true) {
        if (!message_receiver_queue.empty()) {
            mu.lock();
            message msg = message_receiver_queue.front();
            message_receiver_queue.pop();
            mu.unlock();

            if (msg.messg == "Safe to pass") {
                passPlatoon();
                break; // Exit after processing message
            } else {
                std::cout << "Invalid message received: " << msg.messg << std::endl;
                break; // Exit to prevent infinite loop in test
            }
        }
    }
}

void test_behaviourModuleCar() {
    message test_msg;
    test_msg.messg = "Safe to pass";
    
    mu.lock();
    message_receiver_queue.push(test_msg);
    mu.unlock();

    std::thread behaviour_thread(behaviourModuleCar);
    sleep(1); // Allow time for the thread to process the message

    behaviour_thread.join();
    std::cout << "Test completed successfully." << std::endl;
}

int main() {
    test_behaviourModuleCar();
    return 0;
}
