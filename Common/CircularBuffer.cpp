

#define _CRT_SECURE_NO_WARNINGS

#include "CircularBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

CRITICAL_SECTION csProcess;


void cb_init(circular_buffer* cb) {
    InitializeCriticalSectionAndSpinCount(&csProcess, 0x80000400);
    EnterCriticalSection(&csProcess);
    cb->buffer = (DATA*)malloc(BUFFER_SIZE * sizeof(DATA));
    cb->capacity = BUFFER_SIZE;
    cb->head = 0;
    cb->tail = 0;
    LeaveCriticalSection(&csProcess);

}

void cb_push_back(circular_buffer* cb, DATA data) {
    EnterCriticalSection(&csProcess);

    if ((cb->head + 1) % cb->capacity == cb->tail) {
        int newCapacity = cb->capacity * 2;
        DATA* newBuffer = (DATA*)malloc(newCapacity * sizeof(DATA));

        int i = cb->tail;
        int j = 0;
        while (i != cb->head) {
            newBuffer[j] = cb->buffer[i];
            i = (i + 1) % cb->capacity;
            ++j;
        }

        cb->tail = 0;
        cb->head = j;
        cb->capacity = newCapacity;

        free(cb->buffer);

        cb->buffer = newBuffer;
    }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % cb->capacity;

    LeaveCriticalSection(&csProcess);
}

DATA cb_pop_front(circular_buffer* cb) {
    EnterCriticalSection(&csProcess);

    DATA returnData = { {0} };

    if (cb->head != cb->tail) {
        returnData = cb->buffer[cb->tail];
        cb->tail = (cb->tail + 1) % cb->capacity;
    }

    LeaveCriticalSection(&csProcess);

    return returnData;
}

void cb_cleanup(circular_buffer* cb) {
    EnterCriticalSection(&csProcess);
    free(cb->buffer);
    cb->buffer = NULL;
    cb->capacity = 0;
    cb->head = 0;
    cb->tail = 0;
    LeaveCriticalSection(&csProcess);
}

void cb_print(circular_buffer* cb) {
    EnterCriticalSection(&csProcess);

    int i = cb->tail;
    while (i != cb->head) {
        printf("%d: %s\n", i, cb->buffer[i].data);
        i = (i + 1) % cb->capacity;
    }

    LeaveCriticalSection(&csProcess);
}

char* cb_s(circular_buffer* cb) {
    EnterCriticalSection(&csProcess);

    size_t total_length = 0;
    int i = cb->tail;
    while (i != cb->head) {
        total_length += strlen(cb->buffer[i].data) + 1;
        i = (i + 1) % cb->capacity;
    }

    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    i = cb->tail;
    while (i != cb->head) {
        strcpy(result + offset, cb->buffer[i].data);
        offset += strlen(cb->buffer[i].data);
        result[offset++] = '\n';
        i = (i + 1) % cb->capacity;
    }

    result[offset] = '\0';

    LeaveCriticalSection(&csProcess);

    return result;
}

char* cb_sOne(circular_buffer* cb) {
    EnterCriticalSection(&csProcess);

    if (cb->tail == cb->head) {
        LeaveCriticalSection(&csProcess);
        return NULL;
    }

    char* result = _strdup(cb->buffer[(cb->head - 1 + cb->capacity) % cb->capacity].data);
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    LeaveCriticalSection(&csProcess);

    return result;
}

DATA InitData(char* data)
{

    DATA d;
    strcpy(d.data, data);
    return d;

}
