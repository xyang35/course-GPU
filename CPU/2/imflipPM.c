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

void *MFlipM(void *tid)
{
    int r;
    unsigned char Buffer1[16384];    // This is the buffer to get the first row of image;
    unsigned char Buffer2[16384];    // This is the buffer to get the second row of image;

    long ts = *((int *) tid);    // thread ID
    ts *= rows / NumThreads / 2;    // start index
    long te = ts + rows/NumThreads/2 - 1;    // end index

    for (r=ts; r<=te; r++)
    {
        // important!! copy data to cache memory
        memcpy((void *)Buffer1, TheImage.ptr<uchar>(r), cols*sizeof(unsigned char));
        memcpy((void *)Buffer2, TheImage.ptr<uchar>(rows-(r+1)), cols*sizeof(unsigned char));

        // swap two rows
        memcpy((void *) TheImage.ptr<uchar>(r), (void *)Buffer2, cols*sizeof(unsigned char));
        memcpy((void *) TheImage.ptr<uchar>(rows-(r+1)), (void *)Buffer1, cols*sizeof(unsigned char));
    }

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
        // assign task to each thread
        for (i=0; i<NumThreads; i++) {
            ThParam[i] = i;
            // important!! lauch threads
            ThErr = pthread_create(&ThHandle[i], &ThAttr, MFlipM, (void *)&ThParam[i]);
            if (ThErr != 0) {
                printf("\nThread Creation Error %d.\n", ThErr);
                exit(EXIT_FAILURE);
            }
        }

        for (i=0; i<NumThreads; i++) {
            // important!! join all threads
            pthread_join(ThHandle[i], NULL);
        }
    }

    gettimeofday(&t, NULL);
    EndTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
    TimeElapsed = (EndTime-StartTime)/1000.00;
    TimeElapsed /= (double)REPS;

    imwrite(argv[2], TheImage);

    printf("\nTotal execution time: %9.4f ms.\n", TimeElapsed);

    return 1;
}


