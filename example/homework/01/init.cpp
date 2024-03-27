#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Request request;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int message = 0;
    int n = 5;

    for (int i = 0; i < n; i++) {
        // print and send very first message. separated because it does not receive
	if (rank == 0 && i == 0) {
            printf("%d: %d\n", rank, message);
            message++;
            MPI_Isend(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, MPI_STATUSES_IGNORE);
        } 
	
	// receive, print, and send for the reset of the messages
	else {
            MPI_Irecv(&message, 1, MPI_INT, (rank == 0) ? (size - 1) : (rank - 1), 0, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, MPI_STATUSES_IGNORE);
            printf("%d: %d\n", rank, message);
            message++;
            
	    // send message, unless if on the final process on the final ring
	    // or else it will deadlock bc it will not be received
	    if (i != n - 1 || rank != size - 1) {
               MPI_Isend(&message, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD, &request);
               MPI_Wait(&request, MPI_STATUSES_IGNORE);
            }

        }
    }

    MPI_Finalize();
    return 0;
}
