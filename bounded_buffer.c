// Andy Lim and Ifunanya Okafor
// Course: CS 4440-03

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // for sleep()

#define BUFFER_SIZE 10
#define TOTAL_ITEMS 50  // total characters to produce

char buffer[BUFFER_SIZE];
int in = 0;   // next write index
int out = 0;  // next read index

pthread_mutex_t mutex;
sem_t empty;  // counts empty slots
sem_t full;   // counts filled slots

void *producer(void *arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        char item = 'A' + (i % 26);

        // Check if producer needs to wait
        int sval;
        sem_getvalue(&empty, &sval);
        if (sval == 0) {
            printf("Producer waiting (buffer full)...\n");
        }

        sem_wait(&empty);             // wait for empty slot
        pthread_mutex_lock(&mutex);   // enter critical section

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        int used = (in - out + BUFFER_SIZE) % BUFFER_SIZE;
        printf("Produced: %c, buffer slots used: %d\n", item, used);

        pthread_mutex_unlock(&mutex);
        sem_post(&full);              // signal full slot

        sleep(1); // simulate production time
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {

        // Check if consumer needs to wait
        int sval;
        sem_getvalue(&full, &sval);
        if (sval == 0) {
            printf("Consumer waiting (buffer empty)...\n");
        }

        sem_wait(&full);              // wait for item in buffer
        pthread_mutex_lock(&mutex);   // enter critical section

        char item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        int used = (in - out + BUFFER_SIZE) % BUFFER_SIZE;
        printf("Consumed: %c, buffer slots used: %d\n", item, used);

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);             // signal empty slot

        sleep(1); // simulate consumption time
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("All items produced and consumed.\n");
    return 0;
}
