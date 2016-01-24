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

void *MFlip(void *tid)
{
    unsigned char temp;
    int r, c;

    long ts = *((int *) tid);    // thread ID
    ts *= cols / NumThreads;    // start index
    long te = ts + cols/NumThreads - 1;    // end index

    unsigned char *p;
    for (c=ts; c<te; c++)
    {
        r = 0;
        while (r<rows/2)
        {
            // flip pixels
            temp = TheImage.at<uchar>(r, c);
            TheImage.at<uchar>(r, c) = TheImage.at<uchar>(rows-(r+1), c);
            TheImage.at<uchar>(rows-(r+1),c) = temp;

            r++;
        }
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
            ThErr = pthread_create(&ThHandle[i], &ThAttr, MFlip, (void *)&ThParam[i]);
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


