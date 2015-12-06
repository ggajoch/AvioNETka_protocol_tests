//
// Created by gregg on 11/26/2015.
//

#ifndef PROTOCOL_USEFUL_H
#define PROTOCOL_USEFUL_H

void print_byte_table(const void * const data, uint8_t len) {
    const uint8_t * table = reinterpret_cast<const uint8_t *>(data);
    for (int i = 0; i < len; ++i) {
        printf("%#x ", table[i]);
    }
    printf("\n");
}

#endif //PROTOCOL_USEFUL_H
