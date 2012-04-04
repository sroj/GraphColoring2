#include <iostream>
#include <cstring>
#include <cstdlib>
#include "Graph.h"
#include <ctime>
#include <string>


using namespace std;

//Enumeracion que representa el tipo de algoritmo a utilizar

enum algoritmo_t {
    dsatur, brown, brelaz, brown_look_ahead
};

int main(int argc, char **argv) {
    //Lectura de parametros de la consola
    if (argc >= 2 && argc <= 5) {
        algoritmo_t algoritmo = dsatur;
        int tmax = 60 * 60;

        for (int i = 1; i < argc - 1; i++) {
            if (strcmp("-b", argv[i]) == 0) {
                algoritmo = brown;
            } else if (strcmp("-t", argv[i]) == 0) {
                int t = atoi(argv[i + 1]);
                if (t > 0) {
                    tmax = t;
                    i++;
                } else {
                    cout << "El valor de tiempo indicado es invalido\n";
                    cout << "Sintaxis correcta: gsc2 [opciones] <instancia>\n";
                    return EXIT_FAILURE;
                }
            } else if (strcmp("-z", argv[i]) == 0) {
                algoritmo = brelaz;
            } else if (strcmp("-k", argv[i]) == 0) {
                algoritmo = brown_look_ahead;
            } else if (strcmp("-d", argv[i]) != 0) {
                cout << "Sintaxis correcta: gsc2 [opciones] <instancia>\n";
                return EXIT_FAILURE;
            }
        }
        try {
            double executionTime;
            Graph grafo(argv[argc - 1]);
            if (algoritmo == dsatur) {
                executionTime = grafo.Dsatur(tmax);
            } else if (algoritmo == brown) {
                executionTime = grafo.Brown(tmax);
            } else if (algoritmo == brelaz) {
                Graph grafo2(argv[argc - 1]);
                executionTime = grafo.Brelaz(tmax, grafo2);
            } else {
                executionTime = grafo.BrownDynamicReordering(tmax);
            }
            if (executionTime != -1) {
                grafo.printOutput(cout, executionTime);
            } else {
                cout << "El tiempo de ejecucion permitido fue excedido\n";
                return EXIT_FAILURE;
            }
        } catch (string mensaje) {
            cout << mensaje;
            return EXIT_FAILURE;
        }
    } else {
        cout << "El numero de argumentos es incorrecto\n";
        cout << "Sintaxis correcta: gsc2 [opciones] <instancia>\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
