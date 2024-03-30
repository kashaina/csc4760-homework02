//4.2 Demonstrate the use of MPI_Gather and MPI_Bcast to achieve the same result as MPI_Allgather.
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
  float sendbuf[1], recvbuf[size];

  sendbuf[0] = (float)rank;
  recvbuf[0] = -555;

  MPI_Igather(sendbuf, 1, MPI_FLOAT, recvbuf, 1, MPI_FLOAT, 0, MPI_COMM_WORLD, &request[0]);
  MPI_Wait(&request[0], MPI_STATUSES_IGNORE);

  MPI_Ibcast(recvbuf, size, MPI_FLOAT, 0, MPI_COMM_WORLD, &request[1]);
  MPI_Wait(&request[1], MPI_STATUSES_IGNORE);

  cout << "Gather     |  Process: " << setw(2) << rank << "  |  Received: ";
  for (int i = 0; i < size; ++i) {
    cout << recvbuf[i] << " ";
  }
  cout << endl;

  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0){
    cout << std::endl;
  }


  //ii: MPI_Allgather
  recvbuf[0] = -555;

  MPI_Allgather(sendbuf, 1, MPI_FLOAT, recvbuf, 1, MPI_FLOAT, MPI_COMM_WORLD);
  cout << "Allgather  |  Process: " << setw(2) << rank << "  |  Received: ";
  for (int i = 0; i < size; ++i) {
    cout << recvbuf[i] << " ";
  }
  cout << endl;


  MPI_Finalize();
  return 0;
}

