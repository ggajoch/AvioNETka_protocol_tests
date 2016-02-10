#include <protocol/PresentationLayer.h>
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


#define DEBUG

#include "../protocol/NetworkLayer.h"


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
    DataDescriptor tab[] = {
            DataDescriptor({0, false}),
            DataDescriptor({1, true}),
            DataDescriptor({2, true}),
            DataDescriptor({3, true}),
            DataDescriptor({4, false})
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
}

TEST(PresentationLayer, first) {
//    DataDescriptor tab[] = {
//            DataDescriptor({0, false}),
//            DataDescriptor({1, true}),
//            DataDescriptor({2, true}),
//            DataDescriptor({3, true}),
//            DataDescriptor({4, false})
//    };
//    DataDescriptorsTable desc(tab, 5);
//
//    PHYMock phy;
//    NetworkLayer net;
//    PresentationLayer pres;
//    pres.registerLowerLayer(&net);
//    pres.registerDataDescriptors(&desc);
//    net.registerUpperLayer(&pres);
//    net.registerLowerLayer(&phy);
//
//    pres.passDownRegistration(tab[0]);
}
