// Andy Lim and Ifunanya Okafor
// Course: CS 4440-03
// Project 2 - Airline Passengers Problem

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Global counts
int P, B, S, F;
int passengers_completed = 0;

// Semaphores
sem_t baggage_ready;      // passengers signal when ready for baggage
sem_t baggage_done;       // baggage handlers signal after done

sem_t security_ready;     // baggage handlers signal security can start
sem_t security_done;      // security screeners signal after done

sem_t boarding_ready;     // security screeners signal boarding can start
sem_t boarding_done;      // attendants signal after done

// Mutexes
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Passenger threads
void* passenger_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);

    pthread_mutex_lock(&print_mutex);
    printf("Passenger #%d arrived at the terminal.\n", id);
    pthread_mutex_unlock(&print_mutex);

    // Passenger enters baggage line
    pthread_mutex_lock(&print_mutex);
    printf("Passenger #%d is waiting at baggage processing for a handler.\n", id);
    pthread_mutex_unlock(&print_mutex);

    sem_post(&baggage_ready);    // signal a baggage handler
    sem_wait(&baggage_done);     // wait until baggage done

    pthread_mutex_lock(&print_mutex);
    printf("Passenger #%d is waiting to be screened by a screener.\n", id);
    pthread_mutex_unlock(&print_mutex);

    sem_post(&security_ready);   // signal a security screener
    sem_wait(&security_done);    // wait until screening done

    pthread_mutex_lock(&print_mutex);
    printf("Passenger #%d is waiting to board the plane by an attendant.\n", id);
    pthread_mutex_unlock(&print_mutex);

    sem_post(&boarding_ready);   // signal a flight attendant
    sem_wait(&boarding_done);    // wait until boarding done

    pthread_mutex_lock(&print_mutex);
    printf("Passenger #%d has been seated and relaxes.\n", id);
    pthread_mutex_unlock(&print_mutex);

    pthread_mutex_lock(&count_mutex);
    passengers_completed++;
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

// Baggage handler threads
void* baggage_handler_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);

    while (1) {
        sem_wait(&baggage_ready);  // wait for a passenger needing baggage
        pthread_mutex_lock(&count_mutex);
        if (passengers_completed >= P) {
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        pthread_mutex_unlock(&count_mutex);

        pthread_mutex_lock(&print_mutex);
        printf("Baggage handler #%d: processing passenger baggage.\n", id);
        pthread_mutex_unlock(&print_mutex);

        usleep(50000); // simulate processing time

        sem_post(&baggage_done);   // signal passenger baggage done
    }
    return NULL;
}

// Security screener threads
void* security_screener_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);

    while (1) {
        sem_wait(&security_ready);
        pthread_mutex_lock(&count_mutex);
        if (passengers_completed >= P) {
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        pthread_mutex_unlock(&count_mutex);

        pthread_mutex_lock(&print_mutex);
        printf("Security screener #%d: screening passenger.\n", id);
        pthread_mutex_unlock(&print_mutex);

        usleep(50000);
        sem_post(&security_done);
    }
    return NULL;
}

// Flight attendant threads
void* attendant_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);

    while (1) {
        sem_wait(&boarding_ready);
        pthread_mutex_lock(&count_mutex);
        if (passengers_completed >= P) {
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        pthread_mutex_unlock(&count_mutex);

        pthread_mutex_lock(&print_mutex);
        printf("Flight attendant #%d: seating passenger.\n", id);
        pthread_mutex_unlock(&print_mutex);

        usleep(50000);
        sem_post(&boarding_done);
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <P> <B> <S> <F>\n", argv[0]);
        return 1;
    }

    P = atoi(argv[1]);
    B = atoi(argv[2]);
    S = atoi(argv[3]);
    F = atoi(argv[4]);

    if (P <= 0 || B <= 0 || S <= 0 || F <= 0) {
        fprintf(stderr, "Error: all arguments must be > 0\n");
        return 1;
    }

    // Initialize semaphores
    sem_init(&baggage_ready, 0, 0);
    sem_init(&baggage_done, 0, 0);
    sem_init(&security_ready, 0, 0);
    sem_init(&security_done, 0, 0);
    sem_init(&boarding_ready, 0, 0);
    sem_init(&boarding_done, 0, 0);

    // Create handler, screener, and attendant threads
    pthread_t handlers[B], screeners[S], attendants[F];
    for (int i = 0; i < B; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&handlers[i], NULL, baggage_handler_thread, id);
    }

    for (int i = 0; i < S; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&screeners[i], NULL, security_screener_thread, id);
    }

    for (int i = 0; i < F; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&attendants[i], NULL, attendant_thread, id);
    }

    // Create passenger threads
    pthread_t passengers[P];
    for (int i = 0; i < P; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&passengers[i], NULL, passenger_thread, id);
        usleep(10000);
    }

    // Wait for all passengers to finish
    for (int i = 0; i < P; i++) {
        pthread_join(passengers[i], NULL);
    }

    pthread_mutex_lock(&print_mutex);
    printf("\nAll %d passengers have been processed.\nPlane takes off!\n", passengers_completed);
    pthread_mutex_unlock(&print_mutex);

    // Cleanup
    sem_destroy(&baggage_ready);
    sem_destroy(&baggage_done);
    sem_destroy(&security_ready);
    sem_destroy(&security_done);
    sem_destroy(&boarding_ready);
    sem_destroy(&boarding_done);

    pthread_mutex_destroy(&print_mutex);
    pthread_mutex_destroy(&count_mutex);

    return 0;
}
