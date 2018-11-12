#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

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

void Lavar(int cliente, int tipo)
{
    chrono::milliseconds duracion_lavar(aleatorio<20, 200>());
    
    cout << "Cliente " << cliente << "(tipo " << tipo << ") : "
         << "empieza a lavar la ropa(" << duracion_lavar.count() << " milisegundos)" << endl;
    this_thread::sleep_for(duracion_lavar);
}

void EsperarFueraLavanderia(int cliente, int tipo)
{
    chrono::milliseconds duracion_espera(aleatorio<20, 200>());

    cout << "Cliente " << cliente << "(tipo " << tipo << ") : "
         << " espera fuera (" << duracion_espera.count() << " milisegundos)" << endl;

    this_thread::sleep_for(duracion_espera);

    cout << "Cliente " << cliente << "(tipo " << tipo << ") : " 
         << "termina de estar fuera, entra en la Lavandería." << endl;
}


class Lavanderia : public HoareMonitor
{
    private:
        static const unsigned tipos_colada = 2;
        int tipo_ropa[tipos_colada];
        int max_lavadoras[tipos_colada];
        
        CondVar cola[tipos_colada];
    public:
        Lavanderia();
        void llegada(int tipo_colada);
        void fin_colada(int tipo_colada);
};

Lavanderia::Lavanderia()
{
    for (unsigned i = 0; i < tipos_colada; i++){
        tipo_ropa[i] = 0;
        cola[i] = newCondVar();
    }
    max_lavadoras[0] = 3;
    max_lavadoras[1] = 2;
}

void Lavanderia::llegada(int tipo_colada)
{
    if (tipo_ropa[tipo_colada-1] == max_lavadoras[tipo_colada-1]){
        cout << "Lavadoras tipo colada " << tipo_colada << " llenas" << endl;
        cola[tipo_colada-1].wait();
    }
    else                            //Pq c*** puse este else
        tipo_ropa[tipo_colada-1]++;
}

void Lavanderia::fin_colada(int tipo_colada)
{
    cout << "Termina colada tipo " << tipo_colada << " queda una lavadora libre" << endl;
    tipo_ropa[tipo_colada-1]--;
    cola[tipo_colada-1].signal();
}

void Hebra_Cliente_colada_tipo_1( MRef<Lavanderia> lavanderia, int cliente)
{
    while(true)
    {
        lavanderia->llegada(1);
        Lavar(cliente, 1);
        lavanderia->fin_colada(1);
        EsperarFueraLavanderia(cliente, 1);
    }
}

void Hebra_Cliente_colada_tipo_2(MRef<Lavanderia> lavanderia, int cliente)
{
    while(true)
    {
        lavanderia->llegada(2);
        Lavar(cliente, 2);
        lavanderia->fin_colada(2);
        EsperarFueraLavanderia(cliente, 2);
    }
}

int main()
{
    cout << "--------------------------------------------------------" << endl
         << "Problema de la lavanderia." << endl
         << "--------------------------------------------------------" << endl
         << flush;
    const unsigned
        num_clientes_tipo_1 = 5,
        num_clientes_tipo_2 = 4;
    auto monitor = Create<Lavanderia>();

    thread hebra_cliente_colada_tipo_1[num_clientes_tipo_1];
    for (int i = 0; i < num_clientes_tipo_1; i++)
        hebra_cliente_colada_tipo_1[i] = thread(Hebra_Cliente_colada_tipo_1, monitor, i);

    thread hebra_cliente_colada_tipo_2[num_clientes_tipo_2];
    for (int i = 0; i < num_clientes_tipo_2; i++)
        hebra_cliente_colada_tipo_2[i] = thread(Hebra_Cliente_colada_tipo_2, monitor, i);

    for (int i = 0; i < num_clientes_tipo_1; i++)
        hebra_cliente_colada_tipo_1[i].join();

    for (int i = 0; i < num_clientes_tipo_2; i++)
        hebra_cliente_colada_tipo_2[i].join();
}


