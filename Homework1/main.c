#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/****************************************************/
/****************************************************/

#define NGRADES     (0x5)
#define ZERO        (0x30)
#define TEN         (0x30 + 0x31)
#define WHITE_SPACE (0x20)
#define LINEBUF_SZ  (0x40)
#define LINE_OFFSET (0x8)
#define NEWL        (0x0A)
#define COMMA       (0x2C)
#define DTOI(c)     (c & 0x0F)
#define IS_DIGIT(c) (c >= 0x30 && c <= 0x39)


/****************************************************/
/****************************************************/

#define ERR(errno, m) do { \
    printf("%s", m); \
    exit(errno); \
}while(0);


/****************************************************/
/****************************************************/

/* file descriptor */
typedef int FD;

typedef enum {
    LEXICO,
    AVG_GRADE
} sort_type_e;

typedef struct {

    uint8_t grades[5];

    /* padding */
    char    pad[2];

    double avg_grade;
    char  * name;

} student_t;

typedef struct {

    FD         fd;

    /* number of students */
    uint32_t   nstuds;

    /* file size */
    uint64_t   fsize;

    /* number of character per line */
    uint8_t *  szlines;

} stud_file_t;


/****************************************************/
/****************************************************/


stud_file_t *
open_students_info(const char *);

student_t **
get_students(const stud_file_t *);

void
avg_grades_students(student_t **, const uint32_t *);

void
free_students(student_t **, stud_file_t *);

void
_students_info(stud_file_t *);

int
_cmp_fnc(const void *, const void *);


/****************************************************/
/****************************************************/

/* File format: */
/* Line_n: Student_n, grade_1, grade_2, grade_3, grade_4, grade_5 */
int main(int argc, char ** args) {

    const stud_file_t * sfd;
    pid_t pid[2];

    if(argc < 0)
        ERR(-1, "ERR: Invalid number of arguments")

    sfd = open_students_info(args[1]);
    student_t ** studs = get_students(sfd);

    pid[0] = fork();

    if(pid[0] < 0)
        ERR(-1, "ERR: Could not create another process")

    if(pid[0] == 0) {

        pid[1] = fork();

        if(pid[1] < 0)
            ERR(-1, "ERR: Could not create another process")

        if(pid[1] > 0) {
            avg_grades_students(studs, &sfd->nstuds);
        } else {
            qsort(studs, sfd->nstuds, sizeof(student_t *), &_cmp_fnc);
            avg_grades_students(studs, &sfd->nstuds);
        }

    } else {
        printf("%s", "\n");
    }

}


stud_file_t *
open_students_info(const char * path) {

    FD            fd;
    stud_file_t * sfd;

    fd  = open(path, (const char *) 0x0000);

    if(fd < 0)
        ERR(-1, "Could not open the students info file")

    if(!(sfd = malloc(sizeof(stud_file_t))))
        ERR(-1, "Could not allocate memory for the student fd")

    sfd->fd = fd;

    _students_info(sfd);

    return(sfd);

}


student_t **
get_students(const stud_file_t * sfd) {

    char         linebuf[LINEBUF_SZ];
    uint8_t      name_len;
    uint8_t      ch;
    uint8_t      grades_idx;

    /* loop indexes */
    uint32_t     i;

    char       * student_name;
    /* used to copy the name */
    char       * head;

    student_t ** students;

    grades_idx = 0;

    if(!(students = malloc(sizeof(student_t) * (sfd->nstuds))))
        ERR(-1, "Could not allocate memory for the students array")

    for(i = 0; i < sfd->nstuds; ++i) {
        if(!(students[i] = malloc(sizeof(student_t))))
            ERR(-1, "Could not allocate memory for the student")

        students[i]->avg_grade = 0.0;
    }

    for(i = 0; i < sfd->nstuds; ++i) {

        /* read one line into the buffer */
        read(sfd->fd, linebuf, sfd->szlines[i]);


        /* get the length of the name of the student */
        for(ch = LINE_OFFSET, name_len = 0; linebuf[ch] != COMMA; ++ch, ++name_len) {}

        if(!(student_name = malloc(sizeof(char) * (name_len + 1))))
            ERR(-1, "Could not allocate memory for the student name")

        head = student_name;

        /* copy the name of the student into the memory chunk */
        for(ch = LINE_OFFSET; linebuf[ch] != COMMA; ++ch, ++head) {
            *head = linebuf[ch];
        }

        *head = 0;

        students[i]->name = student_name;


        /* get the grades of the student */
        for(; ch < sfd->szlines[i]; ) {

            if(linebuf[ch] == '-')
                ERR(-1, "ERR: Negative grade found")

            /* checking to see if the student has a 10 since it's 2 characters */
            if(linebuf[ch] == WHITE_SPACE && IS_DIGIT(linebuf[ch + 1]) && IS_DIGIT(linebuf[ch + 2])) {

                if(linebuf[ch + 1] + linebuf[ch + 2] != TEN)
                    ERR(-1, "ERR: Grade higher than 10 found")

                students[i]->grades[grades_idx++] = 10;

                /* skip the next comma */
                ch += 3;

            } else if(linebuf[ch] == WHITE_SPACE && IS_DIGIT(linebuf[ch + 1])) {

                if(linebuf[ch + 1] == ZERO)
                    ERR(-1, "ERR: Grade equal to 0 found")

                students[i]->grades[grades_idx++] = (uint8_t) DTOI(linebuf[ch + 1]);
                students[i]->avg_grade += (double) DTOI(linebuf[ch + 1]);
            }


            ++ch;

        }

        /* compute the avg grade of the student */
        students[i]->avg_grade = (double)(students[i]->avg_grade/NGRADES);

        /* reset the idx for the next student */
        grades_idx = 0;

    }

    return students;

}

void
avg_grades_students(student_t ** students, const uint32_t * nstuds) {

    uint32_t i;

    for(i = 0; i < *nstuds; ++i)
        printf("Name: %s | Avg. grade: %.1f\n", students[i]->name, students[i]->avg_grade);
}


void
free_students(student_t ** students, stud_file_t * sfd) {
    uint32_t nstuds;
    uint32_t  i;

    nstuds = sfd->nstuds;


    if(sfd == NULL)
        return;

    free(sfd->szlines);
    sfd->szlines = NULL;
    close(sfd->fd);
    free(sfd);

    for(i = 0; i < nstuds; ++i) {
        free(students[i]->name);
        free(students[i]);
        students[i] = NULL;
    }

    free(students);
}
/****************************************************/
/****************************************************/


int
_cmp_fnc(const void * arg1, const void * arg2) {

    const student_t * s1 = *(student_t **)arg1;
    const student_t * s2 = *(student_t **)arg2;

    return(strcmp(s1->name, s2->name) > 0);
}


void
_students_info(stud_file_t * sfd) {

    char          ch[3];

    /* characters read per line */
    uint8_t       cread;
    uint16_t      nstudents;

    /* the default number of students */
    uint16_t      default_sz;
    uint16_t      idx;

    /* keep track of how many characters we have read so we can move the cursor */
    /* back at the beginning of the file */
    int64_t       fsize;

    nstudents  = 0;
    fsize      = 0;
    idx        = 0;
    cread      = 0;
    default_sz = 8;


    if(!(sfd->szlines = malloc(sizeof(uint8_t) * default_sz)))
        ERR(-1, "Could not allocate memory for the student fd");

    for(; (read(sfd->fd, (void *) ch, 1));  ++fsize, ++cread) {

        if(ch[0] == NEWL) {

            ++nstudents;

            if(idx + 1 == default_sz)
                sfd->szlines = realloc(sfd->szlines, sizeof(uint8_t) * (default_sz *= 2));

            sfd->szlines[idx++] = cread + 1;
            cread = -1;

        }

    }

    if(idx + 1 != default_sz)
        sfd->szlines = realloc(sfd->szlines, sizeof(uint8_t) * (idx));

    sfd->fsize  = fsize;
    sfd->nstuds = nstudents;

    /* move the cursor at the beginning of the file again */
    lseek(sfd->fd, -fsize, SEEK_END);

}
