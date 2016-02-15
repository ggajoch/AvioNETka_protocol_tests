//
// Created by gregg on 2/13/2016.
//

#ifndef PROTOCOL_STACKERROR_H
#define PROTOCOL_STACKERROR_H


enum StackError {
    STACK_OK = 0,
    STACK_TIMEOUT = 1,
    STACK_NO_CONNECTION = 2,
    STACK_PHY_ERROR = 3,
    STACK_SUBSCRIBE = 4
};
#endif //PROTOCOL_STACKERROR_H
