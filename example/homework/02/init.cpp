#include <iostream>
#include <iomanip>
#include <assert.h>
#include <mpi.h>

#include <vector>
#include <algorithm>

using namespace std;

// forward declarations:
void zero_domain(char *domain, int M, int N);
void print_domain(char *domain, int M, int N, int rank, int iter);
void update_domain(char *new_domain, char *old_domain, int M, int N, int rank, int size);

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

  if(argc < 4) {
    if (rank == 0) {
      cout << "usage: " << argv[0] << " M N iterations" << endl;
    }
    MPI_Finalize();
    exit(0);
  }
  
  original_M = atoi(argv[1]); //rows
  N = atoi(argv[2]); //columns. this will not be divided among processes
  iterations = atoi(argv[3]);
 
  // divides M by size and distributes remainders if needed
  M = original_M / size;
  if (rank < (original_M % size)){
    M += 1;
  }
  
  even_domain = new char[M*N];
  odd_domain = new char[M*N];

  zero_domain(even_domain, M, N);
  zero_domain(odd_domain, M, N);

  // fill in even_domain with something meaningful (initial state)
  // this requires min size for default values to fit:
  // calculate and fill using the global row
  if((N >= 8) && (original_M >= 10)){
    for(int i = 0; i < M; ++i){
      int global_row = rank * M + i;
      if (global_row < original_M){
        if (global_row == 0){
          even_domain[i*N + (N-1)] = 1;
          even_domain[i*N + 0] = 1;
          even_domain[i*N + 1] = 1;
        }else if (global_row == 3){
          even_domain[i*N + 5] = 1;
          even_domain[i*N + 6] = 1;
          even_domain[i*N + 7] = 1;
        }else if (global_row == 6 || (global_row >= 7 && global_row <= 9)){
           even_domain[i*N + 7] = 1;
        }
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // prints picture in order by row using mpi_gather
  if (rank == 0) {
    global_domain = new char[size * M * N];
  }
  MPI_Gather(even_domain, M * N, MPI_CHAR, global_domain, M * N, MPI_CHAR, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    cout << "Initial: " << endl;
    for (int j = 0; j < size; ++j) {
      print_domain(&global_domain[j * M * N], M, N, j, -1);
    }
    delete[] global_domain;
  }  

  // iterate
  for(int i = 0; i < iterations; ++i)
  {
    char *temp;
    char *global_domain;

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
        print_domain(&global_domain[j * M * N], M, N, j, i);
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


void print_domain(char *domain, int M, int N, int rank, int iter)
{
  // this is naive; it doesn't understand big domains at all
  for(int i = 0; i < M; ++i)
  {
    cout << setw(2) << rank << ": ";
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
  int global_north = (rank == 0) ? size - 1 : (rank - 1);
  int global_south = (rank == size - 1) ? 0 : (rank + 1);

  MPI_Barrier(MPI_COMM_WORLD);
  //cout << "\nRank: " << rank << "  |  North: " << global_north << "  |  South: " << global_south << endl;

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Isend(&old_domain[0], N, MPI_CHAR, global_north, 0, MPI_COMM_WORLD, &request[0]); //sends top column to north neighbor
  MPI_Irecv(ghost_cells_south, N, MPI_CHAR, global_south, 0, MPI_COMM_WORLD, &request[1]); //receives ghost_cells_south row from south neighbor
  MPI_Isend(&old_domain[(M - 1) * N], N, MPI_CHAR, global_south, 0, MPI_COMM_WORLD, &request[2]); //sends bottom row to south neighbor
  MPI_Irecv(ghost_cells_north, N, MPI_CHAR, global_north, 0, MPI_COMM_WORLD, &request[3]); //receives ghost_cells_north row from north neighbor
  MPI_Waitall(4, request, MPI_STATUSES_IGNORE); 
  
  /*cout << "Process " << rank << " - Ghost Cells North:";
  for (int i = 0; i < N; ++i) {
    //cout << ((ghost_cells_north[i]) ? "*" : " ");
    cout << ghost_cells_north[i];
  }
  cout << endl;

  cout << "Process " << rank << " - Ghost Cells South:";
  for (int i = 0; i < N; ++i) {
    cout << ((ghost_cells_south[i]) ? "*" : " ");
  }
  cout << endl;
  */
  char *warped_domain = new char[(M + 2) * (N + 2)];

  
  for (int i = 0; i < (M + 2); i++) {
    for (int j = 0; j < (N + 2); j++) {
      if (i == 0 && j == 0) { // top-left corner -> right column of ghost_cells_north
        warped_domain[i * (N + 2) + j] = (ghost_cells_north[N - 1]) ? 1 : 0;
      }
      else if (i == 0 && j == N + 1) { // top-right corner -> left column of ghost_cells_north
        warped_domain[i * (N + 2) + j] = (ghost_cells_south[0]) ? 1 : 0;
      }
      else if (i == M + 1 && j == 0) { // bottom-left corner -> right column of ghost_cells_south
        warped_domain[i * (N + 2) + j] = (ghost_cells_south[N - 1]) ? 1 : 0;
      }
      else if (i == M + 1 && j == N + 1) { // bottom-right corner -> left column of ghost_cells_south
        warped_domain[i * (N + 2) + j] = (ghost_cells_north[0]) ? 1 : 0;
      }
      else if (i == 0) { // top row excluding corners -> ghost_cells_north
        warped_domain[i * (N + 2) + j] = (ghost_cells_north[j - 1]) ? 1 : 0;
      }
      else if (i == M + 1) { // bottom row excluding corners -> ghost_cells_south
        warped_domain[i * (N + 2) + j] = (ghost_cells_south[j - 1]) ? 1 : 0;
      }
      else if (j == 0) { // left column excluding corners -> right column of old_domain
        warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N + (N - 1)]) ? 1 : 0;
      }
      else if (j == N + 1) { // right column excluding corners -> left column of old_domain
        warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N]) ? 1 : 0;
      }
      else { // inner domain -> old_domain
        warped_domain[i * (N + 2) + j] = (old_domain[(i - 1) * N + (j - 1)]) ? 1 : 0;
      }
    }
  }

  /*MPI_Barrier(MPI_COMM_WORLD);
  cout << "\nNew domain: " << rank << endl;
  for(int i = 0; i < M + 2; ++i)
  {
    cout << setw(2) << rank << " " << setw(2) << i << "GHOST:";
    for(int j = 0; j < N + 2; ++j)
      cout << ((warped_domain[i * (N + 2) + j]) ? "*" : "X");
    cout << endl;
  }

  print_domain(warped_domain, M + 2, N + 2, rank, 0);
  MPI_Barrier(MPI_COMM_WORLD);
  */

  /*cout << "\nInner part of the domain (excluding ghost cells):" << rank << endl;
  for(int i = 1; i <= M; ++i) {
    cout << setw(2) << rank << " " << setw(2) << i << "GHOST: ";
    for(int j = 1; j <= N; ++j)
        cout << ((warped_domain[i * (N + 2) + j]) ? "*" : "X");
    cout << endl;
  }*/

  for (int i = 1; i <= M; ++i){
    for (int j = 1; j <= N; ++j){
      neighbor_count = 0;
      
      for(int delta_i = -1; delta_i <= 1; delta_i++){
	for(int delta_j = -1; delta_j <= 1; delta_j++){
	  if(delta_i == 0 && delta_j == 0) //skip self
	    continue;

	  if(warped_domain[((i+delta_i+M)%M)*N+((j+delta_j+N)%N)])
	     ++neighbor_count;
	    
	}
      }
      char mycell = warped_domain[i*N + j];
      char newcell = 0;
      if(mycell == 0)
	newcell = (neighbor_count == 3) ? 1 : 0;
      else
	newcell = ((neighbor_count == 2)||(neighbor_count == 3)) ? 1 : 0;
      //new_domain[i * N + j] = newcell;
      new_domain[(i - 1) * N + (j - 1)] = newcell;
    }
  }

  delete[] ghost_cells_north;
  delete[] ghost_cells_south;
}
