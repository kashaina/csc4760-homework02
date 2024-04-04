#include <iostream>
#include <iomanip>
#include <assert.h>
#include <mpi.h>

#include <vector>
#include <algorithm>

using namespace std;

// forward declarations:
void zero_domain(char *domain, int M, int N);
void print_domain(char *domain, int M, int N);
void update_domain(char *new_domain, char *old_domain, int M, int N, int rank, int size);
char* populate_default(char *domain, int original_M, int M, int N, int rank);
char* populate_glider(char *domain, int original_M, int M, int N, int rank);
char* populate_bad_acorn(char *domain, int original_M, int M, int N, int rank);
char* populate_cap(char *domain, int original_M, int M, int N, int rank);
char* populate_mysnake(char *domain, int original_M, int M, int N, int rank);
int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  MPI_Request request;
  MPI_Status status;

  char *even_domain = nullptr;
  char *odd_domain = nullptr;
  char *global_domain = nullptr;

  int original_M, M, N;
  int iterations;
  int option;

  if(argc < 4) {
    if (rank == 0) {
      cout << "usage: " << argv[0] << " M N iterations" << endl << endl;
    }
    MPI_Finalize();
    exit(0);
  }

  original_M = atoi(argv[1]); //rows
  N = atoi(argv[2]); //columns. this will not be divided among processes
  iterations = atoi(argv[3]);
 
  if(original_M < size){
    if (rank == 0){
      cout << "More rows than processes. Error" << endl << endl;
    }
    MPI_Finalize();
    exit(0);
  }

  if (original_M % size != 0){
    if (rank == 0){
      cout << "Please have M be divisible by number of processes. Error" << endl << endl;
    }
    MPI_Finalize();
    exit(0);
  }

  // divides rows evenly between the processes
  M = original_M / size;
  
  even_domain = new char[M*N];
  odd_domain = new char[M*N];

  zero_domain(even_domain, M, N);
  zero_domain(odd_domain, M, N);

  // fill in even_domain with something meaningful (initial state)
  // this requires min size for default values to fit:
  
  //!!CHOOSE WHICH TEST YOU WANT TO RUN!!
  //even_domain = populate_default(even_domain, original_M, M, N, rank);
  even_domain = populate_glider(even_domain, original_M, M, N, rank);
  //even_domain = populate_bad_acorn(even_domain, original_M, M, N, rank);
  //even_domain = populate_cap(even_domain, original_M, M, N, rank);
  //even_domain = populate_mysnake(even_domain, original_M, M, N, rank);

  // prints picture in order by row using mpi_gather
  if (rank == 0) {
    global_domain = new char[size * M * N];
  }
  MPI_Gather(even_domain, M * N, MPI_CHAR, global_domain, M * N, MPI_CHAR, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    cout << "Initial: " << endl;
    for (int j = 0; j < size; ++j) {
      print_domain(&global_domain[j * M * N], M, N);
    }
    delete[] global_domain;
  }  

  // iterate
  for(int i = 0; i < iterations; ++i)
  {
    char *temp;
    char *global_domain;

    // calculates neighbors and updates domain
    update_domain(odd_domain, even_domain, M, N, rank, size);
    MPI_Barrier(MPI_COMM_WORLD);

    // prints picture in order by row using mpi_gather
    global_domain = nullptr;
    if (rank == 0) {
      global_domain = new char[size * M * N];
    }
    MPI_Gather(odd_domain, M * N, MPI_CHAR, global_domain, M * N, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
      cout << "Iteration #" << i << endl;
      for (int j = 0; j < size; ++j) {
        print_domain(&global_domain[j * M * N], M, N);
      }
      delete[] global_domain;
    }

    // swap pointers:
    temp = odd_domain;
    odd_domain  = even_domain;
    even_domain = temp;
  }

  // free dynamic memory:
  delete[] even_domain;
  delete[] odd_domain;

  MPI_Finalize();
  return 0;
}


void zero_domain(char *domain, int M, int N)
{
  for(int i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      domain[i*N+j] = 0;
}


void print_domain(char *domain, int M, int N)
{
  // this is naive; it doesn't understand big domains at all
  for(int i = 0; i < M; ++i)
  {
    for(int j = 0; j < N; ++j)
      cout << ((domain[i*N+j]) ? "*" : " ");
    cout << endl;
  }
}

void update_domain(char *new_domain, char *old_domain, int M, int N, int rank, int size)
{
  MPI_Status status;
  MPI_Request request[4];
  int neighbor_count;

  char *ghost_cells_south = new char[N];
  char *ghost_cells_north = new char[N];
  int global_north = (rank == 0) ? size - 1 : (rank - 1); //calculates which process is above it
  int global_south = (rank == size - 1) ? 0 : (rank + 1); //calculates which process is below it
  char *warped_domain = new char[(M + 2) * (N + 2)];

  // sends and receives bottom and top rows as ghost cells
  MPI_Isend(&old_domain[0], N, MPI_CHAR, global_north, 0, MPI_COMM_WORLD, &request[0]); //sends top column to north neighbor
  MPI_Irecv(ghost_cells_south, N, MPI_CHAR, global_south, 0, MPI_COMM_WORLD, &request[1]); //receives ghost_cells_south row from south neighbor
  MPI_Isend(&old_domain[(M - 1) * N], N, MPI_CHAR, global_south, 0, MPI_COMM_WORLD, &request[2]); //sends bottom row to south neighbor
  MPI_Irecv(ghost_cells_north, N, MPI_CHAR, global_north, 0, MPI_COMM_WORLD, &request[3]); //receives ghost_cells_north row from north neighbor
  MPI_Waitall(4, request, MPI_STATUSES_IGNORE); 
  MPI_Barrier(MPI_COMM_WORLD);

  // creates a copy of old_domain with an extra edge on each side with ghost cells
  for (int i = 0; i < (M + 2); i++) {
    for (int j = 0; j < (N + 2); j++) {
      if (i == 0 && j == 0) { // top-left corner -> rightmost cell of ghost_cells_north
        warped_domain[i * (N + 2) + j] = (ghost_cells_north[N - 1]) ? 1 : 0;
      }
      else if (i == 0 && j == N + 1) { // top-right corner -> leftmost cell of ghost_cells_north
        warped_domain[i * (N + 2) + j] = (ghost_cells_north[0]) ? 1 : 0;
      }
      else if (i == M + 1 && j == 0) { // bottom-left corner -> rightmost cell of ghost_cells_south
        warped_domain[i * (N + 2) + j] = (ghost_cells_south[N - 1]) ? 1 : 0;
      }
      else if (i == M + 1 && j == N + 1) { // bottom-right corner -> leftmost cell of ghost_cells_south
        warped_domain[i * (N + 2) + j] = (ghost_cells_south[0]) ? 1 : 0;
      }
      else if (i == 0) { // top row excluding corners -> ghost_cells_north
        warped_domain[i * (N + 2) + j] = (ghost_cells_north[j - 1]) ? 1 : 0;
      }
      else if (i == M + 1) { // bottom row excluding corners -> ghost_cells_south
        warped_domain[i * (N + 2) + j] = (ghost_cells_south[j - 1]) ? 1 : 0;
      }
      else if (j == 0) { // left column excluding corners -> rightmost column of old_domain
        warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N + (N - 1)]) ? 1 : 0;
      }
      else if (j == N + 1) { // right column excluding corners -> leftmost column of old_domain
        warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N]) ? 1 : 0;
      }
      else { // inner domain -> old_domain
        warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N + (j - 1)]) ? 1 : 0;
	      warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N + (j - 1)]) ? 1 : 0;
      }
    }
  }

  // iterate through only the INNER WARPED DOMAIN since that is what we want to calculate
  for (int i = 1; i <= M; ++i){
    for (int j = 1; j <= N; ++j){
      neighbor_count = 0;
      
      // check neighbors (because we are iterating only through inner warped domain, each cell iterated through has neighbors)
      for(int delta_i = -1; delta_i <= 1; delta_i++){
	for(int delta_j = -1; delta_j <= 1; delta_j++){
	  if(delta_i == 0 && delta_j == 0) //skip self
	    continue;
	  if (warped_domain[(i + delta_i) * (N + 2) + (j + delta_j)] == 1)
            ++neighbor_count;
	}
      }
      char mycell = warped_domain[i * (N + 2) + j];
      char newcell = 0;
      if(mycell == 0)
	newcell = (neighbor_count == 3) ? 1 : 0;
      else
	newcell = ((neighbor_count == 2)||(neighbor_count == 3)) ? 1 : 0;
      new_domain[(i - 1) * N + (j - 1)] = newcell;
    }
  }

  delete[] ghost_cells_north;
  delete[] ghost_cells_south;
}

char* populate_default(char *domain, int original_M, int M, int N, int rank) {
  if((N >= 8) && (original_M >= 10)){
  for(int i = 0; i < M; ++i){
    int global_row = rank * M + i;
    if (global_row < original_M){
      if (global_row == 0){
        domain[i*N + (N-1)] = 1;
        domain[i*N + 0] = 1;
        domain[i*N + 1] = 1;
      }else if (global_row == 3){
        domain[i*N + 5] = 1;
        domain[i*N + 6] = 1;
        domain[i*N + 7] = 1;
      }else if (global_row == 6 || (global_row >= 7 && global_row <= 9)){
        domain[i*N + 7] = 1;
        }
      }
    }
  }
  return domain;
}

char* populate_glider(char *domain, int original_M, int M, int N, int rank) {
  if((N >= 3) && (original_M >= 3)){
  for(int i = 0; i < M; ++i){
    int global_row = rank * M + i;
    if (global_row < original_M){
      if (global_row == 0){
        domain[i*N + 1] = 1;
      }else if (global_row == 1){
        domain[i*N + 2] = 1;
      }else if (global_row == 2){
        domain[i*N + 0] = 1;
        domain[i*N + 1] = 1;
        domain[i*N + 2] = 1;
        }
      }
    }
  }
  return domain;
}

char* populate_bad_acorn(char *domain, int original_M, int M, int N, int rank) {
  if((N >= 60) && (original_M >= 55)){
  for(int i = 0; i < M; ++i){
    int global_row = rank * M + i;

    if (global_row < original_M){
      if (global_row == 50){
        domain[i*N + 51] = 1;
      }else if (global_row == 51){
        domain[i*N + 54] = 1;
      }else if (global_row == 52){
        domain[i*N + 50] = 1;
        domain[i*N + 51] = 1;
        domain[i*N + 54] = 1;
        domain[i*N + 55] = 1;
	domain[i*N + 56] = 1;
        }
      }
    }
  }
  return domain;
}

char* populate_cap(char *domain, int original_M, int M, int N, int rank) {
  if((N >= 13) && (original_M >= 13)){
  for(int i = 0; i < M; ++i){
    int global_row = rank * M + i;

    if (global_row < original_M){
      if (global_row == 9){
        domain[i*N + 10] = 1;
	domain[i*N + 11] = 1;
      }else if (global_row == 10){
        domain[i*N + 9] = 1;
	domain[i*N + 12] = 1;
      }else if (global_row == 11){
        domain[i*N + 9] = 1;
        domain[i*N + 10] = 1;
        domain[i*N + 11] = 1;
        domain[i*N + 12] = 1;
        }
      }
    }
  }
  return domain;
}

char* populate_mysnake(char *domain, int original_M, int M, int N, int rank) {
  if((N >= 7) && (original_M >= 1)){
    for(int i = 0; i < M; ++i){
      domain[i*N] = 1;
      domain[i*N + 3] = 1;
      domain[i*N + 4] = 1;
      domain[i*N + 7] = 1;
    }
  }
  return domain;
}
