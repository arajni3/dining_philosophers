#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define PHILOSOPHER_NUM 5
#define MAX_MEALS 10
#define MAX_THINK_EAT_SEC 4

pthread_mutex_t mutex;
pthread_cond_t chopsticks[PHILOSOPHER_NUM];
pthread_t philosophers[PHILOSOPHER_NUM];

// number of meals eaten for each philosopher
int meals_eaten[PHILOSOPHER_NUM];

int philosopher_id[PHILOSOPHER_NUM];
enum {THINKING, HUNGRY, EATING} state[PHILOSOPHER_NUM];

// allotted run time from thread joining until run_time time has elapsed
int run_time;

// variable that, once set to time(NULL), will hold the time elapsed from Jan 1, 1970 (GMT)
// to the time that it (start) was set to time(NULL)
// Thus, after setting start to time(), the value time(NULL) - start will denote the amount of time elapsed
// from the time start was set to time(NULL) to the time that the second call to time() was made
time_t start;

void test(int philosopher_number) {
    int left = (philosopher_number - 1) % PHILOSOPHER_NUM;
    int right = (philosopher_number + 1) % PHILOSOPHER_NUM;
    if ((state[left] != EATING) && (state[philosopher_number] == HUNGRY) && (state[right] != EATING)) {
        state[philosopher_number] = EATING;
        pthread_cond_signal(chopsticks + philosopher_number);
    }
}
void pickup_forks(int philosopher_number) {
    pthread_mutex_lock(&mutex);
    state[philosopher_number] = HUNGRY;
    test(philosopher_number);

    while (state[philosopher_number] != EATING) {
        pthread_cond_wait(chopsticks + philosopher_number, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}
void return_forks(int philosopher_number) {
    pthread_mutex_lock(&mutex);
    state[philosopher_number] = THINKING;
    test((philosopher_number - 1) % PHILOSOPHER_NUM);
    test((philosopher_number + 1) % PHILOSOPHER_NUM);
    pthread_mutex_unlock(&mutex);
}
void *philo_operation(void *philosopher_number_ptr) {
    int philosopher_number = *((int*)philosopher_number_ptr);
    int time_sleeping;

    srandom((unsigned)time(NULL));
    // if more than run_time time has elapsed since joining the threads, exit this thread
    // then all subsequent threads will exit in this same manner
    while ((meals_eaten[philosopher_number] < MAX_MEALS) && (time(NULL) < start + run_time)) {
        time_sleeping = (int)((random() % MAX_THINK_EAT_SEC) + 1);

        // think
        sleep(time_sleeping);

        pickup_forks(philosopher_number);
        // eat
        printf("Philosopher %d is eating\n", philosopher_number);
        meals_eaten[philosopher_number]++;
        time_sleeping = (int)((random() % MAX_THINK_EAT_SEC) + 1);
        sleep(time_sleeping);

        // go back to thinking
        printf("Philosopher %d is thinking\n", philosopher_number);
        return_forks(philosopher_number);
    }
}
int main(int argc, char** argv) {
    run_time = atoi(argv[1]);

    for (int i = 0; i < PHILOSOPHER_NUM; ++i) {
        meals_eaten[i] = 0;
        state[i] = THINKING;
        philosopher_id[i] = i;
        pthread_cond_init(chopsticks + i, NULL);
    }
    pthread_mutex_init(&mutex, NULL);
    // pthread_mutex_lock(&mutex) to acquire mutex lock, pthread_mutex_unlock(&mutex) to release mutex lock
    for (int i = 0; i < PHILOSOPHER_NUM; ++i) {
        pthread_create(philosophers + i, 0, philo_operation, (void*)(philosopher_id + i));
    }
    start = time(NULL);
    for (int i = 0; i < PHILOSOPHER_NUM; ++i) {
        pthread_join(philosophers[i], NULL);
    }

    int minimum = MAX_MEALS, maximum = 0, average = 0;
    for (int i = 0; i < PHILOSOPHER_NUM; ++i) {
        if (meals_eaten[i] < minimum) {
            minimum = meals_eaten[i];
        }
        if (meals_eaten[i] > maximum) {
            maximum = meals_eaten[i];
        }
        average += meals_eaten[i];
    }
    average /= PHILOSOPHER_NUM;
    printf("Minimum number of meals eaten among the philosophers is %d\n", minimum);
    printf("Maximum number of meals eaten among the philosophers is %d\n", maximum);
    printf("Average number of meals eaten among the philosophers is %d\n", average);
    return 0;
}