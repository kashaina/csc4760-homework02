using namespace std;
#include <iostream>
#include <assert.h>

// forward declarations:
void zero_domain(char *domain, int M, int N);
void print_domain(char *domain, int M, int N);
void update_domain(char *new_domain, char *old_domain, int M, int N);
char* populate_default(char *domain, int original_M, int M, int N, int rank);
char* populate_glider(char *domain, int original_M, int M, int N, int rank);
char* populate_bad_acorn(char *domain, int original_M, int M, int N, int rank);
char* populate_cap(char *domain, int original_M, int M, int N, int rank);
char* populate_mysnake(char *domain, int original_M, int M, int N, int rank);

int main(int argc, char **argv)
{
  char *even_domain = nullptr;
  char *odd_domain = nullptr;

  int M, N;
  int iterations;
  

  if(argc < 4)
  {
    cout << "usage: " << argv[0] << " M N iterations" << endl;
    exit(0);
  }
  
  M = atoi(argv[1]); N = atoi(argv[2]); iterations = atoi(argv[3]);
  
  even_domain = new char[M*N];
  odd_domain = new char[M*N];

  zero_domain(even_domain, M, N);
  zero_domain(odd_domain, M, N);

  // fill in even_domain with something meaningful (initial state)
  // this requires min size for default values to fit:
  
  //!!CHOOSE WHICH TEST YOU WANT TO RUN!!
  // even_domain = populate_default(even_domain, M, M, N, 0);
  even_domain = populate_glider(even_domain, M, M, N, 0);
  //even_domain = populate_bad_acorn(even_domain, M, M, N, 0);
  //even_domain = populate_cap(even_domain, M, M, N, 0);
  //even_domain = populate_mysnake(even_domain, M, M, N, 0);

  // here is where I might print out my picture of the initial domain
  cout << "Initial:"<<endl; print_domain(even_domain, M, N);

  for(int i = 0; i < iterations; ++i)
  {
    char *temp;
    update_domain(odd_domain, even_domain, M, N);
    // here is where I might print out my picture of the updated domain
    cout << "Iteration #" << i << endl; print_domain(odd_domain, M, N);

    // swap pointers:
    temp        = odd_domain;
    odd_domain  = even_domain;
    even_domain = temp;
  }

  // free dynamic memory:
  delete[] even_domain;
  delete[] odd_domain;

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

void update_domain(char *new_domain, char *old_domain, int M, int N)
{
  int neighbor_count;
  
  for(int i = 0; i < M; ++i)
  {
    for(int j = 0; j < N; ++j)
    {
      neighbor_count = 0;
      for(int delta_i = -1; delta_i <= 1; delta_i++)
      {
	for(int delta_j = -1; delta_j <= 1; delta_j++)
	{
	  if(delta_i == 0 && delta_j == 0) //skip self
	    continue;

	  // this first implementation is sequental and wraps the vertical
	  // and horizontal dimensions without dealing with halos (ghost cells)
	  if(old_domain[((i+delta_i+M)%M)*N+((j+delta_j+N)%N)])
	     ++neighbor_count;
	    
	}
      }
      char mycell = old_domain[i*N + j];
      char newcell = 0;
      if(mycell == 0)
	newcell = (neighbor_count == 3) ? 1 : 0;
      else
	newcell = ((neighbor_count == 2)||(neighbor_count == 3)) ? 1 : 0;
      
      new_domain[i*N + j] = newcell;
    } // int j
  } // int i
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
  if((N >= 10) && (original_M >= 5)){
  for(int i = 0; i < M; ++i){
    int global_row = rank * M + i;

    // places structure in the center
    int half_row = global_row / 2 - 1;
    int half_col = N / 2 - 3;

    if (global_row < original_M){
      if (global_row == half_row){
        domain[i*N + half_col + 1] = 1;
      }else if (global_row == half_row + 1){
        domain[i*N + half_col + 3] = 1;
      }else if (global_row == half_row + 2){
        domain[i*N + half_col] = 1;
        domain[i*N + half_col + 1] = 1;
        domain[i*N + half_col + 4] = 1;
        domain[i*N + half_col + 5] = 1;
	domain[i*N + half_col + 6] = 1;
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
