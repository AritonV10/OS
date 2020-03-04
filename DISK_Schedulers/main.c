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
        printf("[" type "]: " fmt "\n", __VA_ARGS__); \
    }while(0);
    
#define DIE(type, fmt, ...) do { \
        LOG(type, fmt, __VA_ARGS__) \
        exit(1); \
    }while(0);


#define ALGO_TIME(a, b) absolute_value((a) - (b)) * seek_time_g + access_time_g
#define IS_DIRECTION_CHANGE(a, b) ((a) < (b) ? 0 : rotation_time_g)


#define NTHREADS              (0x4)
#define FCFS_THREAD           (0x01) 
#define SCAN_THREAD           (0x02)
#define _THREAD               (0x03)
#define _THREAD2              (0x04)


#define CHAR_TO_DIGIT(c)      ((c) & 0x0F)
    
#define SIZE(c)               (sizeof(c)/sizeof(*c))
#define CHAR_BIT              (8)
#define SINT_MAX              (1u << (sizeof(int32_t) * CHAR_BIT - 1))
#define UDOUBLE_EXPONENT_MAX  (1u << (sizeof(double) * CHAR_BIT - 1))

#define U8_SIZE               (sizeof(uint8_t))
#define U8_SIZE_BITS          (U8_SIZE * CHAR_BIT)


#define IS_DONE(c)            (c == DONE)
#define IS_READY_NEXT_LINE(c) (c == LINE)

/* lock the signals */
#define ENTER_CRITICAL
#define EXIT_CRITICAL
    
/******************** TYPE DEFINITIONS **********************************/

typedef int FD;

typedef void(*algo_func)(void*);

typedef uint32_t disk_section;


typedef enum _direction {
    
    LEFT  = 0,
    
    RIGHT = 1,
    
} direction_e;

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


typedef union _signal {
    
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
    
} signal_t;


typedef struct _bitset {
    
    uint8_t   resizeable;
    
    uint32_t  nbits;

    uint8_t   *array;
    
} bitset_t;


/******************** FUNCTION DECLARATIONS *****************************/


/***** BITSET *****/

void *
mmalloc(size_t);

bitset_t *
make_bitset(size_t, uint8_t);

int32_t
bitset_set_bit(bitset_t *, const uint32_t);

uint32_t
bitset_to_index(const uint32_t);

int8_t
bitset_get_bit(const bitset_t *, const uint32_t);

void
bitset_clear(bitset_t *);

/****************************/


void
SCAN_right(const disk_section *, const size_t *, double *, int32_t *, uint32_t, uint32_t *);
    

void
SCAN_left(const disk_section *, const size_t *, double *, int32_t *, uint32_t, uint32_t *);
    

int32_t 
comparator(const void *, const void *); 

uint32_t
find_position(const disk_section *, const size_t *, const disk_section);
    
int32_t
find_closest_distance(const disk_section *, const size_t *, uint32_t, const bitset_t *);

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

task_details_t
SCAN(const disk_section *, const size_t *);

double
CSCAN(const disk_section *, const uint32_t *);



/************************************************************************/
/************************************************************************/


/* volatile in case it gets cached since it fits into the cache */
volatile signal_t thread_signals = {(0x00000000)};

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
        
        disk_section test_sections[] = {9, 20, 25, 33, 56, 77, 120, 155, 200};
        size_t size                  = SIZE(test_sections);         
        
        /* element: 12 -> 
        14, 41, 65, 67, 98, 122, 124, 183 */
        
        rotation_time_g              = (double) .1;
        
        access_time_g                = (double) .2;
    
        seek_time_g                  = (double) .1;
    
        start_sector_g               = 125;
    
    #endif

    task_details_t ex = FCFS(test_sections, &size);
    
    bitset_t *t = make_bitset(200, 1);
    
    printf("%d\n", SCAN(test_sections, &size).head_movement);
    
    /*
    for(int i = 0; i < (uint32_t) ceil((double) t->nbits/U8_SIZE_BITS); ++i) {
        for(int j = 0; j < 8; ++j) {
            printf("%d ", (t->array[i] & (1u << j)) == 0 ? 0 : 1);
        }
        
        printf("%c", '|');
    }
    
    while(!IS_DONE(latch))
        ;
        
    for(index = 0; index < nlines_g; ++index) {
        
        double *times = algo_time_g[index];
        uint32_t min  = find_min_time(times);
        
        
        printf("Dataset %d =\nTime: %d\nBy: %s", (i + 1), algoIndex[index][min], threads[index]);
        
    }
    */
    
    return 0;
}

/******************** FUNCTION DEFINITIONS ******************************/


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


int32_t 
comparator(const void *p, const void *q) { 
    
    disk_section section_a = *((disk_section*)p); 
    
    disk_section section_b = *((disk_section*)q);  
    
    return(section_a >= section_b); 
} 



uint32_t
find_position(const disk_section *sections, const size_t *size, const disk_section section) {
    
    
    uint32_t index;

    for(index = 0; index < *size; ++index)
        if(sections[index] >= section)
            return(index - 1);
    
    return(*size - 1);
}


void
SCAN_left(const disk_section *sections, const size_t *size, double *time_mili, int32_t *head_movement, 
uint32_t position, uint32_t *current_sector) {
    
    
    int32_t index;
    
    for(index = position; index >= 0; --index) {
                    
        *time_mili       = *time_mili + ALGO_TIME(*current_sector, sections[index]) + IS_DIRECTION_CHANGE(*current_sector, sections[index]);   
                    
        *head_movement   += absolute_value(*current_sector - sections[index]);
                    
        printf("%d - %d\n", *current_sector, sections[index]);
                    
        *current_sector = sections[index];
    
    }
    
}


void
SCAN_right(const disk_section *sections, const size_t *size, double *time_mili, int32_t *head_movement, 
uint32_t position, uint32_t *current_sector) {
    
    uint32_t index;
    
    for(index = (position == -1 ? 0 : position + 1); index < *size; ++index) {
                    
        *time_mili       = *time_mili + ALGO_TIME(*current_sector, sections[index]) + IS_DIRECTION_CHANGE(*current_sector, sections[index]);   
                    
        *head_movement   += absolute_value(*current_sector - sections[index]);
                    
        printf("%d - %d\n", *current_sector, sections[index]);
                    
        *current_sector = sections[index];
                    
    }
                
}


task_details_t
SCAN(const disk_section *sections, const size_t *size) {
    
    
    double time_mili;
    
    int32_t index;
    
    int32_t  head_movement;

    uint32_t current_sector;
    
    uint32_t position;
    
    direction_e direction;
    
    task_details_t execution_details;
    
    
    disk_section *sections_copy;
    
    sections_copy = mmalloc((*size) * sizeof(disk_section));
    head_movement = 0;
    time_mili     = 0.0;
    
    
    /* make a copy since the array is global */
    for(index = 0; index < *size; ++index)
        sections_copy[index] = sections[index];
        
    /* sort the array so we can find the position of the start sector */
    qsort(sections_copy, *size, sizeof(disk_section), comparator);
    
    position = find_position(sections_copy, size, start_sector_g);
    
    if(position < (*size - position))
        direction = LEFT;
    else
        direction = RIGHT;
    
    
    if(direction)
        printf("%s\n", "Going RIGHT");
    else
        printf("%s\n", "Going LEFT");
        
    current_sector = start_sector_g;
    
    switch(direction) {
        
        case LEFT:
        
            SCAN_left(sections_copy, size, &time_mili, &head_movement, position, &current_sector);
        
            SCAN_right(sections_copy, size, &time_mili, &head_movement, position, &current_sector);
        
        break;
            
        case RIGHT:
        
            SCAN_right(sections_copy, size, &time_mili, &head_movement, position, &current_sector);
        
            SCAN_left(sections_copy, size, &time_mili, &head_movement, position, &current_sector);
        
        break;
    }
    
    execution_details.time_mili     = time_mili;
    execution_details.head_movement = head_movement;
    
    
    return(execution_details);
}



task_details_t
FCFS(const disk_section *sections, const size_t *size) {
    
    double time_mili;
    
    int32_t index;
    
    int32_t  head_movement;

    uint32_t current_sector;
    
    task_details_t execution_details;
    
    /* TODO: check for underflow */
    
    current_sector = start_sector_g;
    head_movement  = 0;
    
    for(index = 0; index < *size + 1; ++index) {

        time_mili        = time_mili + ALGO_TIME(current_sector, sections[index]) + IS_DIRECTION_CHANGE(current_sector, sections[index]);   
        head_movement   += absolute_value(current_sector - sections[index]);
        
        current_sector = sections[index];
        
    }
    
    execution_details.time_mili     = time_mili;
    execution_details.head_movement = head_movement;
    
    return(execution_details);

}


double
ud_stoi(const char *__restrict number) {
    
    double   buf;
    
    double   mantissa;
    
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
    for(; number != NULL; ++number) {
        
        mantissa = (float)(CHAR_TO_DIGIT(*number))/ten;
        
        ten *= 0x0000000A;
        
        buf += mantissa;
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
        thread_signals.signal_A = 0x01;
    
    
        /* busy wait until the other threads have finished */
        while(!IS_READY_NEXT_LINE(latch))
            ;
    }
    
    /* signal that it has finished all the lines */
    thread_signals.signal_A = 0x10;
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


int32_t
find_closest_distance(const disk_section *sections, const size_t *size, uint32_t section, const bitset_t *table) {
    
    uint32_t index;
    uint32_t deltaD;
    uint32_t position;
    uint32_t abs_distance;
    
    
    deltaD = (1 << 31);
    
    for(index = 0; index < *size; ++index) {
        
        abs_distance = absolute_value(section - sections[index]);
        
        
        if(abs_distance <= deltaD 
        && 
        sections[index] != section
        && 
        bitset_get_bit(table, sections[index]) != 1) {
        
            
            deltaD   = absolute_value(section - sections[index]);
            position = index; 
        }
            
    }
    
    
    if(deltaD != (1 << 31)) {
        bitset_set_bit((bitset_t *)table, sections[position]);
        return(position);
    }
    
    return(-1);
    
}


void *
mmalloc(size_t size) {
    
    void *addr;
    
    addr = malloc(size);
    
    if(addr == NULL)
        DIE("Fatal", "Failed to allocate memory", NULL);
        
    return(addr);
}

bitset_t *
make_bitset(size_t size_bits, uint8_t resizeable) {
    
    
    uint32_t  i;
    uint32_t  nbytes;
    
    bitset_t *bitset;
    
    if(size_bits <= 0)
        return NULL;
    
    nbytes        = (uint32_t) ceil((double) size_bits/U8_SIZE_BITS);
    
    bitset             = mmalloc(sizeof(bitset_t));
    bitset->array      = mmalloc(U8_SIZE * nbytes);
    bitset->nbits      = size_bits;
    bitset->resizeable = resizeable;
    
    for(i = 0; i < nbytes; ++i)
        bitset->array[i] = 0;
    
    
    LOG("Bitset_LOG", "Allocated %d bytes", nbytes);
    
    return(bitset);
    
}
     
uint32_t
bitset_to_index(const uint32_t position) {
    return((uint32_t)((double)(position - 1)/U8_SIZE_BITS));
}

int32_t
bitset_set_bit(bitset_t *set, const uint32_t position) {
    
    
    uint32_t nbyte;
    
    if(set->nbits < position) {
        switch(set->resizeable) {
            case 1:
            
                 set->array = realloc(set->array, (uint32_t) ceil((double) position/U8_SIZE_BITS) + 1);
                 set->nbits = position;
                
            break;
            
            return(-1);
        }
    }
        
    
    nbyte = bitset_to_index(position); 
        
    set->array[nbyte] |= (1u << ((position - 1) % U8_SIZE_BITS));
    
    LOG("Bitset_LOG", "Setting bit %d", position);
    
    return(1);
}

int8_t
bitset_get_bit(const bitset_t *set, uint32_t position) {
    
    uint32_t nbyte;
    
    if(set->nbits < position)
        return -1;
    
    nbyte = bitset_to_index(position);
    
    return( ((set->array[nbyte] & (1u << (((position - 1) % U8_SIZE_BITS)))) == 0 ? 0 : 1) );
}



task_details_t
SSTF(const disk_section *sections, const size_t *size) {
    
    
    double time_mili;
    
    int32_t  index;
    int32_t  head_movement;
    uint32_t min_position;
    
    uint32_t current_sector;
    
    task_details_t execution_details;
    
    bitset_t *table;
    
    table          = make_bitset(500, 1);
    
    current_sector = start_sector_g;
    
    head_movement  = 0;
    
    for(index = 0; index < *size + 1; ++index) {
               
        min_position  = find_closest_distance(sections, size, current_sector, table);
        
        /* check to see if it is the last element */
        if(min_position == -1)
          break;  
        
        time_mili     = time_mili + ALGO_TIME(current_sector, sections[min_position]) + IS_DIRECTION_CHANGE(current_sector, sections[min_position]);
        head_movement += absolute_value(current_sector - sections[min_position]);
        
        printf("%d - %d\n", current_sector, sections[min_position]);
        
        current_sector = sections[min_position];
    }
    
    execution_details.head_movement = head_movement;
    execution_details.time_mili     = time_mili;
    
    return(execution_details);
     
}



