#ifndef _BANK_H
#define _BANK_H

#include <assert.h> /* for assert */
#include <pthread.h>
#include <semaphore.h> /* for sem */
#include <stdlib.h>    /* for atoi() and exit() */
#include <sys/wait.h>  /* for wait() */
#include <fstream>
#include <iostream> /* for cout */
#include <list>
#include <string>

using namespace std;

#define DEPOSITE_MSG(level, w, l, a, m)                                 \
  level + "TID: " + std::to_string(w) + ", LID: " + std::to_string(l) + \
      ", Acc: " + std::to_string(a) + " DEPOSIT $" + std::to_string(m)

#define WITHDRAW_MSG(level, w, l, a, m)                                 \
  level + "TID: " + std::to_string(w) + ", LID: " + std::to_string(l) + \
      ", Acc: " + std::to_string(a) + " WITHDRAW $" + std::to_string(m)

#define TRANSFER_MSG(level, w, l, a, o, m)                                \
  level + "TID: " + std::to_string(w) + ", LID: " + std::to_string(l) +   \
      ", Acc: " + std::to_string(a) + " TRANSFER $" + std::to_string(m) + \
      " TO Acc: " + std::to_string(o)

using namespace std;

#define SUCC \
  std::string { "[ SUCCESS ] " }
#define ERR \
  std::string { "[ FAIL ] " }

struct Account {
  unsigned int accountID;
  long balance;
  pthread_mutex_t lock;
};

class Bank {
 private:
  int num;
  int num_succ;
  int num_fail;

 public:
  Bank(int N);
  ~Bank();  // destructor

  int deposit(int workerID, int ledgerID, int accountID, int amount);
  int withdraw(int workerID, int ledgerID, int accountID, int amount);
  int transfer(int workerID, int ledgerID, int src_id, int dest_id,
               unsigned int amount);

  void print_account();
  void recordSucc(string message);
  void recordFail(string message);

  pthread_mutex_t bank_lock;
  struct Account *accounts;
};

#endif