#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <semaphore>
#include <vector>
#include <string>
using namespace std;

typedef unsigned int uint;

atomic_bool dinnigRunning = true;  // Running condition
uint philosophersAmount;  // Number of philosophers (and forks)
// Min and max time duration of actions
const uint MIN_DURATION = 1000;
const uint MAX_DURATION = 3000;

// States of philosophers
enum class State {
    THINKING = 0,
    HUNGRY = 1,  // Philosopher is trying to get forks
    EATING = 2,
};

vector<State> state;  // States of philosophers

// Mutual exclusion for critical regions
mutex forksMtx;   // actions with forks
mutex outputMtx;  // cout - printing status of philosopher

// Array of binary semaphors - one semaphore per philosopher.
// Acquired semaphore = philosopher i has taken two forks
binary_semaphore **bothForksAvailable;

vector<thread> philosophersThreads;  // Threads - philosophers


// Returns a natural number from the range [min, max]
uint basicRand(uint min, uint max);


// Returns the number of the left neighbor of philosopher i
uint inline left(uint i) {
    return (i - 1 + philosophersAmount) % philosophersAmount;
}

// Returns the number of the right neighbor of philosopher i
uint inline right(uint i) {  
    return (i + 1) % philosophersAmount;
}


// Thinking ...
void think(uint i);
// Philosopher i tries to eat
void takeForks(uint i);
// Eating ...
void eat(uint i);
// Philosopher i finished eating
void putDownForks(uint i);


// Check if philosopher i is hungry and they can use forks to eat
void checkPhilosopher(uint i);
// Philosopher's doing their thing
void philosopher(uint i);


// Initialize states, forks and threads
void init();
// Delete arrays
void releaseMemory();

#endif // PHILOSOPHERS_H