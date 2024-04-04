using namespace std;
#include <iostream>
#include <assert.h>
#include <string>

#include <mpi.h>

// forward declarations:

class Domain
{
public:
  Domain(int _M, int _N, const char *_name="") : domain(new char[(_M+1)*(_N+1)]), M(_M), N(_N), name(_name)  {}
  virtual ~Domain() {delete[] domain;}
  char &operator()(int i, int j) {return domain[i*N+j];}
  char operator()(int i, int j)const {return domain[i*N+j];}

  int rows() const {return M;}
  int cols() const {return N;}

  const string & myname() const {return name;}

protected:
  char *domain; 
  int M;
  int N;

  string name;
};

void zero_domain(Domain &domain);
void print_domain(Domain &domain);
void update_domain(Domain &new_domain, Domain &old_domain, int size, int myrank, MPI_Comm comm);
void parallel_code(int M, int N, int iterations, int size, int myrank, MPI_Comm comm);

int main(int argc, char **argv)
{
  int M, N;
  int iterations;

  if(argc < 4)
  {
    cout << "usage: " << argv[0] << " M N iterations" << endl;
    exit(0);
  }

  int size, myrank;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  int array[3];
  if(myrank == 0)
  {
     M = atoi(argv[1]); N = atoi(argv[2]); iterations = atoi(argv[3]);

     array[0] = M;
     array[1] = N;
     array[2] = iterations;
     
  }
  MPI_Bcast(array, 3, MPI_INT, 0, MPI_COMM_WORLD);
  if(myrank != 0)
  {
    M = array[0];
    N = array[1];
    iterations = array[2];
  }

  
  parallel_code(M, N, iterations, size, myrank, MPI_COMM_WORLD);
  
  MPI_Finalize();
}

void parallel_code(int M, int N, int iterations, int size, int myrank, MPI_Comm comm)
{
  int m = M / size; // perfect divisibility for this version
  int n = N; // 1D decomposition
  
  Domain even_domain(m,n,"even Domain");
  Domain odd_domain(m,n,"odd Domain");

  zero_domain(even_domain);
  zero_domain(odd_domain);

  // fill in even_domain with something meaningful (initial state)
  // this requires min size for default values to fit:
  if((n >= 8) && (m >= 10))
  {
    even_domain(0,(n-1)) = 1;
    even_domain(0,0)     = 1;
    even_domain(0,1)     = 1;
    
    even_domain(3,5) = 1;
    even_domain(3,6) = 1;
    even_domain(3,7) = 1;

    even_domain(6,7) = 1;
    even_domain(7,7) = 1;
    even_domain(8,7) = 1;
    even_domain(9,7) = 1;
  }

  // here is where I might print out my picture of the initial domain
  cout << "Initial:"<<endl; print_domain(even_domain);

  Domain *odd, *even; // pointer swap magic
  odd = &odd_domain;
  even = &even_domain;

  for(int i = 0; i < iterations; ++i)
  {
    update_domain(*odd, *even, size, myrank, comm);
    // here is where I might print out my picture of the updated domain
    cout << "Iteration #" << i << endl; print_domain(*odd);

    // swap pointers:
    Domain *temp = odd;
    odd  = even;
    even = temp;
  }

}

void zero_domain(Domain &domain)
{
  for(int i = 0; i < domain.rows(); ++i)
    for(int j = 0; j < domain.cols(); ++j)
      domain(i,j) = 0;
}

void print_domain(Domain &domain)
{
  cout << domain.myname() << ":" <<endl;
  // this is naive; it doesn't understand big domains at all 
  for(int i = 0; i < domain.rows(); ++i)
  {
    for(int j = 0; j < domain.cols(); ++j)
      cout << (domain(i,j) ? "*" : " ");
    cout << endl;
  }
}

void update_domain(Domain &new_domain, Domain &old_domain, int size, int myrank, MPI_Comm comm)
{
  MPI_Request request[4];
  
  int neighbor_count;
  int N = new_domain.cols();
  int m = new_domain.rows();

  char *top_row = new char[N];
  char *bottom_row = new char[N];

  char *top_halo = new char[N];
  char *bottom_halo = new char[N];

  // int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
  //             MPI_Comm comm, MPI_Request *request)

  // fill the top row
  for(int i = 0; i < N; ++i)
  {
      top_row[i] = old_domain(0,i);
  }
  MPI_Isend(top_row, N, MPI_CHAR, (myrank-1+size)%size, 0, comm, &request[0]);

  for(int i = 0; i < N; ++i)
  {
     bottom_row[i] = old_domain(m-1,i);
  }
  MPI_Isend(bottom_row, N, MPI_CHAR, (myrank+1)%size, 0, comm, &request[1]);

  // send my top row and bottom row to adjacent process
  // receive the halo from top and bottom process
  MPI_Irecv(top_halo, N, MPI_CHAR,    (myrank+size-1)%size, 0, comm, &request[2]);
  MPI_Irecv(bottom_halo, N, MPI_CHAR, (myrank+1)%size, 0, comm, &request[3]);

  MPI_Waitall(4, request, MPI_STATUSES_IGNORE); // complete all 4 transfers

  // at this point, I have halos from my neighbors

  // i=0:
  for(int j = 0; j < new_domain.cols(); ++j)
  {
      neighbor_count = 0;

      for(int delta_i = 0; delta_i <= 1; delta_i++)
      {
	for(int delta_j = -1; delta_j <= 1; delta_j++)
	{
	  if(delta_i == 0 && delta_j == 0) //skip self
	    continue;

	  // this first implementation is sequental and wraps the vertical
	  // and horizontal dimensions without dealing with halos (ghost cells)
	  if(old_domain((0+delta_i),
			(j+delta_j+old_domain.cols())%old_domain.cols()))
	     ++neighbor_count;
	}
      }
      for(int delta_j = -1; delta_j <= 1; delta_j++)
      {
        if(top_halo[(j+delta_j+old_domain.cols())%old_domain.cols()])
	  ++neighbor_count;
      }
      
      char mycell = old_domain(0,j);
      char newcell = 0;
      if(mycell == 0)
	newcell = (neighbor_count == 3) ? 1 : 0;
      else
	newcell = ((neighbor_count == 2)||(neighbor_count == 3)) ? 1 : 0;
      
      new_domain(0,j) = newcell;
  } // int j

  // i=m-1:
  for(int j = 0; j < new_domain.cols(); ++j)
  {
      neighbor_count = 0;

      for(int delta_i = -1; delta_i <= 0; delta_i++)
      {
	for(int delta_j = -1; delta_j <= 1; delta_j++)
	{
	  if(delta_i == 0 && delta_j == 0) //skip self
	    continue;

	  // this first implementation is sequental and wraps the vertical
	  // and horizontal dimensions without dealing with halos (ghost cells)
	  if(old_domain((m-1+delta_i),
			(j+delta_j+old_domain.cols())%old_domain.cols()))
	     ++neighbor_count;
	}
      }
      for(int delta_j = -1; delta_j <= 1; delta_j++)
      {
        if(bottom_halo[(j+delta_j+old_domain.cols())%old_domain.cols()])
	  ++neighbor_count;
      }
      
      char mycell = old_domain(m-1,j);
      char newcell = 0;
      if(mycell == 0)
	newcell = (neighbor_count == 3) ? 1 : 0;
      else
	newcell = ((neighbor_count == 2)||(neighbor_count == 3)) ? 1 : 0;
      
      new_domain(0,j) = newcell;
  } // int j

  // these update as in sequential case:
  for(int i = 1; i < (new_domain.rows()-1); ++i)
  {
    for(int j = 0; j < new_domain.cols(); ++j)
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
	  if(old_domain((i+delta_i+old_domain.rows())%old_domain.rows(),
			(j+delta_j+old_domain.cols())%old_domain.cols()))
	     ++neighbor_count;
	    
	}
      }
      char mycell = old_domain(i,j);
      char newcell = 0;
      if(mycell == 0)
	newcell = (neighbor_count == 3) ? 1 : 0;
      else
	newcell = ((neighbor_count == 2)||(neighbor_count == 3)) ? 1 : 0;
      
      new_domain(i,j) = newcell;
    } // int j
  } // int i


}

char* populate1(char *domain, int original_M, int M, int N, int rank) {
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

char* populate2(char *domain, int original_M, int M, int N, int rank) {
  if((N >= 3) && (original_M >= 3)){
  for(int i = 0; i < M; ++i){
    int global_row = rank * M + i;
    if (global_row < original_M){
      if (global_row == 0){
        domain[i*N + 1] = 1;
      }else if (global_row == 1){
        domain[i*N + 2] = 1;
      }else if (global_row == 3)){
        domain[i*N + 0] = 1;
	domain[i*N + 1] = 1;
	domain[i*N + 2] = 1;
        }
      }
    }
  }
  return domain;
}



