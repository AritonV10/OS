#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#define _LITTLE_INDIAN_

/* used for testing individual functions */
/*
#if defined(_TEST_)
    #define 
#else
    
#endif
*/

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

#define FCFS_THREAD (0x01) 

#define SCAN_THREAD (0x02)

#define _THREAD     (0x03)

#define _THREAD2    (0x04)


#define CHAR_TO_DIGIT(c) ((c) & 0x0F)
    

#define IS_DONE(c)            (c == 0x10101010)
#define IS_READY_NEXT_LINE(c) (c == 0x01010101)
    


#define SINT_MAX              (1u << sizeof(int32_t) - 1)
#define UDOUBLE_EXPONENT_MAX  (1u << sizeof(double) - 1)

/******************** TYPE DEFINITIONS **********************************/

typedef int FD;

typedef void(*algo_func)(void*);

typedef uint32_t disk_section;


/* 
   finished the line:       0x01010101
   finished all the lines:  0x10101010
   
   FCFS_THREAD -> signal_A
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
u32_stoi(const char * __restrict);

double
ud_stoi(const char *__restrict);

double
FCFS(const disk_section *, const size_t *);


double
SCAN(const disk_section *, const size_t *);

double
CSCAN(const disk_section *, const uint32_t *);

/************************************************************************/
/************************************************************************/

/* volatile in case it gets cached since it fits into the cache */
/* if its spread among multiple cores */
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


/* const char   [][]threads = { "FCFS_THREAD", "SCAN_THREAD", "_THREAD", "_THREAD2", "C-STAN_THREAd" }; */

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
        
        disk_section test_sections[] = {98, 183, 41, 122, 14, 124, 65, 67};
        size_t size                  = sizeof(test_sections)/sizeof(*test_sections);         
        
        rotation_time_g              = (double) .1;
        
        access_time_g                = (double) .2;
    
        seek_time_g                  = (double) .1;
    
        start_sector_g               = 53;
    
    #endif
    
    /* open file -> get # lines -> alloc the disk sections -> SEEK_START the cursor -> get # of integers -> alloc the arrays
       -> start the threads -> check the bitmap -> realloc the array for the new numbers */
    
    /* printf("%f\n", FCFS(test_sections, &size)); */
    
    printf("%f\n", FCFS(test_sections, &size));
    
    printf("%d\n", u32_stoi("-123"));
    
    
    printf("%f\n", ud_stoi(".42"));
    
    printf("%d", UDOUBLE_EXPONENT_MAX);
    
    /* busy wait until all the threads are done */
    /*
    while(!IS_DONE(latch))
        ;
    
    */
    /* find the smallest time for each algorithm */
    /*
    for(index = 0; index < nlines_g; ++index) {
        
        double *times = algo_time_g[index];
        uint32_t min  = find_min_time(times);
        
        printf("Dataset %d =\nTime: %d\nBy: %s", (i + 1), algoIndex[index][min], threads[index]);
        
    }
    */
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
    exit(1);
}

/* scans either the left or right sections first */
double
SCAN(const disk_section *sections, const size_t *size) {
    
    double time_mili;
    
    int32_t index;
    uint32_t current_position;
    /* TODO: check for underflow */
    
    time_mili = absolute_value(start_sector_g - sections[0]) * seek_time_g + access_time_g;
    
}

double
FCFS(const disk_section *sections, const size_t *size) {
    
    double time_mili;
    
    int32_t index;
    uint32_t current_position;
    int32_t  sum;
    /* TODO: check for underflow */
    
    
    time_mili = absolute_value(start_sector_g - sections[0]) * seek_time_g + access_time_g;
    
    current_position = sections[0];
    sum = absolute_value(start_sector_g - sections[0]);
    // 1 2 3
    
    for(index = 1; index < *size; ++index) {
        time_mili = time_mili + absolute_value(current_position - sections[index]) * seek_time_g + access_time_g + (sections[index + 1] > sections[index] ? 0 : rotation_time_g);   
        sum += absolute_value(current_position - sections[index]);
        current_position = sections[index];
        
    }
    
    printf("%d\n", sum);
    
    return(time_mili);
    
    
}


double
ud_stoi(const char *__restrict number) {
    
    double   buf;
    
    double   mantisa;
    
    uint32_t ten;
    
    ten = 0x0000000A;
    buf = 0.0;
    
    if(*number == 0x2D)
        return 0.0;
    
    
    /* TODO: check for overflow */
    
    for(; *number != '.'; ++number)
        buf = (buf * 10) + CHAR_TO_DIGIT(*number);
    
    /* move it beyond the decimal point */
    ++number;
    
    /* 43.132 */
    /* inefficient way */
    for(; *number != NULL; ++number) {
        
        mantisa = (float)(CHAR_TO_DIGIT(*number))/ten;
        
        ten *= 0x0000000A;
        
        buf += mantisa;
    }
    
    return(buf);
    
}

uint32_t
u32_stoi(const char * __restrict number) {
    
    uint32_t buf;
    
    buf = 0;
    
    if(*number == 0x2D)
        return(0);
        
    for(; *number != 0; ++number) {
        
        /* check the first bit since it'd be a negative value if it overflows */
        if((((buf * 10) + CHAR_TO_DIGIT(*number)) & 0x80000000) == 0) 
            buf = (buf * 10) + CHAR_TO_DIGIT(*number);
        else
            DIE("Fatal", "Integer overflow - '%s' too big", number);
    }
        
    return(buf);
}

void FCFS_thread_start(void *vptr) {
    
    
    int32_t *sections;
    
    uint32_t index;
    uint32_t algo_index;
    
    size_t   size;
    
    
    double   time_mili;
    
    for(index = 0, algo_index = 0; index < nlines_g; ++index) {
        
        sections  = sections_g[index];
        size      = sizeof(sections)/sizeof(*sections);
        time_mili = FCFS(sections, &size);
    
        algo_time_g[algo_index++][FCFS_THREAD] = time_mili; 
        
        
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
        if(array[index] < min) {
            min_index = index;
            min       = array[index];
        }
    }
    
    return(min);
}



