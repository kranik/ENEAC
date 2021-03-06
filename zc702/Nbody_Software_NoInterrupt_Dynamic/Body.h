#ifndef _BODY_TASK_
#define _BODY_TASK_


#include "tbb/parallel_for.h"
#include "tbb/task.h"
#include "tbb/tick_count.h"
////#include <sys/ioctl.h>
////#include <sys/types.h>
////#include <sys/wait.h>

using namespace tbb;


/*****************************************************************************
 * Fucntions

 * **************************************************************************/


/*****************************************************************************
 * NbodyTask
 * **************************************************************************/
class NbodyTask{
public:
	bool firsttime;
public:
	void sendObjectToGPU( int begin, int end) {
		//ojo en lugar de copiar nbodies elementos se pueden copiar solo aquellos usados
		//if(firsttime){
			//firsttime=false;
			//toFPGA(bodies,nbodies);
			//error = clEnqueueWriteBuffer(command_queue, d_bodies, CL_NON_BLOCKING /*No blocking*/, 0,
			//		sizeof(OctTreeLeafNode) * nbodies, bodies, 0, NULL, NULL);
		//}

	}

	void OperatorGPU(int begin, int end) {
#ifdef XILINX
	  //printf("Step is %d\n",step);
	  ////int result=kernelNBody(firsttime, NUMBODIES, step, epssq, dthf,interrupt,status,file_desc,begin, end, (float*) bodies,(float*) bodies_vel,(float*) bodies_temp);
		kernelNBody(firsttime, NUMBODIES, step, epssq, dthf,begin, end, (float*) bodies,(float*) bodies_vel,(float*) bodies_temp);
	  if(firsttime) firsttime=false;
#else
	  for (int i = begin; i < end; i++) { // Run on CPU
	    ComputeForce( &bodies[i],&bodies_vel[i]);
	  }
#endif
	}

	void getBackObjectFromGPU(int begin, int end) {
		
	  //error = clEnqueueReadBuffer(command_queue, d_bodies, CL_NON_BLOCKING /*No blocking*/, begin * sizeof(OctTreeLeafNode),
	  //		sizeof(OctTreeLeafNode) * (end-begin), &bodies[begin], 0, NULL, NULL);
		
	}

	void OperatorCPU(int begin, int end) {
		//std::cout << "CPU begin: "<< begin<< " CPU end: "<< end<<std::endl;
		for (int i = begin; i < end; i++) {
		 ComputeForce( &bodies[i],&bodies_vel[i]);
		 }
	}

};

#if SCHED == SS

/*****************************************************************************
 * Parallel For
 * **************************************************************************/

class ParallelFor {
	NbodyTask *body;
public:
	ParallelFor(NbodyTask *b){
		body = b;
	}

    void operator()( const blocked_range<int>& range ) const {
        body->OperatorCPU(range.begin(), range.end());
    }
};

// Note: Reads input[0..n] and writes output[1..n-1]. 
void ParallelFORCPUs(size_t start, size_t end, NbodyTask *body ) {
    ParallelFor pf(body);
	parallel_for( blocked_range<int>( start, end ), pf);
}

/*****************************************************************************
 * Raw Tasks
 * **************************************************************************/


class GPUTask: public task {
public:
   NbodyTask *body;
   int begin;
   int end;
   GPUTask(NbodyTask *b, int beg, int e ) : body(b), begin(beg), end(e){}

    // Overrides virtual function task::execute
    task* execute() { 
        //std::cout << "GPU begin: "<< begin << " GPU end: "<< end <<std::endl;     
        tick_count start = tick_count::now();
		body->sendObjectToGPU(begin, end);
		body->OperatorGPU(begin, end);
		body->getBackObjectFromGPU(begin, end);
		tick_count finish = tick_count::now();
//		gpuThroughput = ((end-begin) / ((finish-start).seconds() * 1000));
        return NULL;
    }
};

class CPUTask: public task {
public:
   NbodyTask *body;
   int begin;
   int end;
   CPUTask(NbodyTask *b, int beg, int e ) : body(b), begin(beg), end(e){}

    // Overrides virtual function task::execute
    task* execute() {    
        //std::cout << "CPU begin: "<< begin<< " CPU end: "<< end << std::endl; 
		tick_count start = tick_count::now();
		ParallelFORCPUs(begin, end, body);
		tick_count finish = tick_count::now();
//		cpuThroughput = ((end-begin) / ((finish-start).seconds() * 1000));

        return NULL;
    }
};

class ROOTTask : public task{
public:
	NbodyTask *body;
	int begin;
	int end;
	
	ROOTTask(NbodyTask *b, int beg, int e) : body(b), begin(beg), end(e){}

	task* execute() {     
        int count = 1;
		if((chunkGPU > 0) && (chunkCPU > 0)){
			GPUTask& a = *new( allocate_child() ) GPUTask(body, begin, begin + chunkGPU);
			count++;
			CPUTask& b = *new( allocate_child() ) CPUTask(body, begin + chunkGPU, end);
			count++;
			// Set ref_count to 'two children plus one for the wait".
			set_ref_count(count);
			// Start b running.
			spawn( b );
			// Start a running and wait for all children (a and b).
			spawn_and_wait_for_all(a);
		}
		if((chunkCPU > 0) && (chunkGPU < 1)){
			CPUTask& b = *new( allocate_child() ) CPUTask(body, begin + chunkGPU, end);
			count++;
			// Set ref_count to 'two children plus one for the wait".
			set_ref_count(count);
			// Start b running and wait for all children (a and b).
			spawn_and_wait_for_all(b);
		}
		if((chunkGPU > 0) && (chunkCPU < 1)){
			GPUTask& a = *new( allocate_child() ) GPUTask(body, begin, begin + chunkGPU);
			count++;
			// Set ref_count to 'two children plus one for the wait".
			set_ref_count(count);
			// Start a running and wait for all children (a and b).
			spawn_and_wait_for_all(a);
		}
		if((chunkGPU < 1) && (chunkCPU < 1)){
			cerr << "Impossible case" << endl;
			exit(0);
		}
		
		return NULL;
    }
};
#endif

#endif
