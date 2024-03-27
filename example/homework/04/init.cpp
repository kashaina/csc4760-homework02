#include <mpi.h>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
  int rank, size;
   
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  float sendbuf[1], recvbuf[1], buffer[0];
  recvbuf[0] = -555;
  sendbuf[0] = (float)rank;


  //i: demonstrate the use of MPI Bcast and MPI Reduce to achieve the same result as MPI Allreduce 
  
/*  // MPI_Bcast + MPI_Reduce
  MPI_Reduce(sendbuf, recvbuf, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
  if(rank == 0) {
    cout << "\nProblem 1";
    cout << "\n\nMPI_BCast + MPI_reduce\n";

    buffer[0] = recvbuf[0]; 
  }

  MPI_Bcast(buffer, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  cout << rank << ": " << buffer[0] << endl;
  
  MPI_Barrier(MPI_COMM_WORLD); // wait for MPI processes to complete

  // MPI_Allreduce
  
  recvbuf[0] = -555; // reset value
  sendbuf[0] = float(rank); // reset value

  if(rank == 0) {
    cout << "\nMPI_Allreduce\n";
  }

  MPI_Allreduce(sendbuf, recvbuf, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
  cout << rank << ": " << recvbuf[0] << endl;  
*/
  //ii: demonstrate the use of MPI_Gather and MPI_Bcast to achieve the same result as MPI_Allgather
  
  // MPI_Gather + MPI_Bcast
  recvbuf[0] = -555; // reset value
  sendbuf[0] = float(rank); // reset value
  //buffer[0] = recvbuf[0];
  //vector<float> new_buffer(atoi(argv[1])

  float sendbuf2[size], recvbuf2[size], buffer2[1];

  sendbuf2[size] = float(rank);

  MPI_Gather(sendbuf2, 1, MPI_FLOAT, recvbuf2, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

  if(rank == 0) {
    cout << "\n\n\nProblem 2";
    cout << "\n\nMPI_Gather/MPI_Bcast\n";
    MPI_Bcast(recvbuf2, size, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }
  
    // Output the received data on all processes
    printf("Process %d received data: ", rank);
    for (int i = 0; i < size; i++) {
        printf("%.2f ", recvbuf2[i]);
    }
    printf("\n");

  MPI_Barrier(MPI_COMM_WORLD);

  // MPI_Allgather
  /*recvbuf[0] = -555; // reset value
  sendbuf[0] = float(rank); // reset value
  buffer[0] = recvbuf[0];

  if(rank == 0) {
    cout << "\nMPI_Allgather\n";
  }

  MPI_Allgather(sendbuf, 1, MPI_FLOAT, buffer, 1, MPI_FLOAT, MPI_COMM_WORLD);

  // Print the allgathered data on all processes
  cout << rank << ": ";
  for(int i = 0; i < size; ++i) {
    cout << buffer[i] << " ";
  }
  cout << endl;
*/
  //iii: dmonstrate the use of MPI_Alltoall to send a personalized communication between each process in MPI_COMM_WORLD
/*  vector<float> sendBufferAlltoall(size, (float)rank);
  vector<float> recvBufferAlltoall(size);
  MPI_Alltoall(sendBufferAlltoall.data(), 1, MPI_FLOAT, recvBufferAlltoall.data(), 1, MPI_FLOAT, MPI_COMM_WORLD);
  
  cout << rank << " received: ";
  for(int i = 0; i < size; ++i){
    cout << recvBufferAlltoall[i] << " ";
  }
  cout << endl;
*/

  MPI_Finalize();
  return 0;
}

