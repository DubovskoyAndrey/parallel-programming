#include <iostream>
#include <time.h>
#include <mpi.h>


using namespace std;

int* CreateMatrix(int n, int m)
{
	int* matrix = new int[n * m];
	for (int i = 0; i < n * m; i++)
		matrix[i] = rand() % 10;
	return matrix;
}


void OutputMatrix(int n, int m, int* matrixToOutput)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
			cout << matrixToOutput[i * m + j] << " ";
		cout << endl;
	}
}

int main(int argc, char **argv)
{
	int procNum = 0, procRank = 0;
	double startTime, endTime;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	int n = atoi(argv[1]);//строки в первой
	int m = atoi(argv[2]);//столбцы в первой, строки во второй
	int s = atoi(argv[3]);//столбцы во второй

	//массивы для хранения матриц 
	int* m1 = new int[n*m];
	int* m2 = new int[m*s];
	int* m3 = new int[n*s];
	int* m4 = new int[n*s];

	//Линейный
	if (procRank == 0)
	{
		cout <<"ProcNum: "<< procNum <<" ProcRank: "<< procRank << endl;
		srand(time(0));

		m1 = CreateMatrix(n, m);
		m2 = CreateMatrix(m, s);
		m3 = CreateMatrix(n, s);
		m4 = CreateMatrix(n, s);

		cout << " Matrix 1:"<< endl;
		OutputMatrix(n, m, m1);
		cout << endl;
		cout << " Matrix 2:"<< endl;
		OutputMatrix(m, s, m2);
		cout << endl;

		startTime = MPI_Wtime();


		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < s; j++)
			{
				int tmp = 0;
				for (int k = 0; k < m; k++)
				{
					tmp += m1[i * m + k] * m2[k * s + j];
				}
				m3[i * s + j] = tmp;
			}
		}
		endTime = MPI_Wtime();

		cout << "Linear result matrix: "<< endl;
		OutputMatrix(n, s, m3);
		cout << endl << "Linear time:" << endTime - startTime << endl;
	}

	//Параллельный
	//Cинхронизация процессов
	MPI_Barrier(MPI_COMM_WORLD);
	startTime = MPI_Wtime();

	int* send_quantity = new int[procNum]; //массив кол-в эл-тов, посылаемых i-му процесору
	int* send_displs = new int[procNum]; //массив смещений относительно начала посылаемого буфера для данных, посылаемых процессу i;

	int* rec_quantity = new int[procNum]; //массив кол-ва эл-тов, получаемых от i-ого процессора
	int* rec_displs = new int[procNum]; //массив смещений относительно начала посылаемого буфера для данных, получаемых от i;

	int rowCount; //число строк, переданных i-ому процессору
	int* matrix1par, *matrix2par;

	if (procRank == 0)
	{
		if (procNum > n)
		{
			cout << "procNum > n";
			return -1;
		}
		int ch = n / procNum;
		int r = n % procNum;
		cout << "ch: " << ch << " r: " << r;
		if (procRank < r) rowCount = ch + r;
		else rowCount = ch;

		cout << endl;
		for (int i = 0; i < procNum; i++)
		{			
			send_quantity[i] = ch * m;
			send_displs[i] = i * ch * m;
			rec_quantity[i] = ch * s;
			rec_displs[i] = i * ch * s;			
		}
		r = n - (procNum - 1) * ch;
		send_quantity[procNum - 1] = r * m;		
		rec_quantity[procNum - 1] = r * s;
		for (int i = 0; i < procNum; i++)
		{
			cout << "procRank: " << i <<" "<< send_quantity[i] << " " << send_displs[i] << endl;
		}
	}
	//Рассылка данных (неделимых) процессорам
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Bcast(m2, m * s, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Bcast(send_quantity, procNum, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(send_displs, procNum, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(rec_quantity, procNum, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(rec_displs, procNum, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&rowCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Контейнеры для передачи и получения
	matrix1par = new int[send_quantity[procRank]];
	matrix2par = new int[rec_quantity[procRank]];

	// Рассылка частей процессорам		
	MPI_Scatterv(m1, send_quantity, send_displs, MPI_INT, matrix1par, send_quantity[procRank], MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < rowCount; i++)
	{
		for (int k = 0; k < s; k++)
		{
			matrix2par[i * s + k] = 0;
			for (int j = 0; j < m; j++)
			{				
				matrix2par[i * s + k] += matrix1par[i * m + j] * m2[j * s + k];
			}
		}
	}
	
	// Сбор блоков в главный процессор	
	MPI_Gatherv(matrix2par, rec_quantity[procRank], MPI_INT, m4, rec_quantity, rec_displs, MPI_INT, 0, MPI_COMM_WORLD);
	endTime = MPI_Wtime();

	if (procRank == 0)
	{
		endTime = MPI_Wtime();
		cout << endl << "MPI result martrix:"<< endl;
		OutputMatrix(n, s, m4);
		cout << endl << "MPI time: " << endTime - startTime << endl;
	}
	if (procRank == 0)
	{
		for (int i = 0; i < n*s; i++)
		{
			if (m3[i] != m4[i])
			{
				cout << "Matrixs are not equal" << endl;
				return -1;
			}
		}
		cout << "Matrixs are equal" << endl;		
	}

	delete[] send_displs;
	delete[] send_quantity;
	delete[] rec_displs;
	delete[] rec_quantity;

	delete[] m1;
	delete[] m2;
	delete[] m3;	
	delete[] m4;
	
	MPI_Finalize();
}
