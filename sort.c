#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <alloca.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


#include "top.h"

#define PID 0
#define Pid 1
#define USER 2
#define User 3
#define PR 4
#define Pr 5
#define NI 6
#define Ni 7
#define VIRT 8
#define Virt 9
#define RES 10
#define Res 11
#define SHR 12
#define Shr 13
#define SS 14
#define Ss 15
#define CPU 16
#define Cpu 17
#define TIME 18
#define Time 19
#define COMMAND 20
#define Command 21

//for qsort
typedef struct
  {
    char *lo;
    char *hi;
  } stack_node;



#define SWAP(a, b, size)                                                      \
  do                                                                              \
    {                                                                              \
      size_t __size = (size);                                                      \
      char *__a = (a), *__b = (b);                                              \
                                                                            \
                                                                              \
          char __tmp = *__a;                                                      \
          *__a = *__b;                                                      \
          *__b = __tmp;                                                      \
                                                              \
    } while (0)

#define MAX_THRESH 4
#define STACK_SIZE        (8 * sizeof(size_t))
#define PUSH(low, high)        ((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define        POP(low, high)        ((void) (--top, (low = top->lo), (high = top->hi)))
#define        STACK_NOT_EMPTY        (stack < top)
 //////////////////////////////////////////////////////////////////


int sorting(const void * x,const void * x1, void * arg) {
    data_top * P = (data_top *)x;
    data_top * P1 = (data_top *)x1;
    int flag = *(int*)arg;

    switch( flag) {
        case PID:
            return (P->pid < P1->pid);
            break;
        case Pid:
            return (P->pid > P1->pid);
            break;

        case USER:
            return (!strcmp(P->user, P1->user) );
            break;
        case User:
            return (strcmp(P->user, P1->user) );
            break;

        case PR:
            return (P->pr < P1->pr);
            break;
        case Pr:
            return (P->pr > P1->pr);
            break;

        case NI:
            return (P->ni < P1->ni);
            break;
        case Ni:
            return (P->ni > P1->ni);
            break;

        case VIRT:
            return (P->virt < P1->virt);
            break;
        case Virt:
            return (P->virt > P1->virt);
            break;

        case RES:
            return (P->res < P1->res);
            break;
        case Res:
            return (P->res > P1->res);
            break;

        case SHR:
            return (P->shr < P1->shr);
            break;
        case Shr:
            return (P->shr > P1->shr);
            break;

        case SS:
            return (P->S < P1->S);
            break;
        case Ss:
            return (P->S > P1->S);
            break;

        case CPU:
            return (P->cpu < P1->cpu);
            break;
        case Cpu:
            return (P->cpu > P1->cpu);
            break;

        case TIME:
            return (P->stime < P1->stime);
            break;
        case Time:
            return (P->stime > P1->stime);
            break;

        case COMMAND:
            return (!strcmp(P->com, P1->com) );
            break;
        case Command:
            return (strcmp(P->com, P1->com) );
            break;



    }

}

//    qsort(solution,count_proc, sizeof(parametrs),sorting);

static char* partition(char*  left_ptr, char* right_ptr, size_t size, compar_d_fn_t cmp, void *arg ){
    void * mid = (left_ptr + right_ptr) / 2;

    while (left_ptr <= right_ptr) {
        while ((*cmp)( (void*)left_ptr,(void*)mid,arg) > 0 )
            left_ptr += size;
        while (!sorting((void*)mid, (void *)right_ptr,arg) > 0 )
            right_ptr -= size;
        if (left_ptr < right_ptr) {
            SWAP  (left_ptr, right_ptr, size);
        } else if (left_ptr == right_ptr)
               {
               left_ptr += size;
               right_ptr -= size;
               break;
               }
    }
    return (left_ptr + size);

}

void myqsort(char* lower, char* higher, size_t size, compar_d_fn_t cmp, void *arg ){
    if (lower < higher) {
        char* middle = partition(lower,higher,size,cmp,arg);
        myqsort(lower,middle-size,size,cmp,arg);
        myqsort(middle,higher,size,cmp,arg);
    }
}

void sort(void *const pbase, size_t total_elems, size_t size, compar_d_fn_t cmp, void *arg)
{
    char *lo = (char *) pbase;
    char *hi = &lo[size * (total_elems - 1)];
    myqsort(lo,hi,size,cmp,arg);

}









