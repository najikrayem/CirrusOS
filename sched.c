#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#if defined(_MSC_VER)
    // MS rage...
    #include <intrin.h>
#endif

#define MAX_NUM_THREADS 16

typedef struct {
    uint8_t priority :    4;     // 16 Possible priorities
    uint8_t before:       4;     // its a doubly linked list
    uint8_t next :        4;     // Index of the node that normally would run after this one.
    bool enqueued :       1;
    bool rr       :       1;
} thread_node_t;

typedef struct {
    uint8_t head_idx :    4;
    uint8_t tail_idx :    4;
} prior_head_tail;


typedef struct {
    uint32_t hw_stack_frame[8];
} hw_stack_dump_t;


// This the main structure of the scheduler, it hosts everything, and is used to measure the RAM footprint of the kernel
typedef struct {
    // Location in memory where the thread nodes are stored
    thread_node_t sched_arr[MAX_NUM_THREADS];

    // if a bit is one, then the corresponding idx in sched_arr is available (unallocated)
    uint16_t sched_arr_bitmap;

    // priority heads and tails
    prior_head_tail heads_tails[MAX_NUM_THREADS];

    // core dumps
    //hw_stack_dump_t core_dumps[2];
} sched_mem_t;


static sched_mem_t sched;

// Toggles bit in the sched_arr_bitmap at idx
static inline void toggle_bitmap_bit(uint8_t idx){
    // THIS IS DANGEROUS, MAKE SURE TO ADD RANGE CHECK IF NECESSARY
    uint16_t mask = (uint16_t)1U << idx;
    sched.sched_arr_bitmap ^= mask;
};

// Returns an index of an available spot, or -1 if no available spots.
static inline int8_t alloc_thread_idx(void) {

    int8_t id = -1;

    #if defined(_MSC_VER)
        // MS rage...
        unsigned long idx; // WTF MS
    #endif


    // ENTER critical

        if (sched.sched_arr_bitmap == 0){
            // EXIT Critical
            return id;
        }

        #if defined(_MSC_VER)
            // MS rage...
            _BitScanForward(&idx, sched.sched_arr_bitmap); // WTF MS
            id = (int8_t)idx;
        #else
            // TODO ensure this works well with MCU, might need to assembly
            id = (int8_t)__builtin_ctz(sched_arr_bitmap);
        #endif

        toggle_bitmap_bit(id);
    
    // EXIT Critical
        
    return id;

};

// creates a new thread but does NOT enqueue it. returns its id if successfull, otherwise, returns -1.
int new_thread(uint8_t priority){
    int8_t idx = alloc_thread_idx();

    if (idx != -1){
        //TODO maybe zero out other fields too?
        sched.sched_arr[idx].priority = priority;
    }

    return idx;
}

// int enqueue_thread



int main(){

    // init sched
    sched.sched_arr_bitmap = 0xFFFF;

    printf("sched_mem_t RAM footprint: %zu\n", sizeof(sched));

    printf("Initial bitmap is %X\n", sched.sched_arr_bitmap);

    // int8_t idx;
    // for (uint8_t i = 0; i < 17; i++){
    //     idx = alloc_thread_idx();
    //     printf("Allocated IDX: %hhd, current bitmap: %X\n", idx, sched_arr_bitmap);
    // }

    // toggle_bitmap_bit(0);
    // toggle_bitmap_bit(1);
    // idx = alloc_thread_idx();
    // printf("Allocated IDX: %hhd, current bitmap: %X\n", idx, sched_arr_bitmap);

    return 0;   
}


