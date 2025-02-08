#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define M 5
#define N 5
#define K 3
#define L 3

#define RESULT_ROWS ((M + K - 1) / K)
#define RESULT_COLS ((N + L - 1) / L)

int** makeGrid()
{
    int** M1 = (int**)malloc(M * sizeof(int*));
    for (int i = 0; i < M; i++)
        M1[i] = (int*)malloc(N * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++)
            M1[i][j] = rand() % 20 - 10;
    }
    return M1;
}

int findMax(int** matrix, int xStart, int xEnd, int yStart, int yEnd)
{
    int max = matrix[xStart][yStart];
    
    for (int i = xStart; i < xEnd && i < M; i++)
    {
        for (int j = yStart; j < yEnd && j < N; j++)
        {
            if (matrix[i][j] > max)
                max = matrix[i][j];
        }
    }

    return max;
}

void printMatrix(int** matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
            printf("%3d ", matrix[i][j]);
            
        printf("\n");
    }
}

int main() {
    int** M1 = makeGrid();
    printf("M1:\n");
    printMatrix(M1, M, N);
    
    int shm_fd;
    int* shm_ptr;
    size_t SIZE = RESULT_ROWS * RESULT_COLS * sizeof(int);

    shm_fd = shm_open("q2_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    shm_ptr = (int*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    for (int i = 0; i < RESULT_ROWS; i++)
    {
        for (int j = 0; j < RESULT_COLS; j++)
        {
            pid_t pid = fork();
            
            if (pid == 0)
            {
                int xStart = i * K;
                int xEnd = xStart + K;
                int yStart = j * L;
                int yEnd = yStart + L;
                
                int max = findMax(M1, xStart, xEnd, yStart, yEnd);
                
                shm_ptr[i * RESULT_COLS + j] = max;
                exit(0);
            }
        }
    }
    
    for (int i = 0; i < RESULT_ROWS * RESULT_COLS; i++)
        wait(NULL);
    
    printf("\n");
    for (int i = 0; i < RESULT_ROWS; i++)
    {
        for (int j = 0; j < RESULT_COLS; j++)
            printf("%3d ", shm_ptr[i * RESULT_COLS + j]);

        printf("\n");
    }
    
    munmap(shm_ptr, SIZE);
    shm_unlink("q2_shm");
    close(shm_fd);
}
