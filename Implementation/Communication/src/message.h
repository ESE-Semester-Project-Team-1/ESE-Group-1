#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <iostream>
#include <cstring>

class message {
public:
    // Member variables
    std::string messg;
    int id;
    int timeStamp;
    int receiverPort;
    int valid;

    // Default constructor
    message();

    // Parameterized constructor
    message(std::string new_msg, int new_id, int new_time, int new_receiver_port, int valid_new);

    // Serialize object into a byte array
    void serialize(char* buffer) const;

    // Deserialize byte array into object
    void deserialize(const char* buffer);

    // Print the message for debugging
    void print() const;
};

#endif // MESSAGE_H