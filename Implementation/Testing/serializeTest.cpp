#include <iostream>
#include <cstring>
#include <cassert>

struct message {
    int id;
    long timeStamp;
    int receiverPort;
    bool valid;
    std::string messg;

    void serialize(char* buffer) const {
        memcpy(buffer, &id, sizeof(id));
        memcpy(buffer + sizeof(id), &timeStamp, sizeof(timeStamp));
        memcpy(buffer + sizeof(id) + sizeof(timeStamp), &receiverPort, sizeof(receiverPort));
        memcpy(buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort), &valid, sizeof(valid));

        size_t messg_length = messg.length();
        memcpy(buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid), &messg_length, sizeof(size_t));
        memcpy(buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid) + sizeof(size_t), messg.c_str(), messg_length);
    }
};

void test_serialize() {
    message msg;
    msg.id = 42;
    msg.timeStamp = 1700000000;
    msg.receiverPort = 8080;
    msg.valid = true;
    msg.messg = "Hello, world!";

    size_t buffer_size = sizeof(int) + sizeof(long) + sizeof(int) + sizeof(bool) + sizeof(size_t) + msg.messg.length();
    char* buffer = new char[buffer_size];
    msg.serialize(buffer);

    // Validate serialization
    int id;
    long timeStamp;
    int receiverPort;
    bool valid;
    size_t messg_length;
    char messg_buffer[1024];

    memcpy(&id, buffer, sizeof(int));
    memcpy(&timeStamp, buffer + sizeof(int), sizeof(long));
    memcpy(&receiverPort, buffer + sizeof(int) + sizeof(long), sizeof(int));
    memcpy(&valid, buffer + sizeof(int) + sizeof(long) + sizeof(int), sizeof(bool));
    memcpy(&messg_length, buffer + sizeof(int) + sizeof(long) + sizeof(int) + sizeof(bool), sizeof(size_t));
    memcpy(messg_buffer, buffer + sizeof(int) + sizeof(long) + sizeof(int) + sizeof(bool) + sizeof(size_t), messg_length);
    messg_buffer[messg_length] = '\0';

    assert(id == msg.id);
    assert(timeStamp == msg.timeStamp);
    assert(receiverPort == msg.receiverPort);
    assert(valid == msg.valid);
    assert(std::string(messg_buffer) == msg.messg);

    std::cout << "Serialization test passed!" << std::endl;
    delete[] buffer;
}

int main() {
    test_serialize();
    return 0;
}
