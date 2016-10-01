#include <numa.h>
#include <cstdlib>

int main(int argc, char** argv)
{
	numa_run_on_node(atoi(argv[1]));
	while(1);;
}
