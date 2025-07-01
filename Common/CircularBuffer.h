#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define BUFFER_SIZE 5
typedef struct data_st
{
    char data[100];
}DATA;

typedef struct {
    DATA* buffer;
    int capacity;
    int head;
    int tail;
} circular_buffer;
    

void cb_init(circular_buffer* cb);

void cb_push_back(circular_buffer* cb, DATA data);

DATA cb_pop_front(circular_buffer* cb);

void cb_cleanup(circular_buffer* cb);

void cb_print(circular_buffer* cb);

char* cb_s(circular_buffer* cb);

char* cb_sOne(circular_buffer* cb);

DATA InitData(char* data);