#include <Kokkos_Core.hpp>
#include <cstdio>
#include <iostream>

int main(int argc, char* argv[]) {
  Kokkos::initialize(argc, argv);
  {

  // create and initialize views
  int n = 1001;
  Kokkos::View<double*> original_view("myView", n);

  Kokkos::parallel_for("Loop1", original_view.extent(0), KOKKOS_LAMBDA (const int i) {
    original_view(i) = rand() % 1000;
  });
  Kokkos::fence();

  // create two additional views of same size and datatype
  Kokkos::View<double*> deep_copy_view("copy1_view", n);
  Kokkos::View<double*> parallel_for_view("copy2_view", n);

  // deep_copy
  Kokkos::Timer deep_copy_timer;
  Kokkos::deep_copy(deep_copy_view, original_view);
  Kokkos::fence();
  double deep_copy_time = deep_copy_timer.seconds();

  // user copy
  Kokkos::Timer parallel_for_timer;
  Kokkos::parallel_for(n, KOKKOS_LAMBDA(const int i) {
    parallel_for_view(i) = original_view(i);
  });
  Kokkos::fence();
  double parallel_for_time = parallel_for_timer.seconds();

  // output times 
  std::cout << "Deep Copy Time: " << deep_copy_time << " seconds";
  std::cout << "\nParallel For Copy Time: " << parallel_for_time << " seconds\n\n\n";


  }
  Kokkos::finalize();
}
