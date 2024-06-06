#include "Message.h"


Message *createMessage() {
    srand(time(NULL));
    size_t size;
    do {
        size = rand() % 255;
    } while (size == 0);
    size = ((size + 3) / 4) * 4;

    Message *message = (Message *)malloc(sizeof(Message));

    message->size = size;

    for (size_t i = 0; i < message->size; i++) {
        int upperCase = rand() % 2;
        if (upperCase) {
            message->data[i] = 'A' + rand() % 26;
        } else {
            message->data[i] = 'a' + rand() % 26;
        }
        message->hash = (message->hash * 17) + message->data[i];
    }

    message->type = 0;
    return message;
}

void printMessage(Message *message) {
    printf("_______________MESSAGE_______________\n");
    printf("Type: %u\n", message->type);
    printf("Hash: %u\n", message->hash);
    printf("Size: %u\n", message->size);
    printf("Data:\n");
    for (size_t i = 0; i < message->size; i++) {
        printf("%c", message->data[i]);
        if(i % 37 == 0 && i != 0) {
            printf("\n");
        }
    }
    printf("\n_____________________________________\n");
}
