#include <Kokkos_Core.hpp>
#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  Kokkos::initialize(argc, argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = 10;

  Kokkos::View<double*> myView("myView", n);

  // initialize and send view for rank = 0
  if (rank == 0) {
    for (int i = 0; i < n; i++) {
      myView(i) = (double)(i);
    }
    std::cout << "Process: " << rank << "  |  Sent view:     ";
    for (int i = 0; i < n; i++) {
      std::cout << myView(i) << " ";
    }
    std::cout << std::endl;
  
  std::vector<MPI_Request> requests(size - 1);
 // send View
   for (int i = 1; i < size; i++) {
     MPI_Send(myView.data(), n, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
    }
  }

  // receive View for rank = 1
  else {
    MPI_Recv(myView.data(), n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
    std::cout << "Process: " << rank << "  |  Received view: ";
    for (int i = 0; i < n; i++) {
      std::cout << myView(i) << " ";
    }
    std::cout << std::endl << std::endl;
  }

  myView = Kokkos::View<double*>(); //deallocate view explicitly

  Kokkos::finalize();
  MPI_Finalize();
}

