#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "mpi.h"

#define BensType int
#define MPI_BEN_TYPE MPI_INT

void TopDownMergeSort(BensType *a, BensType *b, int n);
void TopDownSplitMerge(BensType *B, int iBegin, int iEnd, BensType *A);
void TopDownMerge(BensType *A, int iBegin, int iMiddle, int iEnd, BensType *B);
void CopyArray(BensType *A, int iBegin, int iEnd, BensType *B);
void printArray(BensType *array, int n);

int main(int argc, char **argv)
{
    int n = 10000000;
    double startTime;
    double endTime;
    srand(time(0));
    BensType *array = (BensType *)malloc(n*sizeof(BensType));
    //BensType *work = (BensType *)malloc(n*sizeof(BensType));
    int rank, numranks, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(hostname, &len);

    if(rank == 0)
    {
        printf("Array to be sorted:\n");
        for(int i = 0; i < n; i++)
        {
            array[i] = rand() % (n + 1 - 0) + 0;
        }
        //printArray(array, n);
    }
    if(rank == 0)
    {
        startTime = MPI_Wtime();
    }

    int secSize = n / numranks;
    //printf("secSize: %d\n", secSize);
    BensType *scatterMat = (BensType *)malloc(secSize*sizeof(BensType));
    BensType *work = (BensType *)malloc(secSize*sizeof(BensType));
    MPI_Scatter(array, secSize, MPI_BEN_TYPE, scatterMat, secSize, MPI_BEN_TYPE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    TopDownMergeSort(scatterMat, work, secSize);
    MPI_Gather(scatterMat, secSize, MPI_BEN_TYPE, array, secSize, MPI_BEN_TYPE, 0, MPI_COMM_WORLD);

    //I now have numranks sorted arrays inside of array that need to be sorted.
    if(rank == 0)
    {
        int nummerges = numranks - 1;
        printf("nummerges %d\n", nummerges);
        for(int i = 0; i < nummerges; i++)
        {
            int l = 0;
            for(int j = 0; j < (n/secSize)/2; j++)
            {
                int m = l + secSize;
                int h = m + secSize;
                work = (BensType *)malloc(n*sizeof(BensType)); 
                CopyArray(array, l, h, work);
                TopDownMerge(work, l, m, h, array);
                //printArray(array, n);
                l = h;
            }
            secSize = secSize * 2;
        }
    }

    if(rank == 0)
    {
        printf("Sorted Array:\n");
        //printArray(array, n);
        endTime = MPI_Wtime();
        printf("TTC %.12f\n", endTime - startTime);
    }

    MPI_Finalize();
}

// Array A[] has the items to sort; array B[] is a work array.
void TopDownMergeSort(BensType *A, BensType *B, int n)
{
    CopyArray(A, 0, n, B);           // duplicate array A[] into B[]
    TopDownSplitMerge(B, 0, n, A);   // sort data from B[] into A[]
}

// Sort the given run of array A[] using array B[] as a source.
// iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
void TopDownSplitMerge(BensType *B, int iBegin, int iEnd, BensType *A)
{
    if(iEnd - iBegin < 2)                       // if run size == 1
        return;                                 //   consider it sorted
    // split the run longer than 1 item into halves
    int iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
    // recursively sort both runs from array A[] into B[]
    TopDownSplitMerge(A, iBegin,  iMiddle, B);  // sort the left  run
    TopDownSplitMerge(A, iMiddle,    iEnd, B);  // sort the right run
    // merge the resulting runs from array B[] into A[]
    TopDownMerge(B, iBegin, iMiddle, iEnd, A);
}

//  Left source half is A[ iBegin:iMiddle-1].
// Right source half is A[iMiddle:iEnd-1   ].
// Result is            B[ iBegin:iEnd-1   ].
void TopDownMerge(BensType *A, int iBegin, int iMiddle, int iEnd, BensType *B)
{
    int i = iBegin, j = iMiddle;
 
    // While there are elements in the left or right runs...
    for (int k = iBegin; k < iEnd; k++) {
        // If left run head exists and is <= existing right run head.
        if (i < iMiddle && (j >= iEnd || A[i] <= A[j])) {
            B[k] = A[i];
            i = i + 1;
        } else {
            B[k] = A[j];
            j = j + 1;
        }
    }
}

void CopyArray(BensType *A, int iBegin, int iEnd, BensType *B)
{
    for(int k = iBegin; k < iEnd; k++)
        B[k] = A[k];
}

void printArray(BensType *array, int n)
{
    for(int i = 0; i < n; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}
