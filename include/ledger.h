#ifndef _LEDGER_H
#define _LEDGER_H

#include <bank.h>

#ifdef DEBUGMODE
#define debug(msg) \
  std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << msg << std::endl;
#else
#define debug(msg)
#endif

using namespace std;

#define D 0
#define W 1
#define T 2

const int SEED_RANDOM = 377;

struct Ledger {
  int acc;
  int other;
  int amount;
  int mode;
  int ledgerID;
};

extern list<struct Ledger> ledger;
extern Bank *bank;

void InitBank(int num_workers, char *filename);
int load_ledger(char *filename);
void *worker(void *unused);

#endif