#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

#define BensType int
#define mpiBenType MPI_INT

void merge(int *array, int low, int mid, int high, int n);
void sort(int *array, int low, int high, int n);
void printArray(BensType *array, int n);

int main(int argc, char **argv){

    int n = 16;
    srand(time(0));
    BensType *array = (BensType *)malloc(n*sizeof(BensType));
    int rank, numranks, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(hostname, &len);

    if(rank == 0 && n % numranks != 0)
    {
        printf("N: %d, Numranks: %d\n, not possible", n, numranks);
        exit(1);
    }

    if(rank == 0)
    {
         printf("Array to be sorted:\n");
         for(int i = 0; i < n; i++)
         {
            array[i] = rand() % (n + 1 - 0) + 0;
         }
         printArray(array, n);
    }

    int secSize = n/numranks;
    BensType *scatterMat = (BensType *)malloc(secSize*sizeof(BensType));
    MPI_Scatter(array, secSize, mpiBenType, scatterMat, secSize, mpiBenType, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //start the recursive calls
    sort(scatterMat, 0 , secSize-1, secSize);
    
    MPI_Gather(scatterMat, secSize, mpiBenType, array, secSize, mpiBenType, 0, MPI_COMM_WORLD);
    //There are now numranks sorted arrays within array
    //These arrays need to be merged
   
    //Merge smaller arrays inside array
    int c = numranks/2;
    for(int i = 0; i < c-1; i++)
    {
        printf("Woo!");
        MPI_Barrier(MPI_COMM_WORLD);
        int numreceive;
        BensType *myarr;
        if(rank == 0)
        {
            //printf("Woo! %d", i);
            if(i == c-1) //Final merge
            {
                printf("DOING FINAL MERGE");
                merge(array, 0, (n-1/2), n-1, n);
            }
            else
            {
                for(int j = 1; j <= c; j++)
                {
                    MPI_Status status;
                    printf("Rank %d sending to rank %d", rank, j);
                    MPI_Send(&numreceive, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                    myarr = (BensType *)malloc(numreceive*sizeof(BensType));
                    for(int k = 0; k < numreceive; k++)
                    {
                        myarr[k] = array[(c*k-c)*rank]; 
                    }
                    MPI_Send(myarr, numreceive, mpiBenType, j, 0, MPI_COMM_WORLD);
                    MPI_Recv(myarr, numreceive, mpiBenType, 0, 0, MPI_COMM_WORLD, &status);
                    for(int k = 0; k < numreceive; k++)
                    {
                        array[(c*k-c)*rank] = myarr[k];
                    }
                }
            }
        }
        else if(rank <= c)
        {
            MPI_Status status;
            MPI_Recv(&numreceive, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            myarr = (BensType *)malloc(numreceive*sizeof(BensType));
            MPI_Recv(myarr, numreceive, mpiBenType, 0, 0, MPI_COMM_WORLD, &status);
            merge(myarr, 0, (numreceive-1)/2, (numreceive-1), numreceive);
            MPI_Send(myarr, numreceive, mpiBenType, 0, 0, MPI_COMM_WORLD);
        }
        c = c/2;
    }

    //Print final sorted array
    if(rank == 0)
    {
        printf("Sorted Array:\n");
        printArray(array, n);
    }
    MPI_Finalize();
    return 0;
}


void merge(int *array, int low, int mid, int high, int n)
{
    BensType *b = (BensType *)malloc(n*sizeof(BensType));;

    //copy the current elements into temp array
    int v;
    for (v = low; v <= high; v++){
        b[v] = array[v];
    }
    int i = low;
    int j = mid + 1;
    int k = low;

    //merge left side and right side into array in sorted order
    while (i <= mid && j <= high) {
        if (b[i] <= b[j])
            array[k++] = b[i++];
        else
            array[k++] = b[j++];
    }

    //The left side might have left-over elements
    while (i <= mid)
        array[k++] = b[i++];


}

void sort(int *array, int low, int high, int n)
{
    int mid;
    if (low < high) {
        mid = (high + low)/2;
        sort(array, low, mid, n);
        sort(array, mid + 1, high, n);
        merge(array, low, mid, high, n);
   }

}

void printArray(BensType *array, int n)
{
    for(int i = 0; i < n; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}
