// Andy Lim and Ifunanya Okafor
// Course: CS 4440-03
// Project 2 - Mother Hubbard Problem

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define CHILDREN 12

// Global variables
int cycles;
int current_day = 0;
pthread_mutex_t mother_mutex;
sem_t father_sem;

// Mother thread function
void* mother_thread(void* arg) {
    for (int day = 1; day <= cycles; day++) {
        // Mother waits (blocks) until she can proceed
        pthread_mutex_lock(&mother_mutex);
        
        current_day = day;
        
        // Mother performs all tasks for each child
        for (int child = 1; child <= CHILDREN; child++) {
            printf("Child #%d is being fed breakfast.\n", child);
            usleep(100); // Small delay to slow down output
        }
        
        for (int child = 1; child <= CHILDREN; child++) {
            printf("Child #%d is being sent to school.\n", child);
            usleep(100);
        }
        
        for (int child = 1; child <= CHILDREN; child++) {
            printf("Child #%d is being given dinner.\n", child);
            usleep(100);
        }
        
        for (int child = 1; child <= CHILDREN; child++) {
            printf("Child #%d is being given a bath.\n", child);
            usleep(100);
        }
        
        // Mother signals Father to wake up
        printf("\nMother is going to sleep and waking up Father to take care of the children.\n");
        sem_post(&father_sem);
    }
    
    return NULL;
}

// Father thread function
void* father_thread(void* arg) {
    for (int day = 1; day <= cycles; day++) {
        // Father waits (blocks) until Mother signals him
        sem_wait(&father_sem);
        
        // Father reads books and tucks children in
        for (int child = 1; child <= CHILDREN; child++) {
            printf("Child #%d is being read a book.\n", child);
            usleep(100);
        }
        
        printf("\nThis is day #%d of a day in the life of Mother Hubbard.\n", current_day);
        
        // Father signals Mother to wake up for next day
        printf("\nFather is going to sleep and waking up Mother to take care of the children.\n");
        pthread_mutex_unlock(&mother_mutex);
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <number_of_cycles>\n", argv[0]);
        printf("Example: ./mh 100\n");
        return 1;
    }
    
    // Get number of cycles from command line
    cycles = atoi(argv[1]);
    
    if (cycles <= 0) {
        printf("Error: Number of cycles must be positive.\n");
        return 1;
    }
    
    // Initialize mutex (Mother starts unlocked/available)
    pthread_mutex_init(&mother_mutex, NULL);
    
    // Initialize semaphore (Father starts blocked, value = 0)
    sem_init(&father_sem, 0, 0);
    
    // Create thread IDs
    pthread_t mother_tid, father_tid;
    
    // At start: Mother is awake, Father is asleep
    printf("Father is going to sleep and waking up Mother to take care of the children.\n");
    
    // Launch Mother thread
    pthread_create(&mother_tid, NULL, mother_thread, NULL);
    
    // Launch Father thread
    pthread_create(&father_tid, NULL, father_thread, NULL);
    
    // Wait for both threads to complete
    pthread_join(mother_tid, NULL);
    pthread_join(father_tid, NULL);
    
    // Clean up
    pthread_mutex_destroy(&mother_mutex);
    sem_destroy(&father_sem);
    
    return 0;
}
