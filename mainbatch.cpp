
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "Graph.h"
#include <ctime>
#include <string>

using namespace std;

enum algoritmo_t
{
    dsatur, brown, brelaz, brown_korman
};

int main(int argc, char **argv)
{
    if (argc >= 2 && argc <= 5)
    {
        algoritmo_t algoritmo = dsatur;
        int tmax = 60 * 60;

        for (int i = 1; i < argc - 1; i++)
        {
            if (strcmp("-b", argv[i]) == 0)
            {
                algoritmo = brown;
            }
	    else if (strcmp("-z", argv[i]) == 0)
            {
                algoritmo = brelaz;
            }
	    else if (strcmp("-k", argv[i]) == 0)
            {
                algoritmo = brown_korman;
            }
            else if (strcmp("-t", argv[i]) == 0)
            {
                int t = atoi(argv[i + 1]);
                if (t != 0)
                {
                    tmax = t;
                }
                else
                {
                    throw string("El valor indicado para el tiempo es incorrecto");
                }
            }
            else if (strcmp("-d", argv[i]) != 0)
            {
                throw string("Uno de los argumentos es invalido");
            }
        }

        try
        {

            int coloresMin = 0;

            double executionTime;
int sumColores=0;
double sumTiempo=0.0;
int cant=0;
int malos=0;
            for(int i=1; i<=90;i++)
            {
                char nombreArchivo[12];
                sprintf (nombreArchivo, "%d", i);
                Graph grafo(nombreArchivo);

            //Graph grafo(argv[argc - 1]);
            if (algoritmo == dsatur)
            {
                executionTime = grafo.Dsatur(tmax);
            }
            else
            {
                executionTime = grafo.Brown(tmax);

            }
            if(executionTime !=-1)
            {
                coloresMin = grafo.getNumberOfColors();
                grafo.printOutput(cout, executionTime);
            }
else{
malos++;
}

cant++;
		if(cant==3){
cant=0;

                ofstream archivoGenerado;
                strcat(nombreArchivo,".out");
                archivoGenerado.open (nombreArchivo);
                if (archivoGenerado.is_open())
                {
                    archivoGenerado << "colores: " << coloresMin << "\n";
                    archivoGenerado << "tiempo: " << executionTime<< "\n";
                    archivoGenerado.close();
                }
                else
                {
                    throw string("archivoooo2");
                }
malos=0;
}


            }

        }
        catch (string mensaje)
        {
            cout << mensaje;
            return EXIT_FAILURE;
        }
    }
    else
    {
        cout << ("El numero de argumentos es incorrecto");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
