#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int
    num_alumnos = 9,
    num_procesos =num_alumnos + 2,     //Sumamos el proceso del camarero
    una_copa = 1,
    dos_copas = 2,
		preparar_copa = 3,
    id_bareto = 9,
		id_coctelero = 10;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template <int min, int max>
int aleatorio()
{
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

// ---------------------------------------------------------------------

void funcion_bareto()
{
	int num_copas = 7, emisor;
	MPI_Status estado;
	
	while(true)
	{
		if (num_copas > 2)
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
		else if(num_copas != 0)
			MPI_Probe(MPI_ANY_SOURCE, una_copa, MPI_COMM_WORLD, &estado);
		else
		{
			MPI_Ssend(NULL, 0, MPI_INT, id_coctelero, preparar_copa, MPI_COMM_WORLD);
			MPI_Probe(id_coctelero, preparar_copa, MPI_COMM_WORLD, &estado);
		}
		
		emisor = estado.MPI_SOURCE;
		
		switch(estado.MPI_TAG)
		{
		case una_copa:
			MPI_Recv(NULL, 0, MPI_INT, emisor, una_copa, MPI_COMM_WORLD, &estado);
			cout << "Bareto pone una copa al alumno " << emisor << endl;
			MPI_Ssend(NULL, 0, MPI_INT, emisor, una_copa, MPI_COMM_WORLD);
			num_copas--;
			break;
		case dos_copas:
			MPI_Recv(NULL, 0, MPI_INT, emisor, dos_copas, MPI_COMM_WORLD, &estado);
			cout << "Bareto pone dos copa al alumno " << emisor << endl;
			MPI_Ssend(NULL, 0, MPI_INT, emisor, dos_copas, MPI_COMM_WORLD);
			num_copas-= 2;
			break;
		case preparar_copa:
			MPI_Recv(&num_copas, 1, MPI_INT, id_coctelero, preparar_copa, MPI_COMM_WORLD, &estado);
			cout << "Bareto recibe copas del coctelero " << endl;
			break;
		}
		cout << "Copas restantes: " << num_copas << endl;
	}
}

void funcion_coctelero()
{
	const int num_copas = 6;
	MPI_Status estado;
	while(true)
	{
	MPI_Recv(NULL, 0, MPI_INT, id_bareto, preparar_copa, MPI_COMM_WORLD, &estado);
	cout << "Coctelero empieza a preparar copas" << endl;
	sleep_for(seconds(aleatorio<1, 2>()));	
	MPI_Ssend(&num_copas, 1, MPI_INT, id_bareto, preparar_copa, MPI_COMM_WORLD);
	}
}

void funcion_alumno(int id)
{
	MPI_Status estado;
	while(true)
	{
  	sleep_for(seconds(aleatorio<1, 2>()));
		int bebidas = aleatorio<1, 2>();
		cout << "Alumno " << id << " pide " << bebidas << " bebidas" << endl;
		MPI_Ssend(NULL, 0, MPI_INT, id_bareto, bebidas, MPI_COMM_WORLD);
		MPI_Recv(NULL, 0, MPI_INT, id_bareto, bebidas, MPI_COMM_WORLD, &estado);
		cout << "Alumno " << id << " recibe " << bebidas << " bebidas" << endl;
	}
}

int main(int argc, char **argv)
{
  int id_propio, num_procesos_actual;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

  if (num_procesos == num_procesos_actual)
  {
    // ejecutar la función correspondiente a 'id_propio'
    if (id_propio == id_bareto)
      funcion_bareto();
    else	if (id_propio == id_coctelero)
      funcion_coctelero(); 
		else 
			funcion_alumno(id_propio);
  }
  else
  {
    if (id_propio == 0) // solo el primero escribe error, indep. del rol
    {
      cout << "el número de procesos esperados es:    " << num_procesos << endl
           << "el número de procesos en ejecución es: " << num_procesos_actual << endl
           << "(programa abortado)" << endl;
    }
  }

  MPI_Finalize();
  return 0;
}
