//
// Created by rexina on 02.12.2015.
//

#ifndef PROTOCOL_DATASTRUCTS_H
#define PROTOCOL_DATASTRUCTS_H


#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const uint8_t MAX_FRAME_SIZE = 8;

struct PHYDataStruct {
    uint8_t data[MAX_FRAME_SIZE];
    uint8_t len;
    PHYDataStruct() : len(0) {};
    PHYDataStruct(const uint8_t * const source, uint8_t len) : len(0) {
        this->append(source, len);
    }
    void append(const uint8_t * const source, uint8_t len) {
        memcpy(this->data + this->len, source, len);
        this->len += len;
    }

    template<typename T>
    void append(const T & source) {
        memcpy(this->data + this->len, &source, sizeof(T));
        this->len += sizeof(T);
    }
};

const int MAX_PACKET_SIZE = MAX_FRAME_SIZE - 1;

struct NetworkDataStruct {
    const uint8_t id;
    uint8_t data[MAX_PACKET_SIZE];
    uint8_t len;
    NetworkDataStruct(const uint8_t id) : id(id), len(0) {

    }

    NetworkDataStruct(const uint8_t id, const uint8_t * const data, const uint8_t len) :
            id(id), len(len) {
        memcpy(this->data, data, len);
    }

    template<typename T>
    void append(const T & source) {
        memcpy(this->data + this->len, &source, sizeof(T));
        this->len += sizeof(T);
    }

    void append(const uint8_t * const source, uint8_t len) {
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
    float asFloat;
};

class DataDescriptor {
public:
    uint8_t id;
    uint32_t fsxId;
    bool ack;
    bool rxEnabled = true;

	typedef void type;

    DataDescriptor(uint8_t FSXid, bool ack) {
        this->fsxId = FSXid;
        this->ack = ack;
    }

    virtual void callback(dataTypeUnion data) {
    }
};

template<typename T>
class TypedDataDescriptor : public DataDescriptor {
	void (*callbackFunction)(T);
	
public:

	typedef T type;

	static int const length = sizeof(type);

    TypedDataDescriptor(DataDescriptor x) :
            DataDescriptor(x), callbackFunction(nullptr) {
		this->rxEnabled = false;
	}
	
    TypedDataDescriptor(DataDescriptor x, void (*callbackFunction)(type)) :
            DataDescriptor(x),  callbackFunction(callbackFunction) {
		this->rxEnabled = true;
	}

	void call(type value) {
		if(callbackFunction != nullptr) {
			callbackFunction(value);
		} else {
            printf("no callback for data id = %d!\n", this->id);
        }
	}

	void callback(dataTypeUnion);

	dataTypeUnion pack(const type value) const;
};


#define TYPE_SPEC(type, asType, name) 	               	 	        \
	template<>	    	            						        \
	void TypedDataDescriptor<type>::callback(dataTypeUnion x) {		\
		call(x.asType);		                                	    \
	}		                                                    	\
	template<>		                                                \
	dataTypeUnion TypedDataDescriptor<type>::pack(const type value) const {		\
		dataTypeUnion box;			                        		\
		box.asType = value;		                                	\
		return box;		                                			\
	}		                                                		\
	typedef TypedDataDescriptor<type> name ## DataDescriptor;


TYPE_SPEC(bool, asBool, Bool)
TYPE_SPEC(float, asFloat, Float)
TYPE_SPEC(uint8_t, asUint8, Uint8)
TYPE_SPEC(uint16_t, asUint16, Uint16)
TYPE_SPEC(uint32_t, asUint32, Uint32)

#endif //PROTOCOL_DATASTRUCTS_H
