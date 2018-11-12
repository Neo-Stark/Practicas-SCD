#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

const unsigned
    num_fumadores = 3; //Número de fumadores en el estanco

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

class Estanco : public HoareMonitor
{
  private:
    int ingrediente;      //Representa el ingrediente necesario para cada fumador
    bool mostrador_libre; //Indica en cada momento si el mostrador está libre

    CondVar
        mostrador,              //Cola condición para el estanquero
        fumador[num_fumadores]; //Cola condición para cada fumador

  public:
    Estanco();
    void obtenerIngrediente(int i);
    void ponerIngrediente(int i);
    void esperarRecogidaIngrediente();
};

Estanco::Estanco()
{
    ingrediente = -1;
    mostrador_libre = true;
    for (unsigned i = 0; i < num_fumadores; i++)
        fumador[i] = newCondVar();
    mostrador = newCondVar();
}

void Estanco::obtenerIngrediente(int i)
{
    if (mostrador_libre)
        fumador[i].wait();

    if (ingrediente != i)
        fumador[i].wait();

    cout << "Retirado ingrediente: " << ingrediente << endl;
    mostrador_libre = true;
    mostrador.signal();
}

void Estanco::ponerIngrediente(int i)
{
    cout << "Puesto ingrediente: " << i << endl;
    ingrediente = i;
    mostrador_libre = false;
    fumador[i].signal();
}

void Estanco::esperarRecogidaIngrediente()
{
    if (!mostrador_libre)
    {
        cout << "Esperando recogida ingrediente " << ingrediente << endl;
        mostrador.wait();
    }
}
//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador(int num_fumador, MRef<Estanco> estanco)
{
    while (true)
    {
        estanco->obtenerIngrediente(num_fumador);
        fumar(num_fumador);
    }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(MRef<Estanco> estanco)
{
    int i;
    while (true)
    {
        i = producir();
        estanco->ponerIngrediente(i);
        estanco->esperarRecogidaIngrediente();
    }
}
//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------" << endl
         << "Problema de los fumadores." << endl
         << "--------------------------------------------------------" << endl
         << flush;

    auto monitor = Create<Estanco>();
    thread hebra_fumadores[num_fumadores];
    for (int i = 0; i < num_fumadores; i++)
        hebra_fumadores[i] = thread(funcion_hebra_fumador, i, monitor);

    thread hebra_estanquero(funcion_hebra_estanquero, monitor);

    for (int i = 0; i < num_fumadores; i++)
        hebra_fumadores[i].join();
    hebra_estanquero.join();
}