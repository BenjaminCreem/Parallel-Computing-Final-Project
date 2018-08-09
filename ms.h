#define BensType int

void sort(BensType *a, int n);
void TopDownMergeSort(BensType *a, BensType *b, int n);
void TopDownSplitMerge(BensType *B, int iBegin, int iEnd, BensType *A);
void TopDownMerge(BensType *A, int iBegin, int iMiddle, int iEnd, BensType *B);
void CopyArray(BensType *A, int iBegin, int iEnd, BensType *B);
void printArray(BensType *array, int n);
