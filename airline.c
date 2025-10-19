// Names: Ifunanya Okafor and Andy Lim || Date: 11 Oct 2025 || Course: CS 4440-03
// Description: The purpose of this program is to emulate the operations of an unfair, but (hopefully) extermely inexpensive airline using 
// threads and semaphores. The program will create some number of threads B for baggage handlers, some number of threads S
// for security screeners, some number of threads F for flight attendants, and some number of threads P for passengers.
// The passengers each go through airline processing in stages: baggage -> security -> boarding. To ensure multiple threads aren't in one
// of these "critical" processing stages, we rely on the dependable blocking defense of semaphores and mutexes. After all the passengers have 
// successfuly boarded the aircraft and all the other workers (handlers, attendants, and screeners) have departed, the aircraft will commence takeoff.

// Compile Build: gcc -pthread -o airline airline.c
// Run:     ./airline P B S F (where P, B, S and F are unsigned integers [for example, ./airline 200 22 2 20])

// Libraries used
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Struct for Passengers
typedef struct Passenger {
    int id; // identification

    // Semaphores for critical processing stages
    sem_t baggage_done; 
    sem_t security_done;
    sem_t boarding_done;
} Passenger;

//Struct for Stages
typedef struct Stage {
    // Queue for Passengers with capacity = total passengers (as an upper bound)
    Passenger **buf;
    int capacity;
    int head;
    int tail;
    int count;

    // Mutex syncing for queue and cycle
    pthread_mutex_t mtx;
    pthread_cond_t  not_empty;

    // Progress accounting to know when all passengers are processed and workers can exit
    int processed;     // number processed so far
    int total;         // equals total P passengers
    bool done;         // set to true when processed == total
} Stage;

// Global count variables
static int P = 0, B = 0, S = 0, F = 0;

// Global Passenger struct
static Passenger *passengers = NULL;

// Static Stage structs
static Stage baggage_stage;
static Stage security_stage;
static Stage boarding_stage;

// Mutex guard to avoid interleaving/race condition from occurring
static pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;

// Method to safely print one line at a time
static void say(const char *fmt, ...) {
    va_list ap;
    pthread_mutex_lock(&print_mtx);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    fflush(stdout);
    pthread_mutex_unlock(&print_mtx);
}


// -------------- Stage Queuing helper methods -----------------------
static void stage_init(Stage *s, int total) {
    s->capacity = total;
    s->buf = (Passenger **)calloc((size_t)total, sizeof(Passenger*));
    s->head = s->tail = s->count = 0;
    pthread_mutex_init(&s->mtx, NULL);
    pthread_cond_init(&s->not_empty, NULL);
    s->processed = 0;
    s->total = total;
    s->done = false;
}

static void stage_destroy(Stage *s) {
    free(s->buf);
    pthread_mutex_destroy(&s->mtx);
    pthread_cond_destroy(&s->not_empty);
}

static void stage_enqueue(Stage *s, Passenger *p) {
    pthread_mutex_lock(&s->mtx);
    // Max capacity is set to p, so no full wait needed (at most P items can be outstanding per stage)
    s->buf[s->tail] = p;
    s->tail = (s->tail + 1) % s->capacity;
    s->count++;
    pthread_cond_signal(&s->not_empty);
    pthread_mutex_unlock(&s->mtx);
}

// Returns NULL when a stage is finished and workers should leave.
static Passenger* stage_dequeue(Stage *s) {
    pthread_mutex_lock(&s->mtx);
    while (s->count == 0 && !s->done) {
        pthread_cond_wait(&s->not_empty, &s->mtx); 
    }
    if (s->count == 0 && s->done) {
        pthread_mutex_unlock(&s->mtx);
        return NULL; // nothing left to complete
    }
    Passenger *p = s->buf[s->head];
    s->head = (s->head + 1) % s->capacity;
    s->count--;
    pthread_mutex_unlock(&s->mtx);
    return p;
}

static void stage_mark_processed(Stage *s) {
    pthread_mutex_lock(&s->mtx);
    s->processed++;
    if (s->processed == s->total) {
        s->done = true;
        // Wake up any workers blocked in dequeue so they can leave
        pthread_cond_broadcast(&s->not_empty);
    }
    pthread_mutex_unlock(&s->mtx);
}

// --------------- The actual airline workers -------------------

static void* baggage_handler(void *arg) {
    (void)arg;
    for (;;) {
        Passenger *p = stage_dequeue(&baggage_stage);
        if (!p) break; // stage complete, exit thread

      
        sem_post(&p->baggage_done);
        stage_mark_processed(&baggage_stage);
    }
    return NULL;
}

static void* security_screener(void *arg) {
    (void)arg;
    for (;;) {
        Passenger *p = stage_dequeue(&security_stage);
        if (!p) break;

        sem_post(&p->security_done);
        stage_mark_processed(&security_stage);
    }
    return NULL;
}

static void* flight_attendant(void *arg) {
    (void)arg;
    for (;;) {
        Passenger *p = stage_dequeue(&boarding_stage);
        if (!p) break;

        sem_post(&p->boarding_done);
        stage_mark_processed(&boarding_stage);
    }
    return NULL;
}

// ----------------- Passenger thread ------------

static void* passenger_thread(void *arg) {
    Passenger *me = (Passenger *)arg;

    // Arrival
    say("Passenger #%d arrived at the terminal.\n", me->id);

    // Baggage
    say("Passenger #%d is waiting at baggage processing for a handler.\n", me->id);
    stage_enqueue(&baggage_stage, me);
    sem_wait(&me->baggage_done); // block until a handler processes me

    // Security
    say("Passenger #%d is waiting to be screened by a screener.\n", me->id);
    stage_enqueue(&security_stage, me);
    sem_wait(&me->security_done);

    // Boarding
    say("Passenger #%d is waiting to board the plane by an attendant.\n", me->id);
    stage_enqueue(&boarding_stage, me);
    sem_wait(&me->boarding_done);

    // Seated
    say("Passenger #%d has been seated and relaxes.\n", me->id);
    return NULL;
}

// ----------- Main Method --------------

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s P B S F\n", prog);
    fprintf(stderr, "Example: %s 100 3 5 2\n", prog);
}

int main(int argc, char **argv) {
    if (argc != 5) { usage(argv[0]); return 1; }

    P = atoi(argv[1]);
    B = atoi(argv[2]);
    S = atoi(argv[3]);
    F = atoi(argv[4]);

    if (P <= 0 || B <= 0 || S <= 0 || F <= 0) {
        usage(argv[0]);
        return 1;
    }

    // Initialize critical processing stages
    stage_init(&baggage_stage, P);
    stage_init(&security_stage, P);
    stage_init(&boarding_stage, P);

    // Create the airline workers
    pthread_t *b_threads = (pthread_t*)calloc((size_t)B, sizeof(pthread_t));
    pthread_t *s_threads = (pthread_t*)calloc((size_t)S, sizeof(pthread_t));
    pthread_t *f_threads = (pthread_t*)calloc((size_t)F, sizeof(pthread_t));

    for (int i = 0; i < B; ++i) pthread_create(&b_threads[i], NULL, baggage_handler, NULL);
    for (int i = 0; i < S; ++i) pthread_create(&s_threads[i], NULL, security_screener, NULL);
    for (int i = 0; i < F; ++i) pthread_create(&f_threads[i], NULL, flight_attendant, NULL);

    // Create passengers last 
    passengers = (Passenger*)calloc((size_t)P, sizeof(Passenger));
    pthread_t *p_threads = (pthread_t*)calloc((size_t)P, sizeof(pthread_t));

    for (int i = 0; i < P; ++i) {
        passengers[i].id = i + 1;
        sem_init(&passengers[i].baggage_done, 0, 0);
        sem_init(&passengers[i].security_done, 0, 0);
        sem_init(&passengers[i].boarding_done, 0, 0);
    }

    for (int i = 0; i < P; ++i) {
        pthread_create(&p_threads[i], NULL, passenger_thread, &passengers[i]);
    }

    // Wait for all passengers to finish
    for (int i = 0; i < P; ++i) {
        pthread_join(p_threads[i], NULL);
    }

    // At this point, all passengers are seated and ready to go
    // Join workers so they can leave
    for (int i = 0; i < B; ++i) pthread_join(b_threads[i], NULL);
    for (int i = 0; i < S; ++i) pthread_join(s_threads[i], NULL);
    for (int i = 0; i < F; ++i) pthread_join(f_threads[i], NULL);

    say("All %d passengers are seated. The plane takes off!\n", P);

    // Behind-the-scenes sanitation of airport
    for (int i = 0; i < P; ++i) {
        sem_destroy(&passengers[i].baggage_done);
        sem_destroy(&passengers[i].security_done);
        sem_destroy(&passengers[i].boarding_done);
    }
    free(passengers);
    free(p_threads);

    free(b_threads);
    free(s_threads);
    free(f_threads);

    stage_destroy(&baggage_stage);
    stage_destroy(&security_stage);
    stage_destroy(&boarding_stage);

    return 0;
}
