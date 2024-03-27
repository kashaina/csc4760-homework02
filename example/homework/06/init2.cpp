
#include <Kokkos_Core.hpp>
#include <mpi.h>
#include <cstdio>
int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  Kokkos::initialize(argc, argv);
  {
  // Make View and init values
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  Kokkos::View<double*> = myView("myView", 10);
  if (rank == 0) {
    Kokkos::parallel_for("initialize view", Kokkos::RangePolicy<>(0, N), KOKKOS_LAMBDA(int i) {
      myView(i) = i * i;
    });
  }
  // Send View values with MPI functions
  MPI_Barrier(MPI_COMM_WORLD); // Synchronize MPI processes
  if (rank == 1) {
    // Allocate memory to receive the View
    double* recvData = new double[N];
    // Receive the View data from rank 0
    MPI_Status status;
    MPI_Recv(recvData, N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
    // Print out the received View
    printf("Received View on rank %d:\n", rank);
    for (int i = 0; i < N; ++i) {
      printf("%f ", recvData[i]);
    }
    printf("\n");
    delete[] recvData;
  }
  // Output
  printf("\nhello world\n");
  }
  Kokkos::finalize();
  MPI_Finalize();
}
