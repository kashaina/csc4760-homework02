//4.3 Demonstrate the use of MPI_Alltoall to send a personalized communication between each process in MPI_COMM_WORLD
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<float> sendbuf(size), recvbuf(size);

    for (int i = 0; i < size; ++i) {
        sendbuf[i] = (float)(rank * size + i);
    }

    cout << "Process: " << setw(2) << rank << "  |  Original: ";
    for (int i = 0; i < size; ++i) {
        cout << setw(3) << sendbuf[i] << " ";
    }
    cout << endl;

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0){
      cout << std::endl;
    }

    MPI_Request request;
    MPI_Ialltoall(sendbuf.data(), 1, MPI_FLOAT, recvbuf.data(), 1, MPI_FLOAT, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);

    cout << "Process: " << setw(2) << rank << "  |  Received: ";
    for (int i = 0; i < size; ++i) {
        cout << setw(3) << recvbuf[i] << " ";
    }
    cout << endl;

    MPI_Finalize();
    return 0;
}

