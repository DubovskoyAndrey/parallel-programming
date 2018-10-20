#include <math.h> 
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include "mpi.h"

using namespace std;

void CreateVec(float *vec, int N)
{
	cout << "Vector: ";
	srand(time(nullptr));
	for (int i = 0; i < N; i++)
	{
		vec[i] = rand() % 201 - 100;
		cout << vec[i] << " ";
	}
	cout << endl;
}

int main(int argc, char* argv[])
{
	int N;
	cout << "Enter the number of elements" << endl;
	cin >> N;

	float *vec = new float[N];

	float Sum, TotalSum = 0;
	int  ProcRank, ProcNum;
	Sum = 0;
	TotalSum = 0;

	MPI_Status Status;
	// �������������
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	cout << "ProcNum: " << ProcNum << endl;
	cout << "ProcRank: " << ProcRank << endl;
	
	float time1, time2;

	int n = N / ProcNum;
	float *tmp = new float[n];

	if (ProcRank==0)
	{
		cout << "ProcRank: " << ProcRank << endl;
		CreateVec(vec, N);// �������������� �������

		time1 = MPI_Wtime();
		
		for (int i = 0; i < ProcNum - 1; i++)
		{
			tmp = vec + i * n;
			MPI_Send(tmp, n, MPI_FLOAT, i + 1, 0, MPI_COMM_WORLD);
		}
		
		for (int i = (ProcNum - 1)*n; i < N; i++)
		{
			TotalSum += vec[i];
		}
		
		//����� �������� Sum, �� ����� � ���������� ��������
		for (int i = 1; i < ProcNum; i++) {
			MPI_Recv(&Sum, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Status);
			TotalSum += Sum;
		}
		
				
	}
	else
	{
		cout << "ProcRank: " << ProcRank << endl;

		MPI_Recv(tmp, n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &Status);//����� �������� 
		for (int i = 0; i < n; i++)
			Sum += tmp[i];
		MPI_Send(&Sum, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);// �������� Sum
		cout << Sum << endl;
	}
	if (ProcRank == 0)
	{
		TotalSum = TotalSum / N;
		time2 = MPI_Wtime();
		//�����
		cout << "Total sum: " << TotalSum << endl;
		cout << "Runtime: " << time2 - time1 << endl;
	}
	
	
	MPI_Finalize();
	cin.ignore();
	cin.ignore();
	return 0;
}
