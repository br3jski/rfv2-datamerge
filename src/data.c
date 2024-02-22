#include "data.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* data;
    size_t capacity;
    size_t size;
    pthread_mutex_t lock;
} DynamicBuffer;

DynamicBuffer mergedDataBuffer;

void dynamic_buffer_init(DynamicBuffer* buffer) {
    buffer->data = malloc(1);  // Initial allocation
    buffer->capacity = 1;  // Start small to demonstrate dynamic resizing
    buffer->size = 0;
    pthread_mutex_init(&buffer->lock, NULL);
}

void dynamic_buffer_append(DynamicBuffer* buffer, const char* data, size_t data_size) {
    pthread_mutex_lock(&buffer->lock);
    if (buffer->size + data_size > buffer->capacity) {
        size_t new_capacity = buffer->capacity;
        while (new_capacity < buffer->size + data_size)
            new_capacity *= 2;
        char* new_data = realloc(buffer->data, new_capacity);
        if (!new_data) {
            perror("Failed to realloc");
            pthread_mutex_unlock(&buffer->lock);
            return;
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    memcpy(buffer->data + buffer->size, data, data_size);
    buffer->size += data_size;
    pthread_mutex_unlock(&buffer->lock);
}

void process_data(char *data, size_t data_size) {
    dynamic_buffer_append(&mergedDataBuffer, data, data_size);
}

void init_merged_data_buffer() {
    dynamic_buffer_init(&mergedDataBuffer);
}

void cleanup_merged_data_buffer() {
    pthread_mutex_lock(&mergedDataBuffer.lock);
    free(mergedDataBuffer.data);
    pthread_mutex_unlock(&mergedDataBuffer.lock);
    pthread_mutex_destroy(&mergedDataBuffer.lock);
}
