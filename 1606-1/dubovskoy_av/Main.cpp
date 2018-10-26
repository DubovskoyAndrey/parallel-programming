#include <math.h> 
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include "mpi.h"

using namespace std;

void CreateVec(float *vec, int N)
{
	cout << endl;
	cout << "Vector: ";
	srand(time(nullptr));
	for (int i = 0; i < N; i++)
	{
		vec[i] = rand() % 201 - 100;
		cout << vec[i] << " ";
	}
	cout << endl << endl;
}

int main(int argc, char* argv[])
{
	int N=8;
	
	float *vec = new float[N];

	float Sum, TotalSum = 0;
	int  ProcRank, ProcNum;
	Sum = 0;
	TotalSum = 0;
	double time1, time2, time3, time4;
	MPI_Status Status;

	// Инициализация
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	
	if (N < ProcNum)
	{
		cout << "N < ProcNum";
		return -1;
	}
	

	int n = N / (ProcNum-1);
	float *tmp = new float[n];

	if (ProcRank == 0)
	{
		cout << "ProcNum: " << ProcNum << endl;
		cout << "ProcRank: " << ProcRank << endl;
		CreateVec(vec, N);// Инициализвация вектора
		
		//LINER
		time3 = MPI_Wtime();
		for (int i = 0; i < N; i++)
		{
			TotalSum += vec[i];
		}
		time4 = MPI_Wtime();
		cout << "LinerRunTime: " << time4 - time3 << endl << "Result: " << TotalSum/N << endl<< endl;
		TotalSum = 0;


		//PARALLEL
		time1 = MPI_Wtime();

		for (int i = 0; i < ProcNum-1; i++)
		{
			tmp = vec + i * n;
			MPI_Send(tmp, n, MPI_FLOAT, i+1, 0, MPI_COMM_WORLD);
		}
		
		for (int i = (ProcNum - 1)*n; i < N; i++)
		{
			TotalSum += vec[i];
		}
		cout << "ProcRank: " << ProcRank <<" Sum: "<< TotalSum << endl;
		cout << endl;
	}
	else
	{		
		MPI_Recv(tmp, n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &Status);//прием значений 
		cout << "ProcRank: " << ProcRank << endl;
		for (int i = 0; i < n; i++)
		{
			cout << " " << tmp[i];
			Sum += tmp[i];
		}
		cout << " Sum: " << Sum << endl;
		MPI_Send(&Sum, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);// отправка Sum
		
	}
	if (ProcRank == 0)
	{
		//Прием значений Sum, их сумма и вычисление среднего
		for (int i = 1; i < ProcNum; i++) {
			MPI_Recv(&Sum, 1, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Status);
			TotalSum += Sum;
		}
		TotalSum /= N;
		time2 = MPI_Wtime();
		//Вывод
		cout << "Result: " << TotalSum << endl;
		cout << "Runtime: " << time2 - time1 << endl;
		system("pause");
	}


	MPI_Finalize();
	
	return 0;
}
