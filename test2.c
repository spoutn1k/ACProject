#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#pragma mpicoll check (main)

int main(int argc, char * argv[]) {
	MPI_Init(&argc, &argv);

	int a = 2;
	int b = 3;
	int c=0;

	if(c<10)
	{
		if(c <5)
		{
			a = a*a +1;
			MPI_Barrier(MPI_COMM_WORLD);
		}
		else
		{
			a = a*3;
			MPI_Barrier(MPI_COMM_WORLD);
		}

		c += (a * 2);
	}
	else
	{
		b = b*4;
	}

	for (a = 0; a < 10; a += 2) {
		b += 1;
		if (b+a == 4) {
			MPI_Barrier(MPI_COMM_WORLD);
		} else {
			MPI_Finalize();
		}
	}

	for (a = 0; a < 10; a += 2) {
		b++;
	}

	printf("c=%d\n", c);

	MPI_Finalize();
	return 1;
}
