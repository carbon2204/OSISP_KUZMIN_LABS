#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[256];
} Message;

Message *createMessage(void);
void printMessage(Message *message);
