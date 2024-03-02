#include <stdlib.h>
#include <string.h>

typedef struct circular_buffer
{
    void *buffer;
    void *buffer_end;
    size_t capacity;
    size_t count;
    size_t sz;
    void *head;
    void *tail;
} circular_buffer;


int cb_init(circular_buffer *, size_t , size_t );

int cb_push_back(circular_buffer *, const void *);
int cb_pop_front(circular_buffer *, void *);
int cb_peak_front(circular_buffer *, void *);


void cb_free(circular_buffer *);
