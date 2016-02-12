//
// Created by gregg on 2/12/2016.
//

#ifndef PROTOCOL_DATASTRUCTS_H
#define PROTOCOL_DATASTRUCTS_H

#include <stdint.h>
#include <string.h>

static const uint8_t MAX_FRAME_SIZE = 8;

struct PHYDataStruct {
    uint8_t data[MAX_FRAME_SIZE];
    uint8_t len;

    PHYDataStruct() : len(0) { };

    PHYDataStruct(const uint8_t *const source, uint8_t len) : len(0) {
        this->append(source, len);
    }

    void append(const uint8_t *const source, uint8_t len) {
        memcpy(this->data + this->len, source, len);
        this->len += len;
    }

    template<typename T>
    void append(const T &source) {
        memcpy(this->data + this->len, &source, sizeof(T));
        this->len += sizeof(T);
    }
};

const int MAX_PACKET_SIZE = MAX_FRAME_SIZE - 1;

struct NETDataStruct {
    const uint8_t command;
    uint8_t data[MAX_PACKET_SIZE];
    uint8_t len;

    NETDataStruct(const uint8_t command) : command(command), len(0) {

    }

    NETDataStruct(const uint8_t command, const uint8_t *const data, const uint8_t len) :
            command(command), len(len) {
        memcpy(this->data, data, len);
    }

    template<typename T>
    void append(const T &source) {
        memcpy(this->data + this->len, &source, sizeof(T));
        this->len += sizeof(T);
    }

    void append(const uint8_t *const source, uint8_t len) {
        memcpy(this->data + this->len, source, len);
        this->len += len;
    }
};

union dataTypeUnion {
    uint8_t bytes[7];
    bool asBool;
    uint8_t asUint8;
    uint16_t asUint16;
    uint32_t asUint32;
    uint8_t asInt8;
    uint16_t asInt16;
    uint32_t asInt32;
    float asFloat;
};


#endif //PROTOCOL_DATASTRUCTS_H
