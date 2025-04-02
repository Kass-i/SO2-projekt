#include "philosophers.hpp"
using namespace std;


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