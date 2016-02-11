#define DEBUG

#include <protocol/PresentationLayer.h>
#include <protocol/ApplicationLayer.h>
#include <protocol/Stack.h>
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

    std::vector<uint8_t> &out() {
        return PHYOut;
    }

    virtual void passDown(const PHYDataStruct &data) {
        for (int i = 0; i < data.len; ++i) {
            PHYOut.push_back(data.data[i]);
        }
    }
};

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
            new DataDescriptor({0, false}),
            new DataDescriptor({1, true}),
            new DataDescriptor({2, true}),
            new DataDescriptor({3, true}),
            new DataDescriptor({4, false})
    };
    DataDescriptorsTable desc(tab, 5);
    NETDataStruct data(0xAA);
    uint8_t d = 0x0E;
    data.append(d);
    uint32_t dummy = 0x0D0C0B0A;
    data.append(dummy);

    PHYMock phy;
    NetworkLayer net;
    net.registerLowerLayer(&phy);
    net.registerDataDescriptors(&desc);

#define CHECK() \
    EXPECT_EQ(8, phy.out().size()); \
    for(int i = 0; i < phy.out().size(); ++i) { \
        EXPECT_EQ(out[i], phy.out().at(i)); \
    } \
    phy.out().clear();


    {
        net.passDownRegistration(data);
        uint8_t out[] = {0xAA, 0, 0x0E, 0x0A, 0x0B, 0x0C, 0x0D, 0};CHECK();
    }

    {
        net.passDownRegistration(data);
        uint8_t out[] = {0xAA, 1, 0x0E, 0x0A, 0x0B, 0x0C, 0x0D, 1};CHECK();
    }

    {
        net.passDownRegistration(data);
        uint8_t out[] = {0xAA, 2, 0x0E, 0x0A, 0x0B, 0x0C, 0x0D, 1};CHECK();
    }

    {
        net.passDownRegistration(data);
        uint8_t out[] = {0xAA, 3, 0x0E, 0x0A, 0x0B, 0x0C, 0x0D, 1};CHECK();
    }

    {
        net.passDownRegistration(data);
        uint8_t out[] = {0xAA, 4, 0x0E, 0x0A, 0x0B, 0x0C, 0x0D, 0};CHECK();
    }
#undef CHECK
}


TEST(PresentationLayer, registration_and_sending) {
    DataDescriptor *tab[] = {
            new BoolDataDescriptor({0x04030201, false}),
            new FloatDataDescriptor({0x08070605, true}),
            new Uint8DataDescriptor({0x0C0B0A09, true}),
            new Uint16DataDescriptor({0x100F0E0D, true}),
            new Uint32DataDescriptor({0x14131211, false}),
            new DataDescriptor({0x18171615, false}),
    };
    DataDescriptorsTable desc(tab, 6);

    PHYMock phy;
    NetworkLayer net;
    PresentationLayer pres;
    pres.registerLowerLayer(&net);
    net.registerUpperLayer(&pres);
    net.registerLowerLayer(&phy);
    pres.registerDataDescriptors(&desc);

    pres.passDownRegistration(*(tab[0]));
    {
        uint8_t tab[] = {255, 0, 0x01, 0x02, 0x03, 0x04, 1, 0};
        EXPECT_EQ(sizeof(tab), phy.out().size());
        for (int i = 0; i < phy.out().size(); ++i) {
            EXPECT_EQ(tab[i], phy.out().at(i));
        }
        phy.out().clear();
    }
    pres.passDownRegistration(*(tab[1]));
    {
        uint8_t tab[] = {255, 1, 0x05, 0x06, 0x07, 0x08, 2, 1};
        EXPECT_EQ(sizeof(tab), phy.out().size());
        for (int i = 0; i < phy.out().size(); ++i) {
            EXPECT_EQ(tab[i], phy.out().at(i));
        }
        phy.out().clear();
    }
    pres.passDownRegistration(*tab[2]);
    {
        uint8_t tab[] = {255, 2, 0x09, 0x0A, 0x0B, 0x0C, 3, 1};
        EXPECT_EQ(sizeof(tab), phy.out().size());
        for (int i = 0; i < phy.out().size(); ++i) {
            EXPECT_EQ(tab[i], phy.out().at(i));
        }
        phy.out().clear();
    }
    pres.passDownRegistration(*tab[3]);
    {
        uint8_t tab[] = {255, 3, 0x0D, 0x0E, 0x0F, 0x10, 4, 1};
        EXPECT_EQ(sizeof(tab), phy.out().size());
        for (int i = 0; i < phy.out().size(); ++i) {
            EXPECT_EQ(tab[i], phy.out().at(i));
        }
        phy.out().clear();
    }
    pres.passDownRegistration(*tab[4]);
    {
        uint8_t tab[] = {255, 4, 0x11, 0x12, 0x13, 0x14, 5, 0};
        EXPECT_EQ(sizeof(tab), phy.out().size());
        for (int i = 0; i < phy.out().size(); ++i) {
            EXPECT_EQ(tab[i], phy.out().at(i));
        }
        phy.out().clear();
    }
    pres.passDownRegistration(*tab[5]);
    {
        uint8_t tab[] = {255, 5, 0x15, 0x16, 0x17, 0x18, 0, 0};
        EXPECT_EQ(sizeof(tab), phy.out().size());
        for (int i = 0; i < phy.out().size(); ++i) {
            EXPECT_EQ(tab[i], phy.out().at(i));
        }
        phy.out().clear();
    }

    {
        ValuedDataDescriptor x(*tab[0]);
        x.value.asBool = true;
        pres.passDown(x);
        EXPECT_EQ(2, phy.out().size());
        EXPECT_EQ(0, phy.out().at(0));
        EXPECT_EQ(1, phy.out().at(1));
        phy.out().clear();

        x.value.asBool = false;
        pres.passDown(x);
        EXPECT_EQ(2, phy.out().size());
        EXPECT_EQ(0, phy.out().at(0));
        EXPECT_EQ(0, phy.out().at(1));
        phy.out().clear();
    }


    {
        ValuedDataDescriptor x(*tab[1]);
        x.value.asFloat = true;
        pres.passDown(x);
        EXPECT_EQ(5, phy.out().size());
        EXPECT_EQ(1, phy.out().at(0));
        phy.out().clear();
    }

    {
        ValuedDataDescriptor x(*tab[2]);
        x.value.asUint8 = 0xAE;
        pres.passDown(x);
        EXPECT_EQ(2, phy.out().size());
        EXPECT_EQ(2, phy.out().at(0));
        EXPECT_EQ(0xAE, phy.out().at(1));
        phy.out().clear();
    }


    {
        ValuedDataDescriptor x(*tab[3]);
        x.value.asUint16 = 0x8A31;
        pres.passDown(x);
        EXPECT_EQ(3, phy.out().size());
        EXPECT_EQ(3, phy.out().at(0));
        EXPECT_EQ(0x31, phy.out().at(1));
        EXPECT_EQ(0x8A, phy.out().at(2));
        phy.out().clear();
    }

    {
        ValuedDataDescriptor x(*tab[4]);
        x.value.asUint32 = 0x1A5BC17E;
        pres.passDown(x);
        EXPECT_EQ(5, phy.out().size());
        EXPECT_EQ(4, phy.out().at(0));
        EXPECT_EQ(0x7E, phy.out().at(1));
        EXPECT_EQ(0xC1, phy.out().at(2));
        EXPECT_EQ(0x5B, phy.out().at(3));
        EXPECT_EQ(0x1A, phy.out().at(4));
        phy.out().clear();
    }
}


TEST(ApplicationLayer, test1) {
    ApplicationLayer app;
    PresentationLayer pres;
    NetworkLayer net;
    PHYMock phy;
    app.registerLowerLayer(&pres);
    pres.registerUpperLayer(&app);
    pres.registerLowerLayer(&net);
    net.registerUpperLayer(&pres);
    net.registerLowerLayer(&phy);

    BoolDataDescriptor d1({0x04030201, false});
    FloatDataDescriptor d2({0x08070605, true});
    Uint8DataDescriptor d3({0x0C0B0A09, true});
    Uint16DataDescriptor d4({0x100F0E0D, true});
    Uint32DataDescriptor d5({0x14131211, false});
    DataDescriptor d6({0x18171615, false});
    DataDescriptor *tab[] = {
            &d1, &d2, &d3, &d4, &d5, &d6
    };
    DataDescriptorsTable desc(tab, 6);

    app.registerDataDescriptors(&desc);

#define CHECK()         EXPECT_EQ(sizeof(tab), phy.out().size()); \
    for (int i = 0; i < phy.out().size(); ++i) {                  \
        EXPECT_EQ(tab[i], phy.out().at(i));                       \
    }                                                             \
    phy.out().clear();                                            \

    {
        app.sendSubscriptions();
        uint8_t tab[] = {255, 0, 0x01, 0x02, 0x03, 0x04, 1, 0, 255, 1, 0x05, 0x06, 0x07, 0x08, 2, 1, 255, 2, 0x09, 0x0A, 0x0B, 0x0C, 3, 1, 255, 3, 0x0D, 0x0E, 0x0F, 0x10, 4, 1, 255, 4, 0x11, 0x12, 0x13, 0x14, 5, 0, 255, 5, 0x15, 0x16, 0x17, 0x18, 0, 0};
        CHECK();
    }

    {
        app.send(d1, false);
        uint8_t tab[] = {0, 0};
        CHECK();
    }

    {
        app.send(d1, true);
        uint8_t tab[] = {0, 1};
        CHECK();
    }

    for(int i = 0; i < 255; i++) {
        app.send(d3, (uint8_t)i);
        uint8_t tab[] = {2, i};
        CHECK();
    }

    for(int i = 0; i < 1000; i++) {
        uint16_t x = (i >> 7);
        app.send(d4, x);
        uint8_t tab[] = {3, x & 0xFF, x >> 8};
        CHECK();
    }

    for(int i = 0; i < 1000; i++) {
        uint32_t x = (i*i*i);
        app.send(d5, x);
        uint8_t tab[] = {4, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24};
        CHECK();
    }
}

TEST(Stack, testTxd) {
    PHYMock phy;
    BoolDataDescriptor d1({0x04030201, false});
    FloatDataDescriptor d2({0x08070605, true});
    Uint8DataDescriptor d3({0x0C0B0A09, true});
    Uint16DataDescriptor d4({0x100F0E0D, true});
    Uint32DataDescriptor d5({0x14131211, false});
    DataDescriptor d6({0x18171615, false});
    DataDescriptor *tab[] = {
            &d1, &d2, &d3, &d4, &d5, &d6
    };
    auto x = Stack(phy, tab, 6);

    {
        uint8_t tab[] = {255, 0, 0x01, 0x02, 0x03, 0x04, 1, 0, 255, 1, 0x05, 0x06, 0x07, 0x08, 2, 1, 255, 2, 0x09, 0x0A, 0x0B, 0x0C, 3, 1, 255, 3, 0x0D, 0x0E, 0x0F, 0x10, 4, 1, 255, 4, 0x11, 0x12, 0x13, 0x14, 5, 0, 255, 5, 0x15, 0x16, 0x17, 0x18, 0, 0};
        CHECK();
    }

    {
        d1.send(false);
        uint8_t tab[] = {0, 0};
        CHECK();
    }

    {
        d1.send(true);
        uint8_t tab[] = {0, 1};
        CHECK();
    }

    for(int i = 0; i < 255; i++) {
        d3.send((uint8_t)i);
        uint8_t tab[] = {2, i};
        CHECK();
    }

    for(int i = 0; i < 1000; i++) {
        uint16_t x = (i >> 7);
        d4.send(x);
        uint8_t tab[] = {3, x & 0xFF, x >> 8};
        CHECK();
    }

    for(int i = 0; i < 1000; i++) {
        uint32_t x = (i*i*i);
        d5.send(x);
        uint8_t tab[] = {4, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24};
        CHECK();
    }
}


class PHYMockRxTx : public PHYInterface {
    std::vector<uint8_t> PHYOut;
public:
    PHYMockRxTx() {
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
};

class Receiver {
public:
    static std::vector<bool> receivedBool;
    void static receiveBool(bool x) {
        receivedBool.push_back(x);
    }

    static std::vector<float> receivedFloat;
    void static receiveFloat(float x) {
        receivedFloat.push_back(x);
    }

    static std::vector<uint8_t> receivedUint8;
    void static receiveUint8(uint8_t x) {
        receivedUint8.push_back(x);
    }

    static std::vector<uint16_t> receivedUint16;
    void static receiveUint16(uint16_t x) {
        receivedUint16.push_back(x);
    }

    static std::vector<uint32_t> receivedUint32;
    void static receiveUint32(uint32_t x) {
        receivedUint32.push_back(x);
    }

    static std::vector<int8_t> receivedInt8;
    void static receiveInt8(int8_t x) {
        receivedInt8.push_back(x);
    }

    static std::vector<int16_t> receivedInt16;
    void static receiveInt16(int16_t x) {
        receivedInt16.push_back(x);
    }

    static std::vector<int32_t> receivedInt32;
    void static receiveInt32(int32_t x) {
        receivedInt32.push_back(x);
    }
};

std::vector<bool> Receiver::receivedBool;
std::vector<float> Receiver::receivedFloat;
std::vector<uint8_t> Receiver::receivedUint8;
std::vector<uint16_t> Receiver::receivedUint16;
std::vector<uint32_t> Receiver::receivedUint32;
std::vector<int8_t> Receiver::receivedInt8;
std::vector<int16_t> Receiver::receivedInt16;
std::vector<int32_t> Receiver::receivedInt32;

TEST(Stack, testRcv) {
    PHYMockRxTx phy;
    BoolDataDescriptor d1({0x04030201, false}, Receiver::receiveBool);
    FloatDataDescriptor d2({0x08070605, true}, Receiver::receiveFloat);
    Uint8DataDescriptor d3({0x0C0B0A09, true}, Receiver::receiveUint8);
    Uint16DataDescriptor d4({0x100F0E0D, true}, Receiver::receiveUint16);
    Uint32DataDescriptor d5({0x14131211, false}, Receiver::receiveUint32);
    Int8DataDescriptor d6({0x0C0B0A09, true}, Receiver::receiveInt8);
    Int16DataDescriptor d7({0x100F0E0D, true}, Receiver::receiveInt16);
    Int32DataDescriptor d8({0x14131211, false}, Receiver::receiveInt32);
    DataDescriptor d9({0x18171615, false});
    DataDescriptor *tab[] = {
            &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9
    };
    auto stack = Stack(phy, tab, 9);

    {
        std::vector<uint8_t> output({255, 0, 1, 2, 3, 4, 1, 0, 255, 1, 5, 6, 7, 8, 2, 1, 255, 2, 9, 10, 11, 12, 3, 1, 255, 3, 13, 14, 15, 16, 4, 1, 255, 4, 17, 18, 19, 20, 5, 0, 255, 5, 9, 10, 11, 12, 6, 1, 255, 6, 13, 14, 15, 16, 7, 1, 255, 7, 17, 18, 19, 20, 8, 0, 255, 8, 21, 22, 23, 24, 0, 0});
        EXPECT_EQ(output, phy.out());
        phy.out().clear();
    }

    for(int i = 0; i < 2; ++i)
    {
        bool val = i;
        d1.send(val);

        std::vector<uint8_t> data_out({0, val});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<bool> data_received({val});
        EXPECT_EQ(data_received, Receiver::receivedBool);
        Receiver::receivedBool.clear();
        phy.out().clear();
    }

    for(float i = -100000; i < 100000; i += 1001.451) {
        d2.send(i);

        std::vector<uint8_t> data_out = phy.out();
        phy.mock(phy.out());

        std::vector<float> data_received({i});
        EXPECT_EQ(data_received, Receiver::receivedFloat);
        Receiver::receivedFloat.clear();

        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }

    for(int i = 0; i < 255; i++) {
        d3.send((uint8_t)i);

        std::vector<uint8_t> data_out({2, i});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<uint8_t> data_received({i});
        EXPECT_EQ(data_received, Receiver::receivedUint8);
        Receiver::receivedUint8.clear();

        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }

    for(int i = 0; i < 1000; i++) {
        uint16_t x = (i*i >> 2);
        d4.send(x);

        std::vector<uint8_t> data_out({3, x & 0xFF, x >> 8});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<uint16_t> data_received({x});
        EXPECT_EQ(data_received, Receiver::receivedUint16);
        Receiver::receivedUint16.clear();

        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }

    for(int i = 0; i < 1000; i++) {
        uint32_t x = (i*i*i*i+5);

        d5.send(x);

        std::vector<uint8_t> data_out({4, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<uint32_t> data_received({x});
        EXPECT_EQ(data_received, Receiver::receivedUint32);
        Receiver::receivedUint32.clear();

//        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }

    for(int i = -150; i < 150; i++) {
        d6.send((int8_t)i);

        std::vector<uint8_t> data_out({5, i});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<int8_t> data_received({i});
        EXPECT_EQ(data_received, Receiver::receivedInt8);
        Receiver::receivedInt8.clear();

        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }

    for(int i = -1000; i < 1000; i++) {
        int16_t val = (i*i*i >> 2);
        uint16_t x = val;
        d7.send(val);

        std::vector<uint8_t> data_out({6, x & 0xFF, x >> 8});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<int16_t> data_received({val});
        EXPECT_EQ(data_received, Receiver::receivedInt16);
        Receiver::receivedInt16.clear();

        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }

    for(int i = -1000; i < 1000; i++) {
        int32_t val = (i*i*i*i+5);
        uint32_t x = val;

        d8.send(val);

        std::vector<uint8_t> data_out({7, x & 0xFF, (x & 0xFF00) >> 8, (x & 0xFF0000) >> 16, (x & 0xFF000000) >> 24});
        EXPECT_EQ(data_out, phy.out());

        phy.mock(phy.out());
        std::vector<int32_t> data_received({val});
        EXPECT_EQ(data_received, Receiver::receivedInt32);
        Receiver::receivedInt32.clear();

//        data_out.push_back(ACK_ID);
        EXPECT_EQ(phy.out(), data_out);
        phy.out().clear();
    }
}


