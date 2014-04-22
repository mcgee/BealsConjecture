#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include "ulhash3.h"
#include <time.h>
#include <cstdio>
#include <math.h>

#include "stateManager.h"
#include "bealSearcher.h"

std::tuple<int, int> getRange(int rank, int size, int xm)
{
  double xmax = (double)xm;

  double k = 4.0 / xmax;
  double v = sqrt(2.0 / k);
  double y10k = k * xmax * xmax / 2;
  double chunk = y10k / (double)size;
  double fromX = v * sqrt((double)rank * chunk);
  double toX = v * sqrt(((double)rank + 1.0) * chunk);

  return std::make_tuple<int, int>((int)fromX, (int)toX);
}

int main(int argc, char** argv) {
  //google::dense_hash_map<int, int> dmap;

  bool useMPI = true;
  bool useLogFile = true;

  if (argc < 2) {
    fprintf(stderr, "Usage: <max base> <max power>\n", argv[0]);
    exit(1);
  }

  int maxBase = atol(argv[1]);
  int maxPow = atol(argv[2]);

  int fromX;
  int toX;

  if (useMPI)
  {
    MPI::Status stat;
    MPI::Init (argc, argv);

    int size = MPI::COMM_WORLD.Get_size();
    int rank = MPI::COMM_WORLD.Get_rank();
    auto range = getRange(rank, size, maxBase);

    fromX = std::get<0>(range);
    toX = std::get<1>(range);
  }
  else
  {
    fromX = atol(argv[3]);
    toX = atol(argv[4]);
  }

  clock_t startClock = clock();

  std::cout << "from = " << fromX << std::endl;
  std::cout << "to = " << toX << std::endl;

  StateManager stateManager;
  SavedState defaultState(2, fromX, fromX, toX);
  SavedState state = defaultState;

  if (useLogFile)
  {
    SavedState state = stateManager.load(defaultState);

    std::ofstream logStream;
    logStream.open("logfile.txt", std::ios::app);

    logStream << "x_from=" << state.x_from << std::endl;
    logStream << "x_to=" << state.x_to << std::endl;
  }

  BealSearcher bealSearcher;
  Logger logger;

  std::cout << "Generating z^s..." << std::endl;
  auto hashtables = bealSearcher.genZs(2, 3, maxBase, maxPow, useLogFile);
    
  std::cout << "Done. Searching for candidates..." << std::endl;
  bealSearcher.checkSums(state.x, state.x_to, maxPow, hashtables);

  std::get<0>(hashtables).free();
  std::get<1>(hashtables).free();

  //getchar();

  if (useMPI)
  {
    std::cout << MPI::COMM_WORLD.Get_rank() << " finished!" << std::endl;
    MPI::Finalize ();
  }
  else
  {
    std::cout << "All done!" << std::endl;
    clock_t endClock = clock();
    std::cout << "Elapsed: " << (double)(endClock - startClock) / CLOCKS_PER_SEC << " seconds" << std::endl;
    getchar();
  }

  return 0;
}