#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#define N 1048576
#define M 16

// Merge function for merge sort by GFG
void merge(int shm_ptr[], int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
    
    int *L = malloc(n1 * sizeof(int));
    int *R = malloc(n2 * sizeof(int));
    
    for (i = 0; i < n1; i++)
        L[i] = shm_ptr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = shm_ptr[m + 1 + j];
    
    i = 0;
    j = 0;
    k = l;
    
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            shm_ptr[k] = L[i];
            i++;
        } else {
            shm_ptr[k] = R[j];
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        shm_ptr[k] = L[i];
        i++;
        k++;
    }
    
    while (j < n2) {
        shm_ptr[k] = R[j];
        j++;
        k++;
    }
    
    free(L);
    free(R);
}

// Merge sort implementation by GFG
void mergeSort(int shm_ptr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(shm_ptr, l, m);
        mergeSort(shm_ptr, m + 1, r);
        merge(shm_ptr, l, m, r);
    }
}

void mergeParts(int shm_ptr[], int start1, int end1, int start2, int end2)
{
    int *temp = malloc((end2 - start1 + 1) * sizeof(int));
    int i = start1, j = start2, k = 0;
    
    while (i <= end1 && j <= end2)
    {
        if (shm_ptr[i] <= shm_ptr[j])
            temp[k++] = shm_ptr[i++];
        else
            temp[k++] = shm_ptr[j++];
    }
    
    while (i <= end1)
        temp[k++] = shm_ptr[i++];
    
    while (j <= end2)
        temp[k++] = shm_ptr[j++];
    
    for (i = 0; i < k; i++)
        shm_ptr[start1 + i] = temp[i];
}

int main()
{
    int shm_fd;
    int* shm_ptr;
    size_t SIZE = N * sizeof(int);

    shm_fd = shm_open("q3_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    shm_ptr = (int*)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    srand(time(NULL));
    for (int i = 0; i < N; i++)
        shm_ptr[i] = rand() % 100 + 1;
    
    
    // for (int i = 0; i < N; i++)
    //     printf("%d ", shm_ptr[i]);
    // printf("\n\n");
    
    struct timespec tStart, tEnd;
    clock_gettime(CLOCK_MONOTONIC, &tStart);

    int partSize = N / M;
    pid_t pids[M];
    
    for (int i = 0; i < M; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            mergeSort(shm_ptr, i * partSize, (i + 1) * partSize - 1);
            exit(0);
        }
    }
    
    for (int i = 0; i < M; i++)
        waitpid(pids[i], NULL, 0);
    
    // for (int i = 0; i < N; i++)
    //     printf("%d ", shm_ptr[i]);
    // printf("\n\n");
    
    int eachPartSize = partSize;
    int currPartCnt = M;
    
    while (currPartCnt > 1)
    {
        int num_processes = currPartCnt / 2;
        pid_t merge_pids[num_processes];
        
        for (int i = 0; i < num_processes; i++)
        {
            merge_pids[i] = fork();
            if (merge_pids[i] == 0)
            {
                int start1 = i * 2 * eachPartSize;
                int end1 = start1 + eachPartSize - 1;
                int start2 = end1 + 1;
                int end2 = start2 + eachPartSize - 1;
                mergeParts(shm_ptr, start1, end1, start2, end2);
                exit(0);
            }
        }
        
        for (int i = 0; i < num_processes; i++)
            waitpid(merge_pids[i], NULL, 0);
        
        eachPartSize *= 2;
        currPartCnt /= 2;
    }
    

    clock_gettime(CLOCK_MONOTONIC, &tEnd);
    double elapsedTime = (tEnd.tv_sec - tStart.tv_sec) + 
                     (tEnd.tv_nsec - tStart.tv_nsec) / 1e9;
    printf("time taken with n: %d and m: %d:   %.9f seconds\n", N, M, elapsedTime);

    munmap(shm_ptr, SIZE);
    shm_unlink("q3_shm");
}