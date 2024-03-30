
//4.1 Demonstrate the use of MPI_Bcast and MPI_Reduce to achieve the same result as MPI_Allreduce
#include <mpi.h>
#include <iomanip>
#include <iostream>
using namespace std;
int main(int argc, char **argv)
{
  int rank, size;
  MPI_Request request[2];
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  float sendbuf[1], recvbuf[1], buffer[1];
  recvbuf[0] = -555;
  sendbuf[0] = (float)rank;

  //i: MPI_Reduce + MPI_Bcast
  MPI_Ireduce(sendbuf, recvbuf, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD, &request[0]);
  MPI_Waitall(1, &request[0], MPI_STATUSES_IGNORE);
  if(rank == 0) {
    buffer[0] = recvbuf[0];
  }

  MPI_Ibcast(buffer, 1, MPI_FLOAT, 0, MPI_COMM_WORLD, &request[1]);
  MPI_Waitall(1, &request[1], MPI_STATUSES_IGNORE);

  cout << "Reduce     |  Process: " << setw(2) << rank << "  |  Sum: " << buffer[0] << endl;;
  
  MPI_Barrier(MPI_COMM_WORLD);

  //ii: MPI_Allreduce
  recvbuf[0] = -555; // reset value
  sendbuf[0] = float(rank); // reset value

  MPI_Iallreduce(sendbuf, recvbuf, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD, &request[0]);
  MPI_Waitall(1, &request[0], MPI_STATUSES_IGNORE);

  cout << "Allreduce  |  Process: " << setw(2) << rank << "  |  Sum: " << buffer[0] << endl;;
  
  MPI_Finalize();
  return 0;
}

