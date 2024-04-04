#include <Kokkos_Core.hpp>
#include <cstdio>
#include <iostream>

int main(int argc, char* argv[]) {
  Kokkos::initialize(argc, argv);
  {

  // create and initialize views
  int n = 1001;
  Kokkos::View<int*> original_view("myView", n);

  Kokkos::parallel_for("Loop1", original_view.extent(0), KOKKOS_LAMBDA (const int i) {
    original_view(i) = i * i;
  });
  Kokkos::fence();

  // create two additional views of same size and datatype
  Kokkos::View<int*> deep_copy_view("copy1_view", n);
  Kokkos::View<int*> parallel_for_view("copy2_view", n);

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
  std::cout << "Deep Copy Time: " << deep_copy_time << " seconds\n";
  std::cout << "Parallel For Copy Time: " << parallel_for_time << " seconds\n\n";

  //output comparison 
  double speedup_factor = parallel_for_time / deep_copy_time;
  std::cout << "Deep copy is " << speedup_factor << " times faster than parallel for\n\n";

  // compare deep_copy_view and parallel_for_view
  bool views_are_equal = true;
    for (int i = 0; i < n; ++i) {
      if (deep_copy_view(i) != parallel_for_view(i)) {
        views_are_equal = false;
          break;
      }
  }

  // output equality
  if (views_are_equal) {
    std::cout << "deep_copy_view and parallel_for_view are equal\n\n\n";
  } else {
     std::cout << "deep_copy_view and parallel_for_view are not equal\n\n\n";
  }


  }
  Kokkos::finalize();
}
