#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#define REPS 127

Mat TheImage;    // This is the main image
int rows, cols;             // number of rows and colums in image

void Flip()
{
    unsigned char temp;
    int r, c;

    unsigned char *p;
    for (c=0; c<cols; c++)
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
}


int main(int argc, char** argv)
{
    // load image
    TheImage = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    rows = TheImage.rows;
    cols = TheImage.cols;
    printf("rows: %d, cols: %d\n", rows, cols);

    struct timeval t;
    double StartTime, EndTime, TimeElapsed;

    gettimeofday(&t, NULL);
    StartTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);

    int i;
    for (i=0; i<REPS; i++) {
        Flip();    // call flip function
    }

    gettimeofday(&t, NULL);
    EndTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
    TimeElapsed = (EndTime-StartTime)/1000.00;
    TimeElapsed /= (double)REPS;

    imwrite(argv[2], TheImage);

    printf("\nTotal execution time: %9.4f ms.\n", TimeElapsed);

    return 1;
}


