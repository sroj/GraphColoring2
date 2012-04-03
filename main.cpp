//#include <iostream>
//#include <cstring>
//#include <cstdlib>
//#include "Graph.h"
//#include <ctime>
//#include <string>
//
//using namespace std;
//
////Enumeracion que representa el tipo de algoritmo a utilizar
//enum algoritmo_t {
//    dsatur, brown, brelaz, brown_look_ahead
//};
//
//int main(int argc, char **argv) {
//    //Lectura de parametros de la consola
//    if (argc >= 2 && argc <= 5) {
//        algoritmo_t algoritmo = dsatur;
//        int tmax = 60 * 60;
//
//        for (int i = 1; i < argc - 1; i++) {
//            if (strcmp("-b", argv[i]) == 0) {
//                algoritmo = brown;
//            } else if (strcmp("-t", argv[i]) == 0) {
//                int t = atoi(argv[i + 1]);
//                if (t > 0) {
//                    tmax = t;
//                    i++;
//                } else {
//                    cout << "El valor de tiempo indicado es invalido\n";
//                    cout << "Sintaxis correcta: gsc2 [opciones] <instancia>\n";
//                    return EXIT_FAILURE;
//                }
//            } else if(strcmp("-z", argv[i]) == 0){
//                algoritmo = brelaz;
//            } else if(strcmp("-k", argv[i]) == 0){
//                algoritmo = brown_look_ahead;
//            } 
//            else if (strcmp("-d", argv[i]) != 0) {
//                cout << "Sintaxis correcta: gsc2 [opciones] <instancia>\n";
//                return EXIT_FAILURE;
//            }
//        }
//
//        try {
//            double executionTime;
//            Graph grafo(argv[argc - 1]);
//            if (algoritmo == dsatur) {
//                executionTime = grafo.Dsatur(tmax);
//            } else if(algoritmo == brown){
//                executionTime = grafo.Brown(tmax);
//            } else if(algoritmo == brelaz){
//                Graph grafo2(argv[argc - 1]);
//                executionTime = grafo.Brelaz(tmax, grafo2);
//            } else{
//                executionTime = grafo.BrownLookAhead(tmax);
//            }
//
//            if (executionTime != -1) {
//                grafo.printOutput(cout, executionTime);
//                 cout << grafo.isFactible();
//                
//            } else {
//                cout << "El tiempo de ejecucion permitido fue excedido\n";
//                return EXIT_FAILURE;
//            }
//        } catch (string mensaje) {
//            cout << mensaje;
//            return EXIT_FAILURE;
//        }
//    } else {
//        cout << "El numero de argumentos es incorrecto\n";
//        cout << "Sintaxis correcta: gsc2 [opciones] <instancia>\n";
//        return EXIT_FAILURE;
//    }
//    return EXIT_SUCCESS;
//}
//
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "Graph.h"
#include <ctime>
#include <string>

using namespace std;

enum algoritmo_t {
    dsatur, brown, brelaz, brown_korman
};

int main(int argc, char **argv) {
    if (argc >= 2 && argc <= 5) {
        algoritmo_t algoritmo = dsatur;
        int tmax = 59 * 60;

        for (int i = 1; i < argc - 1; i++) {
            if (strcmp("-b", argv[i]) == 0) {
                algoritmo = brown;
            } else if (strcmp("-z", argv[i]) == 0) {
                algoritmo = brelaz;
            } else if (strcmp("-k", argv[i]) == 0) {
                algoritmo = brown_korman;
            } else if (strcmp("-t", argv[i]) == 0) {
                int t = atoi(argv[i + 1]);
                if (t != 0) {
                    tmax = t;
                } else {
                    throw string("El valor indicado para el tiempo es incorrecto");
                }
            } else if (strcmp("-d", argv[i]) != 0) {
                throw string("Uno de los argumentos es invalido");
            }
        }

        try {
            int coloresMin = 0;
            double executionTime = 0;
            int sumColores = 0;
            double sumTiempo = 0.0;
            int cant = 0;
            int malos = 0;
            for (int i = 1; i <= 90; i++) {
                executionTime = 0;
                coloresMin = 0;
                char nombreArchivo[12];
                sprintf(nombreArchivo, "%d", i);
                Graph grafo(nombreArchivo);


                if (algoritmo == dsatur) {
                    executionTime = grafo.Dsatur(tmax);
                } else if (algoritmo == brown) {
                    executionTime = grafo.Brown(tmax);
                } else if (algoritmo == brelaz) {
                    Graph grafo2(nombreArchivo);
                    executionTime = grafo.Brelaz(tmax, grafo2);
                } else if (algoritmo = brown_korman) {
                    executionTime = grafo.BrownLookAhead(tmax);
                }

                if (executionTime != -1) {
                    coloresMin = grafo.getNumberOfColors();
                    cout << grafo.isFactible() << endl;
                    sumColores += coloresMin;
                    sumTiempo += executionTime;
                    // grafo.printOutput(cout, executionTime);
                } else {
                    malos++;
                }
                
                cant++;
                if (cant == 3) {
                    ofstream archivoGenerado;
                    strcat(nombreArchivo, ".out");
                    archivoGenerado.open(nombreArchivo);
                    if (archivoGenerado.is_open()) {
                        if (malos != 3) {
                            archivoGenerado.precision(4);
                            archivoGenerado << "colores promedio: " << fixed << (double) sumColores / (double) (3.0 - malos) << endl;
                            archivoGenerado << "tiempo promedio: " << fixed << (double) sumTiempo / (double) (3.0 - malos) << endl;
                        }
                        archivoGenerado << "instancias que se pasaron del tiempo: " << malos << endl;
                        archivoGenerado.close();
                    } else {
                        throw string("archivoooo2");
                    }
                    cant = 0;
                    malos = 0;
                    sumColores = 0;
                    sumTiempo = 0.0;
                }
            }

        } catch (string mensaje) {
            cout << mensaje;
            return EXIT_FAILURE;
        }
    } else {
        cout << ("El numero de argumentos es incorrecto");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
