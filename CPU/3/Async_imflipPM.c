#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#define REPS 127

Mat TheImage;    // This is the main image
int rows, cols;             // number of rows and colums in image

/* **** for pthread parallel computing ******* */
#include <pthread.h>
#define MAX_THREADS 64

long  NumThreads;    // number of threads work in parallel
int ThParam[MAX_THREADS];     // store thread parameters
pthread_t ThHandle[MAX_THREADS];    //store thread handles
pthread_attr_t ThAttr;    // pthread attribute

/* ******************************************** */


/* ********* for asynchronized multi-thread CPU code ****** */

pthread_mutex_t CounterMutex;    // define mutex
int NextRowToProcess;

/* **************************************** */

void *AsyncMFlipM(void *ThCtr)
{
    int r, j;
    unsigned char Buffer1[16384];    // This is the buffer to get the first row of image;
    unsigned char Buffer2[16384];    // This is the buffer to get the second row of image;

    do {
        // get the next row number
        pthread_mutex_lock(&CounterMutex);    // lock it before accessing
        r = NextRowToProcess;
        NextRowToProcess ++;

        // record how many rows are processed by this thread
        j = *((int *)ThCtr);
        *((int *) ThCtr) = j+1;

        pthread_mutex_unlock(&CounterMutex);    // unlock it after accessing

        if (r>=rows/2) {
            break;
        }

        memcpy((void *)Buffer1, TheImage.ptr<uchar>(r), cols*sizeof(unsigned char));
        memcpy((void *)Buffer2, TheImage.ptr<uchar>(rows-(r+1)), cols*sizeof(unsigned char));

        // swap two rows
        memcpy((void *) TheImage.ptr<uchar>(r), (void *)Buffer2, cols*sizeof(unsigned char));
        memcpy((void *) TheImage.ptr<uchar>(rows-(r+1)), (void *)Buffer1, cols*sizeof(unsigned char));
    } while(r<rows/2);

    pthread_exit(NULL);
}


int main(int argc, char** argv)
{
    // load image
    TheImage = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    rows = TheImage.rows;
    cols = TheImage.cols;
    printf("rows: %d, cols: %d\n", rows, cols);

    if (argc<4) {
        NumThreads = 2;
    } else {
        NumThreads = atoi(argv[3]);
    }
    printf("NumThreads: %d\n", NumThreads);

    struct timeval t;
    double StartTime, EndTime, TimeElapsed;

    gettimeofday(&t, NULL);
    StartTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);

    int i, a, ThErr;

    // pthread initialization
    pthread_attr_init(&ThAttr);
    pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);

    for (a=0; a<REPS; a++) {
        // initialize mutex
        pthread_mutex_init(&CounterMutex, NULL);

        pthread_mutex_lock(&CounterMutex);
        NextRowToProcess = 0;
        for (i=0; i<NumThreads; i++) {
            ThParam[i] = 0;
        }
        pthread_mutex_unlock(&CounterMutex);

        // assign task to each thread
        for (i=0; i<NumThreads; i++) {
            // important!! lauch threads
            ThErr = pthread_create(&ThHandle[i], &ThAttr, AsyncMFlipM, (void *)&ThParam[i]);
            if (ThErr != 0) {
                printf("\nThread Creation Error %d.\n", ThErr);
                exit(EXIT_FAILURE);
            }
        }

        for (i=0; i<NumThreads; i++) {
            // important!! join all threads
            //printf("thread %d: %d\n", i, ThParam[i]);
            pthread_join(ThHandle[i], NULL);
        }
        pthread_attr_destroy(&ThAttr);
        pthread_mutex_destroy(&CounterMutex);
    }

    gettimeofday(&t, NULL);
    EndTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
    TimeElapsed = (EndTime-StartTime)/1000.00;
    TimeElapsed /= (double)REPS;

    imwrite(argv[2], TheImage);

    printf("\nTotal execution time: %9.4f ms.\n", TimeElapsed);

    return 1;
}


