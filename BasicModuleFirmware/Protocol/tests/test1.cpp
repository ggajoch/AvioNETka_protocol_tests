#include <protocol/DataStructs.h>
#include "gtest/gtest.h"

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
    for(int i = 0; i < 8; ++i) {
        EXPECT_EQ(phy.data[i], i+1);
    }

    uint8_t tab2[] = {1, 2, 3, 4};
    phy = PHYDataStruct(tab2, sizeof(tab2));
    uint32_t val3 = 0x08070605;
    phy.append(val3);
    for(int i = 0; i < 8; ++i) {
        EXPECT_EQ(phy.data[i], i+1);
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
    for(int i = 0; i < 7; ++i) {
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
    for(int i = 0; i < 7; ++i) {
        EXPECT_EQ(net2.data[i], i + 1);
    }
}
