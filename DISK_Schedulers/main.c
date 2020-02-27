#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


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
    

#define IS_DONE(c)            (c == DONE)
#define IS_READY_NEXT_LINE(c) (c == LINE)
    

#define CHAR_BIT 8

#define SINT_MAX              (1u << (sizeof(int32_t) * CHAR_BIT - 1))

#define UDOUBLE_EXPONENT_MAX  (1u << (sizeof(double) * CHAR_BIT - 1))



#define BOILER_PLATE \
    double time_mili; \
    int32_t index; \
    int32_t  head_movement; \
    uint32_t current_position; \
    task_details_t execution_details;
    
/******************** TYPE DEFINITIONS **********************************/

typedef int FD;

typedef void(*algo_func)(void*);

typedef uint32_t disk_section;


typedef enum _task_status {
    
    /* if the threads are ready for the next line */
    LINE = 0x01010101,
    
    /* if the threads are done computing all the lines */
    DONE = 0x10101010
    
} task_status_t;

typedef struct _task_details {
    
    double   time_mili;
    
    uint32_t head_movement;
    
} task_details_t;

/* 
   Mappings:
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


typedef struct _bitset {
    
    uint8_t *array;
    
} bitset_t;


/******************** FUNCTION DECLARATIONS *****************************/


void *
mmalloc(size_t);

bitset_t *
make_bitset(size_t);

uint32_t
find_closest_distance(const disk_section *, const size_t *, uint32_t);

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

task_details_t
FCFS(const disk_section *, const size_t *);

task_details_t
SSTF(const disk_section *, const size_t *);

double
SCAN(const disk_section *, const size_t *);

double
CSCAN(const disk_section *, const uint32_t *);

/************************************************************************/
/************************************************************************/


/* volatile in case it gets cached since it fits into the cache */
volatile semaphore_t semaphores = {(0x00000000)};

/* wait for all the threads to finish before continuing */
volatile int32_t latch;


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
uint32_t     **head_movement_g;

/* 
    Arg order:
        1. Path
        2. Disk data 
        
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
    
    if(argc < 0)
        DIE("Warning", "Invalid number of arguments - %d", argc);
    
    
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
    
    task_details_t ex = FCFS(test_sections, &size);
    
   // printf("%d", test_sections[find_closest_distance(test_sections, &size, 41)]);
    
    printf("%f\n", ceil(2.125));
    
    double x = 2.25;
    
    /* 1/(bit_position) ) [11 bits exponent][53 mantissa] => */
    
    printf("%d", ((int64_t) x >> (51)));
    
    /*double x = 2.5;*/
    
    /*printf("%d", (((int64_t) x) & 0x000FFFFFFFFF)); */
    
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

/* 1100 -> 0100 -. 0011 -> 0111, -8, var & (1ul << sizeof(int32_t) * CHAR_BIT - 1), ~!sign * var */





/* branchless abs */
uint32_t
absolute_value(const int32_t value) {
    
    uint32_t mask = (value >> (sizeof(int32_t) * CHAR_BIT - 1));
    
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

task_details_t
FCFS(const disk_section *sections, const size_t *size) {
    
    double time_mili;
    
    int32_t index;
    
    int32_t  head_movement;

    uint32_t current_position;
    
    task_details_t execution_details;
    
    /* TODO: check for underflow */
    
    time_mili        = absolute_value(start_sector_g - sections[0]) * seek_time_g + access_time_g;
    
    current_position = sections[0];
    
    head_movement    = absolute_value(start_sector_g - sections[0]);
    
    for(index = 1; index < *size; ++index) {

        time_mili        = time_mili + absolute_value(current_position - sections[index]) * seek_time_g + access_time_g + (sections[index + 1] > sections[index] ? 0 : rotation_time_g);   
    
        head_movement   += absolute_value(current_position - sections[index]);
        
        current_position = sections[index];
        
    }
    
    execution_details.time_mili     = time_mili;
    execution_details.head_movement = head_movement;
    
    return(execution_details);

}

task_details_t
SSTF(const disk_section *sections, const size_t *size) {
    
    
    double time_mili;
    
    int32_t index;
    
    int32_t  head_movement;

    uint32_t current_position;
    
    uint32_t current;
    
    task_details_t execution_details;
    
    
    // 0 1 2 3 4 
    for(index = 0; index < *size; ++index) {
            
    }
     
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
        /* TODO: bitwise */
        
        if((((buf * 10) + CHAR_TO_DIGIT(*number)) & 0x80000000) == 0) 
            buf = (buf * 10) + CHAR_TO_DIGIT(*number);
        else
            DIE("Fatal", "Integer overflow - '%s' too big", number);
    }
        
    return(buf);
}

void 
FCFS_thread_start(void *vptr) {
    
    
    int32_t *sections;
    
    uint32_t index;
    uint32_t algo_index;
    
    size_t   size;
    
    task_details_t execution_details;
    
    for(index = 0, algo_index = 0; index < nlines_g; ++index) {
        
        sections          = sections_g[index];
        size              = sizeof(sections)/sizeof(*sections);
        execution_details = FCFS(sections, &size);
    
        algo_time_g[algo_index][FCFS_THREAD]        = execution_details.time_mili;
        head_movement_g[algo_index++][FCFS_THREAD]  = execution_details.head_movement;
        
        
        /* signal that it has finished the line */
        semaphores.signal_A = 0x01;
    
    
        /* busy wait until the other threads have finished */
        while(!IS_READY_NEXT_LINE(latch))
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


uint32_t
find_closest_distance(const disk_section *sections, const size_t *size, uint32_t section) {
    
    uint32_t index;
    uint32_t deltaD;
    uint32_t position;
    
    deltaD = (1 << 29);
    
    for(index = 0; index < *size; ++index) {
        
        if(absolute_value(section - sections[index]) < deltaD && sections[index] != section) {
        
            deltaD   = absolute_value(section - sections[index]);
        
            position = index; 
        }
            
    }
    
    return(position);
    
}


void *
mmalloc(size_t size) {
    
    void *addr;
    
    addr = malloc(size);
    
    if(addr == NULL)
        DIE("Fatal", "Failed to allocate memory", NULL);
        
    return(addr);
}


double
cceil(double value) {
    
    /* extract the mantissa from the double and check if it is above > .5 */   
    
    return 0.0;
}

bitset_t *
make_bitset(size_t size) {
    
    
    uint32_t  nbits;
    
    bitset_t *bitset;
    
    
    if(size <= 0)
        return NULL;
        
    
    nbits = ceil((double) size/sizeof(uint8_t));
    
    bitset = mmalloc(sizeof(uint8_t) * nbits);
    
    return(bitset);
    
}
     
/* 0x00 00 00 00 00 xx 00 00 00 00 00 =>
    55 => 55/8

    floor(log(n)/log(2)) - 1;
    
    [00000000][00000000][00000000]
    
    => 10 =>
    
*/
void
bitset_set_bit(bitset_t *set, uint32_t position) {
    
    uint32_t nbyte;
    
    nbyte = floor((double) position/8);
    
    /*set[nbyte] |= (1U << ())*/
}



