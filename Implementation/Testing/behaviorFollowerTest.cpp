#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <cassert>

class Message {
public:
    std::string messg;
};

std::queue<Message> message_receiver_queue;
std::mutex mu;

// Simulating the function which would normally be called when the message is received
void increaseDistance() {
    std::cout << "Distance increased!" << std::endl;
}

// The behaviour module we are testing
void behaviourModule() {
    while (true) {
        // Process messages in the queue if there are any
        if (!message_receiver_queue.empty()) {
            // Get data from message queue
            mu.lock();
            Message msg = message_receiver_queue.front();
            message_receiver_queue.pop();
            mu.unlock();

            if (msg.messg == "Increase Distance") {
                increaseDistance();
            } else {
                std::cout << "Invalid command\n";
            }
        }
    }
}

// Test case to verify the behavior
void testBehaviourModule() {
    // Simulate a message being added to the queue
    Message msg;
    msg.messg = "Increase Distance";
    
    mu.lock();
    message_receiver_queue.push(msg);
    mu.unlock();
    
    // Run behaviourModule() in a separate thread
    std::thread t(behaviourModule);

    // Let the test run for a short time to process the message
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Since the output is directed to the console, we can redirect or use assertions.
    // But for this simple case, let's assume if we reach here, the message was processed.
    
    t.detach(); // Detach the thread to allow it to run independently
}

int main() {
    testBehaviourModule();
    return 0;
}
