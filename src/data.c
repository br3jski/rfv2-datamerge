#include "data.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// A simple dynamic buffer implementation
typedef struct {
    char* data;       // Pointer to the buffer's data
    size_t capacity;  // Total capacity of the buffer
    size_t size;      // Current size of the data in the buffer
    pthread_mutex_t lock; // Mutex for thread-safe access
} DynamicBuffer;

// Initialize a dynamic buffer
void dynamic_buffer_init(DynamicBuffer* buffer) {
    buffer->data = NULL;
    buffer->capacity = 0;
    buffer->size = 0;
    pthread_mutex_init(&buffer->lock, NULL);
}

// Resize the dynamic buffer
void dynamic_buffer_resize(DynamicBuffer* buffer, size_t new_capacity) {
    pthread_mutex_lock(&buffer->lock);
    char* new_data = realloc(buffer->data, new_capacity);
    if (new_data) {
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    pthread_mutex_unlock(&buffer->lock);
}

// Append data to the dynamic buffer
void dynamic_buffer_append(DynamicBuffer* buffer, const char* data, size_t data_size) {
    pthread_mutex_lock(&buffer->lock);
    if (buffer->size + data_size > buffer->capacity) {
        // Resize needed
        size_t new_capacity = (buffer->capacity == 0) ? data_size : buffer->capacity * 2;
        while (new_capacity < buffer->size + data_size) new_capacity *= 2;
        dynamic_buffer_resize(buffer, new_capacity);
    }
    memcpy(buffer->data + buffer->size, data, data_size);
    buffer->size += data_size;
    pthread_mutex_unlock(&buffer->lock);
}

// Free the dynamic buffer
void dynamic_buffer_free(DynamicBuffer* buffer) {
    pthread_mutex_lock(&buffer->lock);
    free(buffer->data);
    buffer->data = NULL;
    buffer->capacity = 0;
    buffer->size = 0;
    pthread_mutex_unlock(&buffer->lock);
    pthread_mutex_destroy(&buffer->lock);
}

// Global instance of the buffer
DynamicBuffer mergedDataBuffer;

// Initialize the global buffer
void init_merged_data_buffer() {
    dynamic_buffer_init(&mergedDataBuffer);
}

// Function to process and merge incoming data
void process_data(char *data, size_t data_size) {
    dynamic_buffer_append(&mergedDataBuffer, data, data_size);
}

// Call this function at program termination to clean up
void cleanup_merged_data_buffer() {
    dynamic_buffer_free(&mergedDataBuffer);
}
