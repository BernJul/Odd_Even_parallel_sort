#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <chrono>


int main (int argc, char **argv){

    MPI_Init(&argc, &argv); 

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_elements; // number of elements to be sorted
    
    num_elements = atoi(argv[1]); // convert command line argument to num_elements

    int elements[num_elements]; // store elements
    int sorted_elements[num_elements]; // store sorted elements

    if (rank == 0) { // read inputs from file (master process)
        std::ifstream input(argv[2]);
        int element;
        int i = 0;
        while (input >> element) {
            elements[i] = element;
            i++;
        }
        std::cout << "actual number of elements:" << i << std::endl;
    }

    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::duration<double> time_span;
    if (rank == 0){ 
        t1 = std::chrono::high_resolution_clock::now(); // record time
    }

    /* TODO BEGIN
        Implement parallel odd even transposition sort
        Code in this block is not a necessary. 
        Replace it with your own code.
        Useful MPI documentation: https://rookiehpc.github.io/mpi/docs
    */


    int host_name_len, loc_arr_len, temp_var, temp_var_2;
 char host_name[MPI_MAX_PROCESSOR_NAME];

 MPI_Get_processor_name(host_name, &host_name_len);


 int* send_count = (int*) malloc(world_size * sizeof(int));
 int* dis_loc = (int*) malloc(world_size * sizeof(int));


 loc_arr_len = num_elements/world_size + (rank<num_elements%world_size);
 temp_var = 0;
 for(int i = 0; i<world_size; i++){
  send_count[i] = num_elements/world_size + (i<num_elements%world_size);
  dis_loc[i] = temp_var;
  temp_var += send_count[i];
 }

 int* locArr = (int*) malloc(loc_arr_len * sizeof(int));


 //distribute to other processes
 temp_var = MPI_Scatterv(elements, send_count, dis_loc, MPI_INT, locArr, loc_arr_len, MPI_INT, 0, MPI_COMM_WORLD);
 if(temp_var != MPI_SUCCESS){
  return -1;
 }
 
 //sort local array
 bool even = 0;
 for(int i = 0; i<num_elements; i++){
  for(int j = (even+dis_loc[rank])%2+1; j<loc_arr_len; j+=2){
   if(locArr[j]<locArr[j-1]){
    temp_var = locArr[j];
    locArr[j] = locArr[j-1];
    locArr[j-1] = temp_var;
   }
  }

/* 
Processors with data at the ends of their arrays that dont match the odd-even cycle will need to send data 
to the previous processor and wait for the previous one to process data and send it back
*/

  //send data to previous processes
  if(rank!=0 && (even!=(dis_loc[rank]%2))){
   temp_var = MPI_Send(&locArr[0], 1, MPI_INT, rank-1, i, MPI_COMM_WORLD);
   if(temp_var != MPI_SUCCESS){
                return -1;
         }
  }

  //receive, compare, and send the data from and to next process
  if(rank!=world_size-1 && (even!=(dis_loc[rank+1]%2))) {
   temp_var = MPI_Recv(&temp_var_2, 1, MPI_INT, rank+1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   if(temp_var != MPI_SUCCESS){
                return -1;
         }
   int temp_var = ((temp_var_2>locArr[loc_arr_len-1]) ? temp_var_2:locArr[loc_arr_len-1]);
   locArr[loc_arr_len-1] = ((temp_var_2<=locArr[loc_arr_len-1]) ? temp_var_2:locArr[loc_arr_len-1]);
   temp_var_2 = MPI_Send(&temp_var, 1, MPI_INT, rank+1, i, MPI_COMM_WORLD);
   if(temp_var_2 != MPI_SUCCESS){
                return -1;
      }
  }

  //receive data from previous process
  if(rank!=0 && (even!=(dis_loc[rank]%2))){
   temp_var = MPI_Recv(&locArr[0], 1, MPI_INT, rank-1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   if(temp_var != MPI_SUCCESS){
                return -1;
            }
  }
  even = (even+1)%2;
  MPI_Barrier(MPI_COMM_WORLD);
 }
 
 //Collect and combine sorted local arrays
 temp_var = MPI_Gatherv(locArr, loc_arr_len, MPI_INT, elements, send_count, dis_loc, MPI_INT, 0, MPI_COMM_WORLD);
 if(temp_var!=MPI_SUCCESS){
  return -1;
 }
    
    /* TODO END */

    if (rank == 0){ // record time (only executed in master process)
        t2 = std::chrono::high_resolution_clock::now();  
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::cout << "Student ID: " << "119010520" << std::endl; // replace it with your student id
        std::cout << "Name: " << "Bernaldy Jullian" << std::endl; // replace it with your name
        std::cout << "Assignment 1" << std::endl;
        std::cout << "Run Time: " << time_span.count() << " seconds" << std::endl;
        std::cout << "Input Size: " << num_elements << std::endl;
        std::cout << "Process Number: " << world_size << std::endl; 
    }

    if (rank == 0){ // write result to file (only executed in master process)
        std::ofstream output(argv[2]+std::string(".parallel.out"), std::ios_base::out);
        for (int i = 0; i < num_elements; i++) {
            output << sorted_elements[i] << std::endl;
        }
    }
    
    MPI_Finalize();
    
    return 0;
}