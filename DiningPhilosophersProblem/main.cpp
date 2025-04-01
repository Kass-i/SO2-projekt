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
uint inline left(uint i);
// Returns the number of the right neighbor of philosopher i
uint inline right(uint i);


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



int main(int argc, char * argv[]) {
    if(argc != 2) {
        cout << "You only need to enter the number of philosophers!! Try again...\n";
        cin.get();
        return 0;
    }
    philosophersAmount = stoi(argv[1]); // Take the number from a user
    cout << "The table consists of " << philosophersAmount << " philosophers\n";
    cout << "Press ENTER to start the program. You can also stop by pressing ENTER\n\n";
    cin.get();

    init();

    cin.get(); // Waiting for a user to stop the program
    dinnigRunning = false;
    for(uint i = 0; i < philosophersAmount; i++) {
        philosophersThreads[i].join();
    }

    cout << "Press ENTER to exit the program...";
    cin.get();
    releaseMemory();
    return 0;
}




// Returns a natural number from the range [min, max]
uint basicRand(uint min, uint max) {
    static mt19937 rnd(time(nullptr));
    return uniform_int_distribution<>(min, max)(rnd);
}



// Returns the number of the left neighbor of philosopher i
uint inline left(uint i) {
    return (i - 1 + philosophersAmount) % philosophersAmount;
}

// Returns the number of the right neighbor of philosopher i
uint inline right(uint i) {  
    return (i + 1) % philosophersAmount;
}



// Thinking ...
void think(uint i) {
    uint duration = basicRand(MIN_DURATION, MAX_DURATION);
    {
        lock_guard<mutex> coutLock(outputMtx);
        cout << i << " is thinking " << duration << "ms\n";
    }
    this_thread::sleep_for(chrono::milliseconds(duration));
}

// Philosopher i tries to eat
void takeForks(uint i) {
    {
        lock_guard<mutex> forkLock{forksMtx};
        state[i] = State::HUNGRY;
        {
            lock_guard<mutex> coutLock(outputMtx);
            cout << "\t\t\t" << i << " is hungry\n";
        }
        checkPhilosopher(i);  // Check if they can take forks
    }
    bothForksAvailable[i]->acquire();  // Block if forks were not acquired
}

// Eating ...
void eat(uint i) {
    uint duration = basicRand(MIN_DURATION, MAX_DURATION);
    {
        lock_guard<mutex> coutLock(outputMtx);
        cout << "\t\t\t\t\t" << i << " is eating " << duration << "ms\n";
    }
    this_thread::sleep_for(chrono::milliseconds(duration));
}

// Philosopher i finised eating
void putDownForks(uint i) { 
    lock_guard<mutex> forkLock{forksMtx};
    state[i] = State::THINKING;
    checkPhilosopher(left(i));   // Check if left neighbor can now eat
    checkPhilosopher(right(i));  // Check if right neighbor can now eat
}


// Check if philosopher i is hungry and they can use forks to eat
void checkPhilosopher(uint i) { 
    if (state[i] == State::HUNGRY && state[left(i)] != State::EATING && state[right(i)] != State::EATING) {
        state[i] = State::EATING;
        bothForksAvailable[i]->release(); // The philosopher can eat
    }
}

// Philosopher's doing their thing
void philosopher(uint id) {  
    while (dinnigRunning) {
        think(id);
        takeForks(id);
        eat(id);
        putDownForks(id);
    }
}


// Initialize states, forks and threads
void init() {
    bothForksAvailable = new binary_semaphore*[philosophersAmount];

    for(uint i = 0; i < philosophersAmount; i++) {
        state.push_back(State::THINKING);
        bothForksAvailable[i] = new binary_semaphore(0);
        philosophersThreads.emplace_back([&, i] { philosopher(i); });  // Create threads
    }
}

// Delete arrays
void releaseMemory() {
    for (uint i = 0; i < philosophersAmount; i++)
        delete bothForksAvailable[i];
    
    delete[] bothForksAvailable;
}