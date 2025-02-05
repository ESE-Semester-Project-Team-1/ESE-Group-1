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

    void deserialize(const char* buffer) {
        memcpy(&id, buffer, sizeof(id));
        memcpy(&timeStamp, buffer + sizeof(id), sizeof(timeStamp));
        memcpy(&receiverPort, buffer + sizeof(id) + sizeof(timeStamp), sizeof(receiverPort));
        memcpy(&valid, buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort), sizeof(valid));

        size_t messg_length;
        memcpy(&messg_length, buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid), sizeof(size_t));

        char messg_buffer[messg_length + 1];
        memcpy(messg_buffer, buffer + sizeof(id) + sizeof(timeStamp) + sizeof(receiverPort) + sizeof(valid) + sizeof(size_t), messg_length);
        messg_buffer[messg_length] = '\0';
        messg = std::string(messg_buffer);
    }
};

void test_deserialize() {
    message msg;
    msg.id = 42;
    msg.timeStamp = 1700000000;
    msg.receiverPort = 8080;
    msg.valid = true;
    msg.messg = "Hello, world!";

    size_t buffer_size = sizeof(int) + sizeof(long) + sizeof(int) + sizeof(bool) + sizeof(size_t) + msg.messg.length();
    char* buffer = new char[buffer_size];
    msg.serialize(buffer);

    // Deserialize and validate
    message msg_deserialized;
    msg_deserialized.deserialize(buffer);
    assert(msg_deserialized.id == msg.id);
    assert(msg_deserialized.timeStamp == msg.timeStamp);
    assert(msg_deserialized.receiverPort == msg.receiverPort);
    assert(msg_deserialized.valid == msg.valid);
    assert(msg_deserialized.messg == msg.messg);

    std::cout << "Deserialization test passed!" << std::endl;
    delete[] buffer;
}

int main() {
    test_deserialize();
    return 0;
}
