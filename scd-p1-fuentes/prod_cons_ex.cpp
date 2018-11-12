#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std;
using namespace SEM;

//**********************************************************************
// variables compartidas

const int num_items = 40,            // número de items
    tam_vec_1 = 10,
    tam_vec_2 = 15;                    // tamaño del buffer
    
unsigned cont_prod[num_items] = {0}, // contadores de verificación: producidos
    cont_cons[num_items] = {0};      // contadores de verificación: consumidos
Semaphore libres1 = tam_vec_1;          // Semáforo que controla las celdas libres del vector1
Semaphore libres2 = tam_vec_2;          // Semáforo que controla las celdas libres del vector2
Semaphore ocupadas1 = 0;              // Semáforo que controla las celdas ocupadas del vector1
Semaphore ocupadas2 = 0;				// Semáforo que controla las celdas ocupadas del vector2
int vector1[tam_vec_1];                    //Buffers intermedio
int vector2[tam_vec_2];
int prod_cons = 0;

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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
    static int contador = 0;
    this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));

    cout << "producido: " << contador << endl
         << flush;

    cont_prod[contador]++;
    return contador++;
}
//----------------------------------------------------------------------

void consumir_dato(unsigned dato)
{
    cont_cons[dato]++;
    this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));

    cout << "                  consumido: " << dato << endl;
}

//----------------------------------------------------------------------

void test_contadores()
{
    bool ok = true;
    cout << "comprobando contadores ....";
    for (unsigned i = 0; i < num_items; i++)
    {
        if (cont_prod[i] != 1)
        {
            cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl;
            ok = false;
        }
        if (cont_cons[i] != 1)
        {
            cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl;
            ok = false;
        }
    }
    if (ok)
        cout << endl
             << flush << "solución (aparentemente) correcta." << endl
             << flush;
}

//----------------------------------------------------------------------

void funcion_hebra_productora()
{
    int primera_libre = 0;
    for (unsigned i = 0; i < num_items; i++)
    {
        int dato = producir_dato();
        if (dato%2 == 0)
        	prod_cons++;
        else
        	prod_cons += 2;
        	
        sem_wait(libres1);                   
        vector1[primera_libre] = dato;          //Se introduce el dato en la primera celda libre
        if (primera_libre == tam_vec_1 - 1)   //Se comprueba si el contador ha llegado al tope del vector
            primera_libre = 0;              //Si es así se reinicia a 0
        else
            primera_libre++;
        sem_signal(ocupadas1);
    }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora()
{
    int primera_ocupada = 0;
    for (unsigned i = 0; i < num_items; i++)
    {
        int dato;
        sem_wait(ocupadas2);
        dato = vector2[primera_ocupada];            //Se lee el dato de la primera celda ocupada
        if (dato%2 == 0)
        	prod_cons++;
        else
        	prod_cons--;
        	
        if (primera_ocupada == tam_vec_2 - 1)     //Se comprueba si hemos llegado al final del vector
            primera_ocupada = 0;
        else
            primera_ocupada++;
        sem_signal(libres2);
        consumir_dato(dato);
    }
}
//----------------------------------------------------------------------

void funcion_hebra_intermedia()
{
	int primera_ocupada1 = 0,
		primera_libre2 = 0;
	for (unsigned i = 0; i < num_items; i++)
    {
    	sem_wait(ocupadas1);
		if(vector1[primera_ocupada1]%3 == 0)
			vector1[primera_ocupada1]*=10;
		vector2[primera_libre2] = vector1[primera_ocupada1];
		if (primera_ocupada1 == tam_vec_1 - 1)   //Se comprueba si el contador ha llegado al tope del vector
            primera_ocupada1 = 0;              //Si es así se reinicia a 0
        else
            primera_ocupada1++;
		sem_wait(libres2);
		cout << "Valor prod_cons: " << prod_cons << endl;
		if (primera_libre2 == tam_vec_2 - 1)   //Se comprueba si el contador ha llegado al tope del vector
            primera_libre2 = 0;              //Si es así se reinicia a 0
        else
            primera_libre2++;
		sem_signal(libres1);
		sem_signal(ocupadas2);
	}	
}

int main()
{
    cout << "--------------------------------------------------------" << endl
         << "Problema de los productores-consumidores (solución FIFO)." << endl
         << "--------------------------------------------------------" << endl
         << flush;

    thread hebra_productora(funcion_hebra_productora),
        hebra_consumidora(funcion_hebra_consumidora),
        hebra_intermedia(funcion_hebra_intermedia);

    hebra_productora.join();
    hebra_consumidora.join();
    hebra_intermedia.join();

    test_contadores();
}
