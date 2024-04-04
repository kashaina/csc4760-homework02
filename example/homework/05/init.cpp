#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <iostream>
#include <iomanip>

using namespace std;

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int Q = 4;

  if (rank == 0){
    cout << "\nWorld size: " << size;
    cout << "\nQ: 4\n";
  }

    // split communicator based on ranks divided by Q
    int color1 = rank / Q;
    MPI_Comm comm1;
    MPI_Comm_split(MPI_COMM_WORLD, color1, rank, &comm1);

    // split communicator based on ranks mod Q
    int color2 = rank % Q;
    MPI_Comm comm2;
    MPI_Comm_split(MPI_COMM_WORLD, color2, rank, &comm2);

    // get and print ranks and sizes of new communicators
    int rank1, rank2;
    MPI_Comm_rank(comm1, &rank1);
    MPI_Comm_rank(comm2, &rank2);

    cout << "World rank: " << setw(2) << rank << "  |  Comm1 rank: " << rank1 << "  |  Comm2 rank: " << rank2 << endl;

    MPI_Comm_free(&comm1);
    MPI_Comm_free(&comm2);


    MPI_Finalize();
    return 0;
}

