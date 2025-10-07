# Banking System – Multithreaded Ledger Simulation

This project implements a **multithreaded banking system** that processes a ledger of transactions (deposits, withdrawals, and transfers) using multiple worker threads. It demonstrates thread synchronization with mutexes, proper deadlock prevention, and concurrent data access control in C++.

---

## Overview

The system simulates a bank that:
- Maintains multiple accounts, each protected by a mutex.  
- Loads a **ledger file** containing a list of transactions.  
- Spawns multiple **worker threads**, each processing ledger entries concurrently.  
- Ensures that all updates to accounts are atomic and thread-safe.

---

## Key Features

- Thread-safe **deposit**, **withdraw**, and **transfer** operations.  
- Use of **pthread mutexes** to prevent data races.  
- **Ledger-based transaction simulation** read from a file.  
- **Deadlock prevention** in transfers via ordered locking (lock smaller account ID first).  
- Detailed transaction logging and statistics (success/fail counts).  

---

## File Structure
```
banking-system/
├── include/
| ├── bank.h
│ └── ledger.h
├── src/
│ ├── bank.cpp
| ├── ledger.cpp
│ └── main.cpp
├── README.md
└── .gitignore
```
---

## How It Works

### 1. Bank Initialization
The program begins by creating a `Bank` object with **10 accounts**, each initialized with:
- A unique account ID.
- A starting balance of `0`.
- Its own **pthread mutex** for synchronization.

### 2. Ledger Loading
The system reads a ledger file, where each line represents one transaction:
<account> <other_account> <amount> <mode>

- `<account>`: the primary account ID.  
- `<other_account>`: the secondary account ID (used for transfers).  
- `<amount>`: the amount to deposit/withdraw/transfer.  
- `<mode>`: the operation type  
  - `0` = deposit  
  - `1` = withdraw  
  - `2` = transfer  

All ledger entries are stored in a global linked list (`std::list<Ledger>`).

### 3. Worker Threads
`InitBank()` spawns multiple worker threads (based on user input).  
Each thread:
- Dequeues a ledger entry under a **global ledger lock**.  
- Executes the corresponding transaction on the shared `Bank` object.  
- Logs success/failure messages atomically using the bank’s internal mutex.  

### 4. Transaction Operations

Each transaction from the ledger file corresponds to one of three types: **Deposit**, **Withdraw**, or **Transfer**.  
All operations are thread-safe and use per-account locking to prevent data races.

---

#### Deposit
```cpp
Bank::deposit(int workerID, int ledgerID, int accountID, int amount)
```
- Locks the target account to ensure exclusive access.
- Increases the balance by the specified amount.
- Logs the result using the macro `DEPOSITE_MSG()`:
  ```
  [ SUCCESS ] TID: 0, LID: 1, Acc: 2 DEPOSIT $100
  ```
- Always succeeds and returns `0`.

---

#### Withdraw
```cpp
Bank::withdraw(int workerID, int ledgerID, int accountID, int amount)
```
- Locks the account before modifying its balance.
- Checks whether the account has sufficient funds:
  - If yes: subtracts the amount and logs `[ SUCCESS ]`.
  - If no: logs `[ FAIL ]` and leaves the balance unchanged.
- Returns `0` on success, `-1` on failure.

Example:
```
[ SUCCESS ] TID: 1, LID: 4, Acc: 0 WITHDRAW $50
[ FAIL ]    TID: 2, LID: 5, Acc: 3 WITHDRAW $500
```

---

#### Transfer
```cpp
Bank::transfer(int workerID, int ledgerID, int srcID, int destID, unsigned int amount)
```
- Transfers funds between two accounts safely.
- Locks **both accounts** in ascending order by ID to avoid deadlock:
  ```cpp
  if (srcID < destID) { lock(src); lock(dest); }
  else { lock(dest); lock(src); }
  ```
- If the source account has enough funds:
  - Deducts from the source, adds to the destination, and logs `[ SUCCESS ]`.
- Otherwise:
  - Logs `[ FAIL ]` and leaves both balances unchanged.
- Invalid if transferring to the same account (`srcID == destID`).

Example:
```
[ SUCCESS ] TID: 3, LID: 10, Acc: 1 TRANSFER $200 TO Acc: 2
[ FAIL ]    TID: 0, LID: 11, Acc: 3 TRANSFER $1000 TO Acc: 4
```

---

## Logging Format

All messages are printed through macros in `bank.h` for consistent formatting:
- `DEPOSITE_MSG()`  
- `WITHDRAW_MSG()`  
- `TRANSFER_MSG()`

Example log sequence:
```
[ SUCCESS ] TID: 0, LID: 0, Acc: 0 DEPOSIT $500
[ SUCCESS ] TID: 1, LID: 2, Acc: 0 WITHDRAW $100
[ FAIL ]    TID: 2, LID: 5, Acc: 3 TRANSFER $300 TO Acc: 0
```

After processing, the system prints final account balances and transaction summary:
```
ID# 0 | 400
ID# 1 | 250
Success: 10  Fails: 5
```
