#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

#define MAX_NUM 100000000

int isPrime(long num)
{
    if (num < 2) return 0;
    if (num == 2) return 1;
    if (num % 2 == 0) return 0;
    
    long sqrtNum = sqrt(num);
    for (long i = 3; i <= sqrtNum; i += 2)
    {
        if (num % i == 0) return 0;
    }
    return 1;
}

long calculateSum(long start, long end) {
    long sum = 0;
    for (long i = start; i <= end; i++)
    {
        if (isPrime(i) == 1)
            sum += i;
    }

    return sum;
}

int main() {
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);    
    long interval = (MAX_NUM / nproc);

    int pipes[nproc][2];
    pid_t pids[nproc];
    
    for (int i = 0; i < nproc; i++)
        pipe(pipes[i]);
    
    
    for (int i = 0; i < nproc; i++)
    {
        pids[i] = fork();
        
        
        if (pids[i] == 0)
        {
            close(pipes[i][0]);
            
            long start = i * interval + 1;
            long end = (i + 1) * interval;
            long intervalSum = calculateSum(start, end);
            write(pipes[i][1], &intervalSum, sizeof(intervalSum));
            
            close(pipes[i][1]);
            exit(0);
        }
        else
            close(pipes[i][1]);        
    }
    
    long total_sum = 0;
    long intervalSum;
    
    for (int i = 0; i < nproc; i++)
    {
        read(pipes[i][0], &intervalSum, sizeof(intervalSum));
        total_sum += intervalSum;
        
        printf("sum of interval %d: %ld\n", i, intervalSum);
        close(pipes[i][0]);
        
        waitpid(pids[i], NULL, 0);
    }
    
    printf("sum of all prime numbers in range: %ld\n", total_sum);
}