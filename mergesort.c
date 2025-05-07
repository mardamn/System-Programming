#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<time.h>
#define ARRAYNUM 10000000
#define TNUM 8
int arr[ARRAYNUM];

/*
Array of random 10000000 elements sorted.
In this code sorting took  3.366839 seconds.
In single threaded version of merge sort it took 5.856198 seconds to sort same sized array.
If array size is expanded, the difference in times of threaded and nonthreaded merge sort gets bigger.
*/

void merge(int left, int middle, int right)
{
    
    int n1 = middle - left + 1;
    int n2 =  right - middle;
    int i, j, k;

    int* m1 = malloc(sizeof(int) * n1 );
    int* m2 = malloc(sizeof(int) * n2 );

    for (i = 0; i < n1; i++)
        m1[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        m2[j] = arr[middle + 1+ j];

    i = 0; 
    j = 0; 
    k = left; 
    while (i < n1 && j < n2)
    {
        if (m1[i] <= m2[j])
        {
            arr[k] = m1[i];
            i++;
        }
        else
        {
            arr[k] = m2[j];
            j++;
        }
        k++;
    }
    while (i < n1)
    {
        arr[k] = m1[i];
        i++;
        k++;
    }

   
    while (j < n2)
    {
        arr[k] = m2[j];
        j++;
        k++;
    }

    free(m1);
    free(m2);
}

void mergeSort(int l, int r)
{
    if (l < r)
    {
        int m = l+(r-l)/2;
        mergeSort(l, m);
        mergeSort(m+1, r);
        merge(l, m, r);
    }
}

void* t_sort(void* arg)
{
  int *a = (int*)arg;
  int part = *a;

  int l = part * (ARRAYNUM / TNUM);
  int r = (part + 1) * (ARRAYNUM / TNUM) - 1;

  int m = l + (r - l) / 2;

  if (l < r) 
  {
    mergeSort(l, m);
    mergeSort(m + 1, r);
    merge(l, m, r);
}

}

int main()
{
    struct timeval start, end;
    double elapsed;

    for(int j=0;j<ARRAYNUM;j++){ 

        arr[j] = (rand() % 100);
    }

    gettimeofday(&start, NULL);

    int i, ar[TNUM];
    pthread_t threads[TNUM];


    for ( i = 0; i < TNUM; i++){
     ar[i] = i;
     pthread_create(&threads[i], NULL, t_sort, (void*)(ar + i));
    }  

    for (int i = 0; i < TNUM; i++)
        pthread_join(threads[i], NULL);

        int l=0,r=0;
        for(int j=TNUM/2;j>=1;j=j/2){
           l=0;
           r=ARRAYNUM/j;
           for(int i=0; i<j;i++){

            merge(l,(l+r-1)/2,r-1);
            
            l=r;
            if(r+r<ARRAYNUM) r=r+r;  
            else r=ARRAYNUM;
        }
    }

    gettimeofday(&end, NULL);
    long long start_sec, end_sec;
    start_sec = start.tv_sec * 1000000 + start.tv_usec;
    end_sec = end.tv_sec * 1000000 + end.tv_usec;
    elapsed = (double)(end_sec - start_sec) / 1000000.0;
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
