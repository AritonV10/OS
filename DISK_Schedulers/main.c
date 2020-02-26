#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#define _LITTLE_INDIAN_


#define NARGUMENTS  (0x05)


/* TODO: define for clang and ms */
/* TODO: do for C-SCAN and SCAN */

#if defined(__GNUC__)
    #define NORET __attribute__((noreturn))
#else
    #error Compiler not supported
#endif

#define __DEBUG__


#define LOG(type, fmt, ...) do { \
        printf("[" type "]: " fmt, __VA_ARGS__); \
    }while(0);
    
#define DIE(type, fmt, ...) do { \
        LOG(type, fmt, __VA_ARGS__) \
        exit(1); \
    }while(0);


#define SIZE(c) (sizeof(c)/sizeof(*c))

#define NTHREADS    (0x4)

#define FIFO_THREAD (0x01) 

#define SCAN_THREAD (0x02)

#define _THREAD     (0x03)

#define _THREAD2    (0x04)


#define IS_DONE(c)            (c == 0x10101010)
#define IS_READY_NEXT_LINE(c) (c == 0x01010101)
    
    

#define SINT_MAX              (1u << sizeof(int32_t) - 1)


/******************** TYPE DEFINITIONS **********************************/

typedef int FD;

typedef void(*algo_func)(void*);

typedef uint32_t disk_section;


/* 
   finished the line:       0x01010101
   finished all the lines:  0x10101010
   
   FIFO_THREAD -> signal_A
   SCAN_THREAD -> signal_B
   _THREAD     -> signal_C
   _THREAD2    -> signal_D
   
 */
 
typedef union _semaphore {
    
    uint32_t signals;
    
    #if defined(_LITTLE_INDIAN_)    
        
        struct {
            
          uint8_t signal_A;
          
          uint8_t signal_B;
          
          uint8_t signal_C;
          
          uint8_t signal_D;
        
        };
    
    #else 
    
        struct {
            
          uint8_t signal_D;
          
          uint8_t signal_C;
          
          uint8_t signal_B;
          
          uint8_t signal_A;
        
        };
    
    #endif
    
} semaphore_t;


/******************** FUNCTION DECLARATIONS *****************************/

uint32_t
find_min_time(const double *);

uint32_t
absolute_value(const int32_t);

void
die(const char *, const char *, const char *, ...) NORET;

uint32_t
stoi(const char * __restrict);

double
FIFO(const disk_section *, const uint32_t *);


/************************************************************************/
/************************************************************************/

/* volatile in case it gets cached since it fits into the cache */
volatile semaphore_t semaphores = {(0x00000000)};

/* wait for all the threads to finish before continuing */
int32_t volatile latch;


/********** DISK SPEC **********/

static double  access_time_g;
static double  rotation_time_g;
static double  seek_time_g;

static int32_t start_sector_g;

static uint8_t direction_g;

/******************************/

static uint32_t nlines_g;


const char   [][]threads = { "FIFO_THREAD", "SCAN_THREAD", "_THREAD", "_THREAD2", "C-STAN_THREAd" };
disk_section **sections_g;
double       **algo_time_g;


/* 
    Arg list:
        1. Seek time
        2. Rotation time
        3. Direction
        4. Start position
        5. Access time
        ...
*/

int main(int argc, char **argv) {
    
    uint32_t index;
    
    #if defined(__DEBUG__)
        
        disk_section test_sections[] = {12, 6, 20, 30, 100, 150, 9};
        size_t size                  = sizeof(test_sections)/sizeof(*test_sections);         
        
        rotation_time_g              = (double) .2;
        
        access_time_g                = (double) .2;
    
        seek_time_g                  = (double) .1;
    
        start_sector_g               = 15;
    
    #endif
    
    /* open file -> get # lines -> alloc the disk sections -> SEEK_START the cursor -> get # of integers -> alloc the arrays
       -> start the threads -> check the bitmap -> realloc the array for the new numbers */
    
    /* printf("%f\n", FIFO(test_sections, &size)); */
    
    printf("%f\n", FIFO(test_sections, &size));
    
    
    /* busy wait until all the threads are done */
    while(!IS_DONE(latch))
        ;
        
    /* find the smallest time for each algorithm */
    for(index = 0; index < nlines_g; ++index) {
        
        double *times = algo_time_g[index];
        uint32_t min  = find_min_time(times);
        
        printf("Dataset %d =\nTime: %d\nBy: %s", (i + 1), algoIndex[index][min], threads[index]);
        
    }
    
    return 0;
}

/******************** FUNCTION DEFINITIONS ******************************/

/* 1100 -> 0100 -. 0011 -> 0111, -8, var & (1ul << sizeof(int32_t) - 1), ~!sign * var */



/* branchless abs */
uint32_t
absolute_value(const int32_t value) {
    
    uint32_t mask = (value >> (sizeof(int32_t) - 1));
    
    return((value + mask) ^ mask);
}

void
die(const char *type, const char *fmt, const char *arg, ...) {
    LOG("Info", "%s %d", arg);
}

/* scans either the left or right sections first */
double
SCAN(const disk_section *sections, const size_t *size) {
    
    double time_mili;
    
    int32_t index;
    uint32_t current_position;
    /* TODO: check for underflow */
    
    time_mili = absolute_value(start_sector_g - sections_g[0]) * seek_time_g + access_time_g;
    
}

double
FIFO(const disk_section *sections, const size_t *size) {
    
    double time_mili;
    
    int32_t index;
    uint32_t current_position;
    /* TODO: check for underflow */
    
    time_mili = absolute_value(start_sector_g - sections_g[0]) * seek_time_g + access_time_g;
    
    current_position = sections_g[0];
    
    
    
    for(index = 1; index < *size; ++index) {
        time_mili = time_mili + absolute_value(current_position - sections_g[index]) * seek_time_g + access_time_g + (sections_g[index + 1] > sections_g[index] ? 0 : rotation_time_g);   
        current_position = sections_g[index];
    }
    
    return(time_mili);
    
    
}

uint32_t
stoi(const char * __restrict number) {
    
    #define CHAR_TO_DIGIT(c) ((c) & 0x0F)
    
    uint32_t buf;
    
    buf = 0;
    
    for(; *number != 0; ++number) {
        if(((buf * 10) + CHAR_TO_DIGIT(*number)) <= SINT_MAX) 
            buf = (buf * 10) + CHAR_TO_DIGIT(*number);
            
    }
        
    return(buf);
}

void FIFO_thread_start(void *vptr) {
    
    
    int32_t *sections;
    
    uint32_t index;
    uint32_t algoindex;
    size_t   size;
    
    
    double   time_mili;
    
    for(index = 0, algoindex = 0; index < nlines; ++index) {
        
        sections  = disk_section[index];
        size      = sizeof(sections)/sizeof(*sections);
        time_mili = FIFO(sections, &size);
    
        algo_time[algoIndex++][FIFO_THREAD] = time_mili; 
        
        
        /* signal that it has finished the line */
        semaphores.signal_A = 0x01;
    
        /* busy wait until the other threads have finished or there are no more lines */
        while(IS_READY_NEXT_LINE(latch))
            ;
    }
    
    /* signal that it has finished all the lines */
    semaphores.signal_A = 0x10;
}


uint32_t
find_min_time(const double *array) {
    
    
    uint32_t min_index; 
    size_t   index;
    size_t   size;
    
    double   min;
    
    min = (999.0);
    
    for(index = 0; index < NTHREADS; ++index) {
        if(array[i] < min) {
            min_index = i;
            min       = array[i];
        }
    }
    
    return(min)
};


