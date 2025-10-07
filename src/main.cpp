#include <ledger.h>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <num_of_threads> <leader_file>\n" << endl;
    exit(-1);
  }

  int p = atoi(argv[1]);
  InitBank(p, argv[2]);

  return 0;
}
