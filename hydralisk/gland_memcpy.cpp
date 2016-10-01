#include <iostream>
#include <numa.h>
#include <chrono>
#include <string>
#include <unistd.h>

static long N=1024 * 1024 / sizeof(double) * 128; // 128 MB
bool humanreadable = false;

void memcpy_task(double *a, double *b)
{
	typedef std::chrono::high_resolution_clock Time;
   	typedef std::chrono::milliseconds ms;
   	typedef std::chrono::duration<float> fsecDuration;
    	auto lastTime = Time::now();
	
	memcpy(a, b, sizeof(double) * N);
	
	fsecDuration d = Time::now()-lastTime;
	float throughput = 0.128/(d.count());
	if(!humanreadable)
		write(STDOUT_FILENO, &throughput, sizeof(throughput));
	else
		std::cout << throughput << std::endl;
}
 
 
int main(int argc, char** argv)
{
	if(argc < 3)
	{	
		std::cout << "Usage:" << std::endl
			  << "gland <localNodeId> <remoteNodeId> [--human]" <<std::endl;
		exit(EXIT_FAILURE);
	}
	if(argc == 4 && strncmp(argv[3],"--human",7) == 0)
	{
		std::cout << "enabling human readable output" << std::endl;
		humanreadable = true;
	}

	size_t localNode = std::stoi(argv[1]);
	size_t remoteNode = std::stoi(argv[2]);
	
	numa_run_on_node(localNode);
	
	double *a = (double*) numa_alloc_onnode( N * sizeof(double), localNode );
	double *b = (double*) numa_alloc_onnode( N * sizeof(double), remoteNode );

	for(int i = 0; i<N ; i++)
	{
		a[i]=(double)i;
		b[i]=(double)-i;
	}

	while(1) memcpy_task(a, b);
}
