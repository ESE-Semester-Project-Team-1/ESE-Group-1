#include "message.h"

// Default constructor
message::message() : messg(""), id(0), timeStamp(0), receiverPort(0), valid(0) {}

// Parameterized constructor
message::message(std::string new_msg, int new_id, int new_masa, int new_receiver_port, int valid_new) {
    messg = new_msg;
    id = new_id;
    timeStamp = new_masa;
    receiverPort = new_receiver_port;
    valid = valid_new;
}

// Serialize object into a byte array
void message::serialize(char* buffer) const {
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
void message::deserialize(const char* buffer) {
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
void message::print() const {
    std::cout << "Message: " << messg << ", ID: " << id << ", Timestamp: " << timeStamp
              << ", ReceiverPort: " << receiverPort << ", Valid: " << valid << std::endl;
}