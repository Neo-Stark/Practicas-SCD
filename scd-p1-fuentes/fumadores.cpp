#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std;
using namespace SEM;

const unsigned num_fumadores = 3;
Semaphore mostr_vacio = 1;
Semaphore ingr_disp[num_fumadores] = {0, 0, 0};

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

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar(int num_fumador)
{

    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar(aleatorio<20, 200>());

    // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
         << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
    this_thread::sleep_for(duracion_fumar);

    // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
}

int producir()
{
    // calcular milisegundos aleatorios de duración de la acción de producir)
    chrono::milliseconds duracion_producir(aleatorio<20, 200>());

    // informa de que comienza a producir

    cout << "Estanquero: "
         << " empieza a producir (" << duracion_producir.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_producir' milisegundos
    this_thread::sleep_for(duracion_producir);
    int ingrediente = aleatorio<0, 2>();

    // informa de que ha terminado de producir

    cout << "Estanquero produce ingrediente: " << ingrediente << endl;
    return ingrediente;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador(int num_fumador)
{
    while (true)
    {
        sem_wait(ingr_disp[num_fumador]);
        cout << "Retirado ingrediente: " << num_fumador << endl;
        sem_signal(mostr_vacio);
        fumar(num_fumador);
    }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero()
{
    int i;
    while (true)
    {
        i = producir();
        sem_wait(mostr_vacio);
        cout << "Puesto ingrediente: " << i << endl;
        sem_signal(ingr_disp[i]);
    }
}
//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------" << endl
         << "Problema de los fumadores." << endl
         << "--------------------------------------------------------" << endl
         << flush;
    thread hebra_fumadores[num_fumadores];
    for (int i = 0; i < num_fumadores; i++)
        hebra_fumadores[i] = thread(funcion_hebra_fumador, i);

    thread hebra_estanquero(funcion_hebra_estanquero);

    for (int i = 0; i < num_fumadores; i++)
        hebra_fumadores[i].join();
    hebra_estanquero.join();
}