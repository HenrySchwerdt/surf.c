#ifndef surf_tcp_buffer_h
#define surf_tcp_buffer_h
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define WINDOW_LENGTH 8000

typedef struct
{
    char *buffer;
    int buffer_size;
    char *overflow;
    int overflow_size;
    int is_last;
    size_t capacity;
    size_t buffer_length;
    size_t overflow_length;
} Buffer;

Buffer *init_buffer()
{
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->buffer = (char *)malloc(WINDOW_LENGTH);
    buffer->buffer_size = WINDOW_LENGTH;
    buffer->overflow = (char *)malloc(WINDOW_LENGTH);
    buffer->overflow_size = WINDOW_LENGTH;
    buffer->capacity = WINDOW_LENGTH;
    buffer->buffer_length = 0;
    buffer->is_last = 0;
    buffer->overflow_length = 0;
    return buffer;
}

// Free the memory allocated for the buffer
void free_buffer(Buffer *buffer)
{
    free(buffer->buffer);
    free(buffer->overflow);
    free(buffer);
}

static int check_for_nl(char *str, ssize_t length)
{
    for (int i = 0; i < length; i++)
    {
        if (str[i] == '\n')
        {
            return i + 1;
        }
    }
    return 0;
}

static int check_for_end(char *str, ssize_t length)
{
    for (int i = 0; i < length - 3; i++)
    {
        if (str[i] == 10 && str[i + 1] == 13 && str[i + 2] == 10 && str[i + 3] == 13) {
            return i + 3;
        }
    }
    return 0;
}

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int read_line(int client_socket, Buffer *buffer)
{
    if (buffer->is_last && !buffer->overflow_length)
    {
        return -1;
    }
    else if (buffer->is_last)
    {
        int overflow_nl = check_for_nl(buffer->overflow, buffer->overflow_length);

        for (int i = 0; i < min(overflow_nl, buffer->is_last); i++)
        {
            buffer->buffer[i] = buffer->overflow[i];
        }
        buffer->buffer_length = overflow_nl;
        for (int i = overflow_nl; i < buffer->overflow_length; i++)
        {
            buffer->overflow[i - overflow_nl] = buffer->overflow[i];
        }
        buffer->overflow_length = buffer->overflow_length - (overflow_nl - 1);
        return overflow_nl;
    }
    free(buffer->buffer);
    buffer->buffer = NULL;
    buffer->buffer_size = WINDOW_LENGTH;
    buffer->buffer_length = 0;
    buffer->buffer = (char *)malloc(WINDOW_LENGTH);

    if (buffer->overflow_length > 0)
    {
        int overflow_nl = check_for_nl(buffer->overflow, buffer->overflow_length);
        if (overflow_nl)
        {
            for (int i = 0; i < overflow_nl; i++)
            {
                buffer->buffer[i] = buffer->overflow[i];
            }
            buffer->buffer_length = overflow_nl;
            for (int i = overflow_nl; i < buffer->overflow_length; i++)
            {
                buffer->overflow[i - overflow_nl] = buffer->overflow[i];
            }
            buffer->overflow_length = buffer->overflow_length - (overflow_nl - 1);
            return overflow_nl;
        }
        else
        {
            for (int i = 0; i < buffer->overflow_length; i++)
            {
                buffer->buffer[i] = buffer->overflow[i];
            }
            buffer->buffer_length = buffer->overflow_length;
            buffer->overflow_length = 0;
        }
    }
    char window[WINDOW_LENGTH];
    int iteration = 0;
    while (1)
    {
        ssize_t bytes_received = recv(client_socket, window, WINDOW_LENGTH, 0);
        if (bytes_received == 0)
        {
            break;
        }
        iteration++;
        buffer->is_last = check_for_end(window, bytes_received);
        int nl_index = check_for_nl(window, bytes_received);
        if (!nl_index && buffer->buffer_length + bytes_received < buffer->buffer_size)
        {
            for (int i = 0; i < bytes_received; i++)
            {
                buffer->buffer[i + buffer->buffer_length] = window[i];
            }
            buffer->buffer_length += bytes_received;
        }
        else if (!nl_index && buffer->buffer_length + bytes_received <= buffer->buffer_size)
        {
            buffer->buffer = (char *)realloc(buffer->buffer, buffer->buffer_size + WINDOW_LENGTH);
            buffer->buffer_size += WINDOW_LENGTH;
            for (int i = 0; i < bytes_received; i++)
            {
                buffer->buffer[i + buffer->buffer_length] = window[i];
            }
            buffer->buffer_length += bytes_received;
        }
        else if (nl_index && buffer->buffer_length + nl_index < buffer->buffer_size)
        {
            for (int i = 0; i < nl_index; i++)
            {
                buffer->buffer[i + buffer->buffer_length] = window[i];
            }
            buffer->buffer_length += nl_index;
            for (int i = nl_index; i < bytes_received; i++)
            {
                buffer->overflow[i - nl_index] = window[i];
            }
            buffer->overflow_length += bytes_received - nl_index;
            break;
        }
        else
        {
            buffer->buffer = (char *)realloc(buffer->buffer, buffer->buffer_size + nl_index);
            buffer->buffer_size += nl_index;
            for (int i = 0; i < nl_index; i++)
            {
                buffer->buffer[i + buffer->buffer_length] = window[i];
            }
            buffer->buffer_length += nl_index;
            buffer->buffer_length += nl_index;
            for (int i = nl_index; i < bytes_received; i++)
            {
                buffer->overflow[i - nl_index] = window[i];
            }
            buffer->overflow_length += bytes_received - nl_index;
            break;
        }
    }

    if (iteration == 0 && buffer->buffer_length == 0)
    {
        return -1;
    }
    return buffer->buffer_length;
}

#endif