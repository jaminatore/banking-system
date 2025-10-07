#include "../include/ledger.h"
#include "../include/bank.h"
#include <sstream>

using namespace std;


pthread_mutex_t ledger_lock;

list<struct Ledger> ledger;
Bank *bank;

/**
 * @brief Initializes a banking system with a specified number of worker threads
 * and a ledger file.
 *
 * This function sets up a banking system with a specified number of worker
 * threads and loads a ledger from the given file. It then creates and
 * initializes the necessary threads to perform banking operations concurrently.
 * After all threads have completed their tasks, it prints the final state of
 * the bank's accounts.
 *
 * @attention
 * - Initialize the bank with 10 accounts.
 * - If `load_ledger()` fails, exit and free allocated memory.
 * - Be careful how you pass the thread ID to ensure the value does not get
 * changed.
 * - Don't forget to join all created threads.
 *
 * @param num_workers The number of worker threads to be created for concurrent
 * operations.
 * @param filename    The name of the file containing the ledger to be loaded
 * into the bank.
 */
void InitBank(int num_workers, char *filename) {
  // initialize bank
  bank = new Bank(10); 
  // load_ledger fails, exit and free memory
  if (load_ledger(filename) != 0) {
    delete bank;
    return; 
  }
  // create array of workers
  pthread_t* workers = new pthread_t[num_workers];
  // initialize threads
  for (int i = 0; i < num_workers; i++) {
    void* id = (void*)(intptr_t) i; 
    pthread_create(&workers[i], NULL, worker, id);
  }
  // join threads at the end
  for (int i = 0; i < num_workers; i++) {
    int id = i; 
    pthread_join(workers[id], NULL);
  }
  // print balances
  bank->print_account(); 
  // free memory
  delete bank; 
  delete[] workers;
}

/**
 * @brief Loads a ledger from a specified file into the banking system.
 *
 * This function reads transaction data from the given file, where each line
 * represents a ledger entry. The format is as follows:
 *   - Account (int): the account number
 *   - Other (int): for transfers, the other account number; otherwise not used
 *   - Amount (int): the amount to deposit, withdraw, or transfer
 *   - Mode (Enum): 0 for deposit, 1 for withdraw, 2 for transfer
 * The function then creates ledger entries and appends them to the ledger list
 * of the banking system.
 *
 * @attention
 * - If the file cannot be opened, the function returns -1, indicating failure.
 * - The function expects a specific file format as indicated above.
 * - Each line in the file corresponds to a ledger entry.
 * - The ledgerID starts with 0.
 *
 * @param filename The name of the file containing the ledger data.
 * @return 0 on success, -1 on failure to open the file.
 */
int load_ledger(char *filename) {
  // load file and initialize variables
  int ledgerID = 0;
  ifstream file(filename);
  string current_line;
  // cant open file
  if (!file.is_open()) { return -1; }
  // while there are valid lines
  while (getline(file, current_line)) {
    // new entry object
    Ledger current_entry;
    istringstream iss(current_line);
    // if all entries are valid, append
    if (iss >> current_entry.acc >> current_entry.other >> current_entry.amount >> current_entry.mode) {
        current_entry.ledgerID = ledgerID++; 
        ledger.push_back(current_entry); 
    } 
  }
  // close and return if successful
  file.close();
  return 0;
}

/**
 * @brief Worker function for processing ledger entries concurrently.
 *
 * This function represents a worker thread responsible for processing ledger
 * entries from the global ledger list. Each worker is assigned a unique ID, and
 * they dequeue ledger entries one by one, performing deposit, withdraw, or
 * transfer operations based on the entry's mode. Workers continue processing
 * until the ledger is empty.
 *
 * @attention
 * - The workerID is a unique identifier assigned to each worker thread. Ensure
 * proper dereferencing.
 * - The function uses a mutex (ledger_lock) to ensure thread safety while
 * accessing the global ledger.
 * - It continuously dequeues ledger entries, processes them, and updates the
 * bank's state accordingly.
 * - The worker handles deposit (D), withdraw (W), and transfer (T) operations
 * based on the ledger entry's mode.
 *
 * @param workerID A pointer to the unique identifier of the worker thread.
 * @return NULL after completing ledger processing.
 */
void *worker(void *workerID) {
  // type casting
  int id = (int) (intptr_t) workerID; 
  // grab entries from ledger
  while (1) {
    // entry object
    Ledger current_entry; 
    // check if empty
    if (ledger.empty()) { return NULL; }
    // crit section + entry object + update ledger
    pthread_mutex_lock(&ledger_lock);
    current_entry = ledger.front(); 
    ledger.pop_front(); 
    // unlock
    pthread_mutex_unlock(&ledger_lock); 
    // deposit case
    if (current_entry.mode == D) {
      bank->deposit(id, current_entry.ledgerID, current_entry.acc, current_entry.amount); 
    }
    // withdraw case
    else if (current_entry.mode == W) {
      bank->withdraw(id, current_entry.ledgerID, current_entry.acc, current_entry.amount);
    }
    // transfer case
    else {
      bank->transfer(id, current_entry.ledgerID, current_entry.acc, current_entry.other, current_entry.amount);
    }
  }
  // return after success 
  return NULL; 
}
