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
    pres.registerDataDescriptors(&desc);
    net.registerUpperLayer(&pres);
    net.registerLowerLayer(&phy);

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


TEST(Stack, test1) {
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
    DataDescriptorsTable desc(tab, 6);
    auto x = Stack(phy, &desc);
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
