#define DEBUG

#include <cstdio>
#include <protocol/StackInterfaces.h>
#include <protocol/Commands.h>
#include <protocol/NetworkLayer.h>
#include "gtest/gtest.h"
#include "protocol/DataStructs.h"

TEST(dataStructs, phy) {
    auto phy = PHYDataStruct();
    EXPECT_EQ(phy.len, 0);
    uint8_t tab[] = {1, 2, 3, 4, 5};
    phy.append(tab, sizeof(tab));
    EXPECT_EQ(phy.len, 5);
    uint8_t val = 6;
    phy.append(val);
    EXPECT_EQ(phy.len, 6);
    uint16_t val2 = 0x0807;
    phy.append(val2);
    EXPECT_EQ(phy.len, 8);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(phy.data[i], i + 1);
    }

    uint8_t tab2[] = {1, 2, 3, 4};
    phy = PHYDataStruct(tab2, sizeof(tab2));
    uint32_t val3 = 0x08070605;
    phy.append(val3);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(phy.data[i], i + 1);
    }
}

TEST(dataStructs, net) {
    auto net = NETDataStruct(0xAA);
    EXPECT_EQ(net.command, 0xAA);
    EXPECT_EQ(net.len, 0);
    uint8_t tab[] = {1, 2, 3, 4};
    net.append(tab, sizeof(tab));
    EXPECT_EQ(net.command, 0xAA);
    EXPECT_EQ(net.len, 4);
    uint8_t val = 5;
    net.append(val);
    EXPECT_EQ(net.command, 0xAA);
    EXPECT_EQ(net.len, 5);
    uint16_t val2 = 0x0706;
    net.append(val2);
    EXPECT_EQ(net.command, 0xAA);
    EXPECT_EQ(net.len, 7);
    for (int i = 0; i < 7; ++i) {
        EXPECT_EQ(net.data[i], i + 1);
    }
    EXPECT_EQ(net.command, 0xAA);

    uint8_t tab2[] = {1, 2, 3};
    auto net2 = NETDataStruct(0xFE, tab2, sizeof(tab2));
    EXPECT_EQ(net2.command, 0xFE);
    EXPECT_EQ(net2.len, 3);
    uint32_t val3 = 0x07060504;
    net2.append(val3);
    EXPECT_EQ(net2.command, 0xFE);
    EXPECT_EQ(net2.len, 7);
    for (int i = 0; i < 7; ++i) {
        EXPECT_EQ(net2.data[i], i + 1);
    }
}


class PHYMock : public PHYInterface {
    std::vector<uint8_t> PHYOut;
public:
    PHYMock() {
        PHYOut.clear();
    }

    virtual void passDown(const PHYDataStruct &data) {
        for (int i = 0; i < data.len; ++i) {
            PHYOut.push_back(data.data[i]);
        }
    }
    std::vector<uint8_t> &out() {
        return PHYOut;
    }

    void mock(const std::vector<uint8_t> vec) {
        PHYDataStruct dataStruct;
        for(auto &x : vec) {
            dataStruct.append(x);
        }
        netInterface->passUp(dataStruct);
    }

    void mock(const std::vector<uint8_t>::iterator begin,  const std::vector<uint8_t>::iterator end) {
        PHYDataStruct dataStruct;
        for(auto it = begin; it != end; ++it) {
            dataStruct.append(*it);
        }
        netInterface->passUp(dataStruct);
    }

    void print() {
        printf("[PHY]out: ");
        for(auto &x : this->out()) {
            printf("%d, ",x);
        }
        printf("\n");
    }

    void compare(std::vector<uint8_t> vec) {
        EXPECT_EQ(vec, this->out());
        out().clear();
    }
};

#define COMPARE(vec) EXPECT_EQ(vec, phy.out()); phy.out().clear();

TEST(PHYMock, 1) {
    PHYMock test;

    PHYDataStruct data;
    uint32_t x = 0x04030201;
    data.append(x);
    test.passDown(data);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(test.out()[i], i + 1);
    }
}

TEST(NetworkLayer, registration) {
    DataDescriptor *tab[] = {
            new TypedDataDescriptor<bool>({0, false}),
            new TypedDataDescriptor<float>({1, true}),
            new TypedDataDescriptor<uint8_t>({2, true}),
            new TypedDataDescriptor<uint16_t>({3, true}),
            new TypedDataDescriptor<uint32_t>({4, false}),
            new TypedDataDescriptor<int8_t>({5, true}),
            new TypedDataDescriptor<int16_t>({6, true}),
            new TypedDataDescriptor<int32_t>({7, false})
    };

    PHYMock phy;
    NetworkLayer net(phy, tab, 8);

    std::vector<uint8_t> ref({255, 0, 0, 0, 0, 0, 0, 0, 255, 1, 1, 0, 0, 0, 1, 1, 255, 2, 2, 0, 0, 0, 2, 1, 255, 3, 3, 0, 0, 0, 3, 1, 255, 4, 4, 0, 0, 0, 4, 0, 255, 5, 5, 0, 0, 0, 5, 1, 255, 6, 6, 0, 0, 0, 6, 1, 255, 7, 7, 0, 0, 0, 7, 0});
    EXPECT_EQ(ref, phy.out());

}


TEST(Stack, testTxd) {
    Command::reset();
    TypedDataDescriptor<bool> d1({0x04030201, false});
    TypedDataDescriptor<float> d2({0x08070605, true});
    TypedDataDescriptor<uint8_t> d3({0x0C0B0A09, true});
    TypedDataDescriptor<uint16_t> d4({0x100F0E0D, true});
    TypedDataDescriptor<uint32_t> d5({0x14131211, false});
    DataDescriptor *tab[] = {
            &d1, &d2, &d3, &d4, &d5
    };

    PHYMock phy;
    NetworkLayer net(phy, tab, 5);

    phy.compare({255, 0, 0x01, 0x02, 0x03, 0x04, 0, 0, 255, 1, 0x05, 0x06, 0x07, 0x08, 1, 1, 255, 2, 0x09, 0x0A, 0x0B, 0x0C, 2, 1, 255, 3, 0x0D, 0x0E, 0x0F, 0x10, 3, 1, 255, 4, 0x11, 0x12, 0x13, 0x14, 4, 0});

    d1.send(false);
    phy.compare({0, 0});
    d1.send(true);
    phy.compare({0, 1});

    for(int i = 0; i < 255; i++) {
        d3.send(i);
        phy.compare({2, i});
    }

    for(int i = 0; i < 1000; i++) {
        uint16_t x = (i >> 7);
        d4.send(x);
        phy.compare({3, x & 0xFF, x >> 8});
    }

    for(int i = 0; i < 1000; i++) {
        uint32_t x = (i*i*i);
        d5.send(x);
        phy.compare({4, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24});
    }
}

//class Receiver {
//public:
//    static std::vector<bool> receivedBool;
//    void static receiveBool(bool x) {
//        receivedBool.push_back(x);
//    }
//
//    static std::vector<float> receivedFloat;
//    void static receiveFloat(float x) {
//        receivedFloat.push_back(x);
//    }
//
//    static std::vector<uint8_t> receivedUint8;
//    void static receiveUint8(uint8_t x) {
//        receivedUint8.push_back(x);
//    }
//
//    static std::vector<uint16_t> receivedUint16;
//    void static receiveUint16(uint16_t x) {
//        receivedUint16.push_back(x);
//    }
//
//    static std::vector<uint32_t> receivedUint32;
//    void static receiveUint32(uint32_t x) {
//        receivedUint32.push_back(x);
//    }
//
//    static std::vector<int8_t> receivedInt8;
//    void static receiveInt8(int8_t x) {
//        receivedInt8.push_back(x);
//    }
//
//    static std::vector<int16_t> receivedInt16;
//    void static receiveInt16(int16_t x) {
//        receivedInt16.push_back(x);
//    }
//
//    static std::vector<int32_t> receivedInt32;
//    void static receiveInt32(int32_t x) {
//        receivedInt32.push_back(x);
//    }
//};
//
//std::vector<bool> Receiver::receivedBool;
//std::vector<float> Receiver::receivedFloat;
//std::vector<uint8_t> Receiver::receivedUint8;
//std::vector<uint16_t> Receiver::receivedUint16;
//std::vector<uint32_t> Receiver::receivedUint32;
//std::vector<int8_t> Receiver::receivedInt8;
//std::vector<int16_t> Receiver::receivedInt16;
//std::vector<int32_t> Receiver::receivedInt32;
//
//TEST(Stack, testRcv) {
//    PHYMockRxTx phy;
//    BoolDataDescriptor d1({0x04030201, false}, Receiver::receiveBool);
//    FloatDataDescriptor d2({0x08070605, true}, Receiver::receiveFloat);
//    Uint8DataDescriptor d3({0x0C0B0A09, true}, Receiver::receiveUint8);
//    Uint16DataDescriptor d4({0x100F0E0D, true}, Receiver::receiveUint16);
//    Uint32DataDescriptor d5({0x14131211, false}, Receiver::receiveUint32);
//    Int8DataDescriptor d6({0x0C0B0A09, true}, Receiver::receiveInt8);
//    Int16DataDescriptor d7({0x100F0E0D, true}, Receiver::receiveInt16);
//    Int32DataDescriptor d8({0x14131211, false}, Receiver::receiveInt32);
//    DataDescriptor d9({0x18171615, false});
//    DataDescriptor *tab[] = {
//            &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9
//    };
//    auto stack = Stack(phy, tab, 9);
//
//    {
//        std::vector<uint8_t> output({255, 0, 1, 2, 3, 4, 1, 0, 255, 1, 5, 6, 7, 8, 2, 1, 255, 2, 9, 10, 11, 12, 3, 1, 255, 3, 13, 14, 15, 16, 4, 1, 255, 4, 17, 18, 19, 20, 5, 0, 255, 5, 9, 10, 11, 12, 6, 1, 255, 6, 13, 14, 15, 16, 7, 1, 255, 7, 17, 18, 19, 20, 8, 0, 255, 8, 21, 22, 23, 24, 0, 0});
//        EXPECT_EQ(output, phy.out());
//        phy.out().clear();
//    }
//
//    for(int i = 0; i < 2; ++i)
//    {
//        bool val = i;
//        d1.send(val);
//
//        std::vector<uint8_t> data_out({0, val});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<bool> data_received({val});
//        EXPECT_EQ(data_received, Receiver::receivedBool);
//        Receiver::receivedBool.clear();
//        phy.out().clear();
//    }
//
//    for(float i = -100000; i < 100000; i += 1001.451) {
//        d2.send(i);
//
//        std::vector<uint8_t> data_out = phy.out();
//        phy.mock(phy.out());
//
//        std::vector<float> data_received({i});
//        EXPECT_EQ(data_received, Receiver::receivedFloat);
//        Receiver::receivedFloat.clear();
//
//        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//
//    for(int i = 0; i < 255; i++) {
//        d3.send((uint8_t)i);
//
//        std::vector<uint8_t> data_out({2, i});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<uint8_t> data_received({i});
//        EXPECT_EQ(data_received, Receiver::receivedUint8);
//        Receiver::receivedUint8.clear();
//
//        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//
//    for(int i = 0; i < 1000; i++) {
//        uint16_t x = (i*i >> 2);
//        d4.send(x);
//
//        std::vector<uint8_t> data_out({3, x & 0xFF, x >> 8});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<uint16_t> data_received({x});
//        EXPECT_EQ(data_received, Receiver::receivedUint16);
//        Receiver::receivedUint16.clear();
//
//        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//
//    for(int i = 0; i < 1000; i++) {
//        uint32_t x = (i*i*i*i+5);
//
//        d5.send(x);
//
//        std::vector<uint8_t> data_out({4, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<uint32_t> data_received({x});
//        EXPECT_EQ(data_received, Receiver::receivedUint32);
//        Receiver::receivedUint32.clear();
//
////        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//
//    for(int i = -150; i < 150; i++) {
//        d6.send((int8_t)i);
//
//        std::vector<uint8_t> data_out({5, i});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<int8_t> data_received({i});
//        EXPECT_EQ(data_received, Receiver::receivedInt8);
//        Receiver::receivedInt8.clear();
//
//        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//
//    for(int i = -1000; i < 1000; i++) {
//        int16_t val = (i*i*i >> 2);
//        uint16_t x = val;
//        d7.send(val);
//
//        std::vector<uint8_t> data_out({6, x & 0xFF, x >> 8});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<int16_t> data_received({val});
//        EXPECT_EQ(data_received, Receiver::receivedInt16);
//        Receiver::receivedInt16.clear();
//
//        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//
//    for(int i = -1000; i < 1000; i++) {
//        int32_t val = (i*i*i*i+5);
//        uint32_t x = val;
//
//        d8.send(val);
//
//        std::vector<uint8_t> data_out({7, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24});
//        EXPECT_EQ(data_out, phy.out());
//
//        phy.mock(phy.out());
//        std::vector<int32_t> data_received({val});
//        EXPECT_EQ(data_received, Receiver::receivedInt32);
//        Receiver::receivedInt32.clear();
//
////        data_out.push_back(ACK_ID);
//        EXPECT_EQ(phy.out(), data_out);
//        phy.out().clear();
//    }
//}
//
//
