#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // check if the number of processes is 24, or exit
    if (size != 24) {
	if (rank == 0) {
            printf("\nPlease try again with 24 processes.\n\n");
	}
	MPI_Finalize();
        return 0;
    }

    int P = 6;
    int Q = 4;

    if (rank == 0){
        printf("\nWorld size: 24\n");
	printf("P: 6\n");
	printf("Q: 4\n");
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

    printf("World rank: %2d  |  Comm1 rank: %d  |  Comm2 rank: %d\n", rank, rank1, rank2);

    MPI_Comm_free(&comm1);
    MPI_Comm_free(&comm2);


    MPI_Finalize();
    return 0;
}

