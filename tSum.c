/** 
 * file: tSum.c 
 * author:  Michael Mathews 
 * course: CSI 3336 
 * assignment: Project 9 
 * due date:  5/1/2023 
 * 
 * date modified: 5/1/2023 
 *      - file created 
 * 
 * makes a specified number of threads and adds up values in a file using them.
 * prints the sum to the screen
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


#define MAX_THREADS 128

typedef struct {
    int start;
    int end;
    int *total;
    sem_t *sem;
    int *array;
} threadData;

/** 
 * thread_function 
 * 
 * adds numbers together passed from a file
 * 
 * Parameters:
 *      arg - threadData with information for the current thread 
 * 
 * Output: 
 *      return: none 
 */
void *thread_function(void *arg) {
    threadData *data = arg;
    int i, sum = 0;
    for (i = data->start; i < data->end; i++) {
        sum += data->array[i];
    }
    sem_wait(data->sem);
    *data->total += sum;
    sem_post(data->sem);

    return 0;
}

/** 
 * print 
 *                                         
 * prints a number to the screen using only system calls
 * 
 * Parameters:
 *      total - the number to be printed to the screen
 * 
 * Output: 
 *      return: none 
 */
void print(int total){
    int remainder, length = 0, tens = 1, negative = 0, i;
    
    write(1, "Total: ", 7);
    if(total < 0){ 
        total *= -1;
        write(1, "-", 1);
    }
    
    do {
        length++;
        tens *= 10;
    } while(total / tens != 0);
    
    char *num = malloc(length * sizeof(char));
    tens = 10;
    
    for(i = length-1; i >= 0; i--){
        remainder = total % tens;
        remainder /= tens/10;
        num[i] = remainder + '0';
        total -= remainder;
        tens *= 10;
    }

    for(i = 0; i < length; i++){
        write(1, &num[i], 1);
    }
    write(1, "\n", 1);
    free(num);
}
    

int main(int argc, char** argv) {
    //open file
    int fd;
    if (argc != 3) {
        write(1, "Usage: <filename> <number>\n", 27);
        return 1;
    }
    fd = open(argv[1], 0);
    if (fd == -1) {
        write(1, "Error: file not open\n", 21);
        return 1;
    }
    //get make array for numbers
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    int *array = malloc(size);
    int num = atoi(argv[2]);
    //read in file to int array
    read(fd, array, size);
    
    //initialize semaphore and threads
    pthread_t *threads = malloc(num * sizeof(pthread_t));
    int i, total = 0;
    sem_t sem;
    sem_init(&sem, 0, 1);

    //add values with threads
    if(num < 1){
        write(1, "Error: enter a higher number\n", 29);
        return 1;
    } 

    // divide work among threads
    int chunkSize = size/4/num, remainder = size/4 % num;
    threadData *data = malloc(num * sizeof(threadData));
    for (i = 0; i < num; i++) {
        data[i].start = i * chunkSize;
        if(i < remainder){
            data[i].start += i;
        } else { data[i].start += remainder; }
        data[i].end = data[i].start + chunkSize;
        if(i < remainder){
            data[i].end++;
        }
        data[i].total = &total;
        data[i].sem = &sem;
        data[i].array = array;
        pthread_create(&threads[i], NULL, thread_function, (void *)&data[i]);
    }
    for (i = 0; i < num; i++) {
        pthread_join(threads[i], NULL);
    }
    //delete semaphore and free memory
    sem_destroy(&sem);
    free(threads);
    free(data);
    
    //print output
    print(total);
    return 0;
}
