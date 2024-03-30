// 4.3 Demonstrate the use of MPI Alltoall to send a personalized communication between each process in MPI_COMM WORLD
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
  int rank, size;
  MPI_Request request;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  vector<float> sendbuf(size), recvbuf(size);
  
  // initialize each sendbuf with numbers
  for (int i = 0; i < size; ++i) {
    sendbuf[i] = (float)(rank * size + i);
  }

  MPI_Alltoall(sendbuf.data(), 1, MPI_FLOAT, recvbuf.data(), 1, MPI_FLOAT, MPI_COMM_WORLD);

  cout << setw(3) << rank << ": ";
  for (int i = 0; i < size; ++i) {
    cout << setw(3) << recvbuf[i] << " ";
  }
  cout << endl;


  MPI_Finalize();
  return 0;
}
