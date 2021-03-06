/*
 * File:   Graph.h
 * Author: Simon Rojas & Marlin Aranguren
 *
 * Created on March 17, 2012, 9:30 PM
 */

#ifndef GRAPH_H
#define	GRAPH_H

#include <list>
#include <queue>
#include <vector>
#include <iostream>
#include <ostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <algorithm>
#include "GraphNode.h"
#include "CompareDegreeFunctor.h"
#include "CompareLabelFunctor.h"

using namespace std;

//Clase Graph
//Representa un grafo, implementado mediante un arreglo de adyacencias.
//Incluye metodos que implementan los algoritmos de coloracion de Brown
//y de Brelaz.

class Graph {
public:

    //Constructor
    //Recibe la ruta de un archivo en formato DIMACS Standard e inicializa
    //el grafo a partir de la información contenida en el mismo.

    Graph(char path[]) {
        int u;
        int v;
        numColored = 0;
        string linea;
        ifstream instancia(path);
        char* token;
        bool primeraLineaLeida = false;

        if (!instancia.is_open())
            throw string("No se pudo abrir el archivo");

        while (instancia.good()) {
            getline(instancia, linea);
            char copia [linea.size() + 1];
            strcpy(copia, linea.c_str());

            token = strtok(copia, " ");
            while (token != NULL) {
                if (strcmp(token, "e") == 0) {
                    if (!primeraLineaLeida)
                        throw string("Formato de archivo invalido");
                    token = strtok(NULL, " ");
                    u = atoi(token);
                    token = strtok(NULL, " ");
                    v = atoi(token);
                    addArc(u, v);
                    break;
                } else if (strcmp(token, "c") == 0) {
                    break;
                } else if (strcmp(token, "p") == 0) {
                    token = strtok(NULL, " ");
                    token = strtok(NULL, " ");
                    numNodes = atoi(token);
                    token = strtok(NULL, " ");
                    numEdges = atoi(token);
                    nodesArray = new GraphNode* [numNodes];
                    colorationOrder = new GraphNode* [numNodes];
                    adjacencyArray = new vector<GraphNode*>* [numNodes];
                    nodesDegreeSortedArray = new GraphNode* [numNodes];
                    finalColorationBrown = new GraphNode* [numNodes];
                    allowedColors = new list<int>* [numNodes];
                    greedyOrdering = new GraphNode* [numNodes];
                    initializeNodesArrays();
                    initializeAdjancencyArray();
                    initializeAllowedColors();
                    primeraLineaLeida = true;
                    break;
                }
                token = strtok(NULL, " ");
            }
        }
        instancia.close();
        uncoloredNodes = new list<GraphNode*> (nodesArray, nodesArray + numNodes);
        sortNodesByDegree();
        initializeBrownGreedyOrderArray();
    }

    //Destructor

    virtual ~Graph() {
        for (int i = 0; i < numNodes; i++) {
            delete(adjacencyArray[i]);
            delete(nodesArray[i]);
            delete(finalColorationBrown[i]);
        }
        delete uncoloredNodes;
        delete[] nodesDegreeSortedArray;
        delete[] adjacencyArray;
        delete[] nodesArray;
        delete[] finalColorationBrown;
        delete[] colorationOrder;
        delete[] allowedColors;
    }

    //Copy-constructor

    Graph(const Graph& other) {
        numNodes = other.numNodes;
        numEdges = other.numEdges;
        numColored = other.numColored;
        nodesArray = new GraphNode* [other.numNodes];
        nodesDegreeSortedArray = new GraphNode* [other.numNodes];
        adjacencyArray = new vector<GraphNode*>* [other.numNodes];

        for (int i = 0; i < numNodes; i++) {
            nodesArray[i] = new GraphNode(*(other.nodesArray[i]));
        }

        for (int i = 0; i < numNodes; i++) {
            nodesDegreeSortedArray[i] =
                    nodesArray[other.nodesDegreeSortedArray[i]->GetLabel() - 1];
        }

        for (int i = 0; i < numNodes; i++) {
            unsigned int vectorSize = other.adjacencyArray[i]->size();
            vector<GraphNode*>* adjacentNodes = other.adjacencyArray[i];

            for (unsigned int j = 0; j < vectorSize; j++) {
                adjacencyArray[i]->push_back(nodesArray[(adjacentNodes->at(j))->GetLabel() - 1]);
            }
        }
    }

    //Recibe dos etiquetas de nodos y almacena en la lista de adyacencia del
    //nodo "u", el apuntador al nodo "v" y en la lista de adyacencia del nodo
    //"v" almacena el apuntador al nodo con etiqueta "u". Además incrementa el
    //grado de ambos nodos.

    void addArc(int u, int v) {
        (adjacencyArray[u - 1])->push_back(nodesArray[v - 1]);
        (adjacencyArray[v - 1])->push_back(nodesArray[u - 1]);

        //Incrementar el grado de cada nodo
        ++(*(nodesArray[u - 1]));
        ++(*(nodesArray[v - 1]));
    }

    //Recibe la etiqueta de un nodo y retorna un vector con apuntadores a los
    //nodos adyacentes a ese nodo.

    const vector<GraphNode*>* neighbors(int node_label) {
        if (node_label <= 0 || node_label > numNodes) {
            throw string("Etiqueta de nodo invalida en Graph::Neighbors");
        }
        return adjacencyArray[node_label - 1];
    }

    //Recibe la etiqueta de un nodo y el color que se desea utilizar para
    //colorearlo. Este método invoca el método setColor(Color) de la clase
    //GraphNode.

    void setColor(int node_label, int color) {
        if (node_label <= 0 || node_label > numNodes) {
            throw string("Etiqueta de nodo invalida en Graph::SetColor");
        }
        nodesArray[node_label - 1]->SetColor(color);
    }

    //Este método se utiliza únicamente para colorear nodos en el algoritmo
    //Dsatur. Recibe la etiqueta de un nodo y el color con el que se desea
    //colorear el nodo. Elimina de la lista de nodos no coloreados
    //(uncoloredNodes) el apuntador al nodo que se coloreó. Luego revisa
    //grado de saturación de los nodos adyacentes a este y lo incrementa
    //si corresponde.

    void setColorDsatur(int node_label, int color) {
        if (node_label > 0 && node_label <= numNodes) {
            if (nodesArray[node_label - 1]->GetColor() == 0) {
                numColored++;
                nodesArray[node_label - 1]->SetColor(color);
                colorationOrder[numColored - 1] = nodesArray[node_label - 1];
            }
            bool encontrado = false;

            for (list<GraphNode*>::iterator it = uncoloredNodes->begin();
                    it != uncoloredNodes->end() && !encontrado; it++) {
                if ((*it)->GetLabel() == node_label) {
                    uncoloredNodes->erase(it);
                    encontrado = true;
                }
            }
        } else {
            throw string("Etiqueta de nodo invalida en Graph::SetColorDsatur");
        }

        const vector<GraphNode*> *adjacents = neighbors(node_label);

        for (unsigned int i = 0; i < adjacents->size(); i++) {
            const vector<GraphNode*> *adjacentNeighbors =
                    neighbors((*adjacents)[i]->GetLabel());
            bool found = false;
            for (unsigned int j = 0; j < adjacentNeighbors->size(); j++) {
                if ((*adjacentNeighbors)[j]->GetLabel() != node_label &&
                        (*adjacentNeighbors)[j]->GetColor() == getColor(node_label)
                        && !found) {
                    found = true;
                }
            }
            if (!found) {
                nodesArray[(*adjacents)[i]->GetLabel() - 1]->
                        IncrementSaturationDegree();
            }
        }
    }

    //Este método recibe una etiqueta y devuelve el color en el que está
    //coloreado el nodo al que corresponde la etiqueda. Hace uso del método
    //getColor() de la clase GraphNode para colorear el nodo.

    int getColor(int node_label) {
        if (node_label <= 0 || node_label > numNodes) {
            throw string("Etiqueta de nodo invalida en Graph::getColor");
        }
        return nodesArray[node_label - 1]->GetColor();
    }

    //Este método recibe un objeto de tipo ostream (llamado output) y el tiempo
    //de duración de la ejecución del algoritmo. Luego imprime donde indique la
    //variable output los resultados de la ejecución del programa con una
    //instancia en particular.

    void printOutput(ostream& output, double time) {
        output.precision(8);
        output << "-----------------------------------" << endl;
        output << "TIEMPO DE EJECUCION: " << fixed << time << endl;
        output << "NUMERO DE COLORES ENCONTRADOS: " << getNumberOfColors()
                << endl;
        output << "COLORACION ENCONTRADA:" << endl;
        output << "Vertice\tColor" << endl;

        for (int i = 0; i < numNodes; i++) {
            output << i + 1 << "\t" << nodesArray[i]->GetColor() << endl;
        }
        output << "-----------------------------------" << endl;
    }

    //Este método retorna un apuntador a un vector que contiene apuntadores a
    //el/los nodo(s) no coloreados con grado de saturación máxima. Para ello
    //revisa en la lista unconloredNodes cuál es el máximo grado de saturación
    //y luego guarda en el vector los nodos que tengan ese grado máximo de
    //saturación.

    const vector<GraphNode*>* MaximalSaturationDegree() {
        vector<GraphNode*>* maximalSaturation = new vector<GraphNode*>;
        int max = 0;

        for (list<GraphNode*>::iterator it = uncoloredNodes->begin();
                it != uncoloredNodes->end(); it++) {
            if ((*it)->GetSaturationDegree() > max) {
                max = (*it)->GetSaturationDegree();
            }
        }

        for (list<GraphNode*>::iterator it = uncoloredNodes->begin();
                it != uncoloredNodes->end(); it++) {
            if ((*it)->GetSaturationDegree() == max) {
                maximalSaturation->push_back(*it);
            }
        }
        return maximalSaturation;
    }

    //Esta función revisa en el grafo cuál es el color más alto en el que estan
    //coloreados los nodos y retorna ese número.

    int getNumberOfColors() {
        int max = 0;
        for (int i = 0; i < numNodes; i++) {
            if (nodesArray[i]->GetColor() > max) {
                max = nodesArray[i]->GetColor();
            }
        }
        return max;
    }

    //Ejecuta el algoritmo de coloracion de Brown sobre el grafo.
    //Recibe un parametro que indica el tiempo maximo, en segundos, durante el
    //el cual se permitira la ejecución del método. Retorna el tiempo de
    //ejecución del algoritmo, o -1 en caso de que se agote el tiempo máximo.

    double Brown(int tmax) {
        int bestPartialNumColors[numNodes];
        clock_t startTime = clock();
        int nodeLabel = numNodes;
        int bestNumColors = initialColoration(bestPartialNumColors);

        bool backtracking = false;

        while (nodeLabel >= 1) {
            //Terminar la ejecucion del metodo si se excede del tiempo máximo
            if (((clock() - startTime) / (double) CLOCKS_PER_SEC) >= (double) tmax)
                return -1;

            if (!backtracking)
                nodeLabel = findByColor(bestNumColors) - 1;

            if (!tryNewColor(nodeLabel, bestNumColors)) {
                nodeLabel--;
                backtracking = true;
                continue;
            }

            resetColoration(nodeLabel + 1);
            nodeLabel = colorForward(nodeLabel + 1, &bestNumColors,
                    bestPartialNumColors);

            if (nodeLabel == numNodes) {
                backtracking = false;
            } else {
                backtracking = true;
            }
        }

        copyFinalToNodesArray();
        return (clock() - startTime) / (double) CLOCKS_PER_SEC;
    }

    // Esta función obtiene una clique a partir de un order de coloración
    // establecido por el algoritmo Dsatur. Esta es una clique maximal aproximada
    // los nodos de esta clique son almacenados en una lista y luego retornados.

    list<GraphNode*> GetClique(Graph& grafo2) {
        list<GraphNode*> clique;
        for (int i = 0; i < numNodes; i++) {
            clique.push_back(nodesArray[grafo2.colorationOrder[i]->GetLabel() - 1]);
            nodesArray[grafo2.colorationOrder[i]->GetLabel() - 1]->SetInClique(true);
            if (grafo2.colorationOrder[i + 1]->GetColor() <= grafo2.colorationOrder[i]->GetColor()) {
                break;
            }
        }
        return clique;
    }

    // Esta función recibe dos nodos y revisa si son adyacentes, de ser así 
    // retorna verdadero de lo contrario retorna falso.

    bool isAdyacent(GraphNode* node1, GraphNode* node2) {
        const vector<GraphNode*>* adjacents = neighbors(node1->GetLabel());
        int nodeLabel = node2->GetLabel();
        for (int i = 0; i < adjacents->size(); i++) {
            if ((*adjacents)[i]->GetLabel() == nodeLabel) {
                return true;
            }
        }
        return false;
    }

    // Este procedimiento se utiliza para el algoritmo de Brélaz, recibe el nodo
    // xk a partir del cual se quiere realizar el procediemiento, su posición en 
    // el orden de coloración y el número de colores de la solución parcial 
    // hasta el nodo anterior a xk en el orden de coloración. El método etiqueta
    // los nodos adyacentes a xk, que tengan menor rank  en el orden y que no 
    // estén etiquetados. Si hay varios que cumplen las características y que 
    // tienen el mismo color, etiqueta el de menor rank. 

    void Label(GraphNode* node_xk, int position, int numColors) {
        bool usedColors[numColors];
        for (int i = 0; i < numColors; i++) {
            usedColors[i] = false;
        }

        for (int i = 0; i < position; i++) {
            if (colorationOrder[i]->GetLabelBrelaz() == 0 && isAdyacent(node_xk, colorationOrder[i])) {
                if (usedColors[colorationOrder[i]->GetColor() - 1] == false) {
                    colorationOrder[i]->SetLabelBrelaz(position + 1);
                    usedColors[colorationOrder[i]->GetColor() - 1] = true;
                }
            }
        }
    }

    //Retorna la posición teórica del nodo etiquetado con mayor rank en el orden
    // de coloración.

    int maximalRankLabeled(int k) {
        for (int i = k; i >= 0; i--) {
            if (colorationOrder[i]->GetLabelBrelaz() != 0) {
                return i + 1;
            }
        }
    }

    //Esta función recibe el tiempo máximo de ejecución del algoritmo Dsatur.
    //Retorna un double que representa el tiempo que se tardó el algoritmo en
    //ejecutarse. La función está basada en la heurística Dsatur, que permite
    //obtener una solución aproximada al problema de coloración con un esfuerzo
    //computacional bajo. Si el tiempo de ejecución en algún momento excede el
    //tiempo máximo que recibió la función, el programa aborta.

    double Dsatur(int tmax) {
        clock_t startTime = clock();
        clock_t endTime;
        clock_t insideTime;
        setColorDsatur(nodesDegreeSortedArray[0]->GetLabel(), 1);
        while (numColored < numNodes) {
            const vector<GraphNode*>* maximalSaturation =
                    MaximalSaturationDegree();
            if (maximalSaturation->size() == 1) {
                int label = maximalSaturation->front()->GetLabel();
                int minColor = GetMinimumFeasibleColor(label);
                if (minColor == 0) {
                    throw string("Error en minima coloración");
                }
                setColorDsatur(label, minColor);
            } else if (maximalSaturation->size() > 1) {
                int maxDegreeLabel = 0;
                int maxDegree = -1;
                for (unsigned int i = 0; i < maximalSaturation->size(); i++) {
                    if ((*maximalSaturation)[i]->GetDegree() > maxDegree) {
                        maxDegree = (*maximalSaturation)[i]->GetDegree();
                        maxDegreeLabel = (*maximalSaturation)[i]->GetLabel();
                    }
                }
                if (maxDegree == 0) {
                    maxDegreeLabel = (*maximalSaturation)[0]->GetLabel();
                }
                int minColor = GetMinimumFeasibleColor(maxDegreeLabel);

                if (minColor == 0) {
                    throw string("Error en minima coloración");
                }
                setColorDsatur(maxDegreeLabel, minColor);
            } else {
                throw string("Error en minima coloración");
            }

            insideTime = clock();
            if ((double) (insideTime - startTime) / (double) CLOCKS_PER_SEC >= tmax) {
                return (-1);
            }
            delete maximalSaturation;
            maximalSaturation = NULL;
        }
        endTime = clock();
        return (double) (endTime - startTime) / (double) CLOCKS_PER_SEC;
    }

    // Esta función ejecuta el algoritmo de Brélaz, que busca una coloración 
    // óptima, realizando una modificación al algoritmo de brown que le permita
    // colorear primero una clique maximal aproximada y además que acote los
    // colores según una cota superior establecida por la Eurística Dsatur.
    // recibe un grafo al cual se le aplica Dsatur para calcular la clique y la 
    // cota superior y el tiempo máximo de ejecución. Retorna el tiempo de 
    // ejecución si el algoritmo no excedió el tmax y -1 en caso contrario.  

    double Brelaz(int tmax, Graph& grafo2) {
        double t = grafo2.Dsatur(tmax);
        int q = grafo2.getNumberOfColors();
        list<GraphNode*> clique = GetClique(grafo2);
        int w = clique.size();
        setColorationOrderBrelaz(grafo2, w);
        bool back = false;
        int k = w + 1;
        int maximalActualColoration[numNodes];

        if (w == q) {
            copyFinalToNodesArrayBrelaz();
            return (t);
        }

        clock_t startTime = clock();
        for (int i = 0; i < w; i++) {
            colorationOrder[i]->SetColor(i + 1);
            colorationOrder[i]->SetLabelBrelaz(k);
            maximalActualColoration[i] = i + 1;
        }

        while (true) {
            if (((clock() - startTime) / (double) CLOCKS_PER_SEC) >= (double) tmax)
                return -1;

            if (!back) {
                int uk = maximalActualColoration[k - 2];
                GetFeasibleColors(colorationOrder[k - 1], uk, q, k);
            } else {
                allowedColors[k - 1]->remove(colorationOrder[k - 1]->GetColor());
                colorationOrder[k - 1]->SetLabelBrelaz(0);
            }

            if (!allowedColors[k - 1]->empty() &&
                    (allowedColors[k - 1]->front() < q)) {
                allowedColors[k - 1]->sort();
                int minimumFeasibleColor = allowedColors[k - 1]->front();
                colorationOrder[k - 1]->SetColor(minimumFeasibleColor);
                if (minimumFeasibleColor > maximalActualColoration[k - 2]) {
                    maximalActualColoration[k - 1] = minimumFeasibleColor;
                } else {
                    maximalActualColoration[k - 1] = maximalActualColoration[k - 2];
                }
                k++;
                if (k > numNodes) {
                    copyNodesArrayBrelaz();
                    q = getNumberOfColors();
                    if (q == w) {
                        copyFinalToNodesArrayBrelaz();
                        return (clock() - startTime) / (double) CLOCKS_PER_SEC;
                    }
                    for (int i = 0; i < numNodes; i++) {
                        if (colorationOrder[i]->GetColor() == q) {
                            k = i + 1;
                            break;
                        }
                    }
                    for (int i = k - 1; i < numNodes; i++) {
                        colorationOrder[i]->SetLabelBrelaz(0);
                    }
                    back = true;
                } else {
                    back = false;
                }
            } else {
                back = true;
            }
            if (back) {
                Label(colorationOrder[k - 1], k - 1, maximalActualColoration[k - 2]);
                k = maximalRankLabeled(k - 1);
                for (int i = k; i < numNodes; i++) {
                    allowedColors[i]->clear();
                }
                if (k <= w) {
                    copyFinalToNodesArrayBrelaz();
                    return (clock() - startTime) / (double) CLOCKS_PER_SEC;
                }
            }
        }
    }

    int initialColorationDynamic(int bestPartialNumColors[]) {

    }

    int colorForwardDynamic(int node_label, int* bestNumColors,
            int bestPartialNumColors[]) {

    }

    double BrownDynamicReordering(int tmax) {

    }

private:

    //Arreglo de adyacencias del grafo
    vector<GraphNode*> **adjacencyArray;

    //Arreglo de nodos ordenado por etiqueta
    GraphNode** nodesArray;

    //Arreglo de nodos ordenado por etiqueta
    GraphNode** colorationOrder;

    //Arreglo de nodos ordenado por grado
    GraphNode** nodesDegreeSortedArray;

    //Arreglo de nodos que almacena la coloración final obtenida por el
    //algoritmo de Brown
    GraphNode** finalColorationBrown;

    //Número de nodos del grafo
    int numNodes;

    //Número de nodos ya coloreados por el algoritmo dsatur
    int numColored;

    //Número de lados del grafo
    int numEdges;

    //Lista de nodos aún no coloreados por el algoritmo dsatur
    list<GraphNode*> * uncoloredNodes;

    //Arreglo de listas de colores permitidos para cada nodo
    list<int> **allowedColors;

    //Arreglo de nodos ordenados de acuerdo a la estrategia greedy
    //propuesta por Brown
    GraphNode** greedyOrdering;

    //Este método ordena los apuntadores a nodos según su grado (de mayor grado
    //a menor grado)

    void sortNodesByDegree() {
        CompareDegreeFunctor comparer;
        sort(nodesDegreeSortedArray, nodesDegreeSortedArray + numNodes, comparer);
    }

    //Este método inicializa los arreglos de nodos del grafo, creando los objetos
    //de la clase GraphNode que describen al grafo.

    void initializeNodesArrays() {
        for (int i = 0; i < numNodes; i++) {
            nodesArray[i] = nodesDegreeSortedArray[i] = new GraphNode();
            finalColorationBrown[i] = new GraphNode();
            nodesArray[i]->SetLabel(i + 1);
        }
    }

    //Este método se utiliza para inicializar el arreglo de adyacencias,
    //asignándole un vector a cada posición, donde estarán contenidos apuntadores
    //a los nodos adyacentes al i-ésimo nodo.

    void initializeAdjancencyArray() {
        for (int i = 0; i < numNodes; i++) {
            adjacencyArray[i] = new vector<GraphNode*>;
        }
    }

    //Este método se utiliza para inicializar el arreglo de allowedColors,
    //asignándole un vector a cada posición, donde estarán contenidos apuntadores
    //a los nodos adyacentes al i-ésimo nodo.

    void initializeAllowedColors() {
        for (int i = 0; i < numNodes; i++) {
            allowedColors[i] = new list<int>;
        }
    }

    //Esta función recibe la etiqueta de un nodo y retorna un entero que
    //representa el mínimo color con el que se puede colorear dicho nodo.
    //Revisa en la lista de adyacencia en la posición del nodo con esa etiqueta
    //y marca como utilizados los colores de los vecinos del nodo, de esta
    //manera se sabe de qué colores no puede ser coloreado el grafo. Luego
    //a partir del número uno, si se puede colorear con ese color, de no ser así,
    //revisa si puede colorearlo de color 2, y así sucesivamente hasta que
    //encuentra el mínimo color posible.

    int GetMinimumFeasibleColor(int node_label) {
        if (node_label <= 0 || node_label > numNodes) {
            throw string("Etiqueta de nodo invalida en Graph::GetMinimumFeasibleColor");
        }

        bool adjacentColors[numNodes];
        bool * colorMinimo;
        int color = 0;

        for (int i = 0; i < numNodes; i++)
            adjacentColors[i] = false;

        vector<GraphNode*> * adjacentNodes =
                adjacencyArray[greedyOrdering[node_label - 1]-> GetLabel() - 1];

        for (unsigned int i = 0; i < adjacentNodes->size(); i++) {
            color = ((*adjacentNodes)[i])->GetColor();
            if (color > 0)
                adjacentColors[color - 1] = true;
        }

        if ((colorMinimo = find(adjacentColors, adjacentColors + numNodes, false))
                < adjacentColors + numNodes)
            return colorMinimo - adjacentColors + 1;
        else
            return 0;
    }

    void GetFeasibleColorsBrown(int node_label, int best_num_colors,
            int best_num_colors_partial_solution) {
        if (node_label <= 0 || node_label > numNodes) {
            throw string("Etiqueta de nodo invalida en Graph::GetMinimumFeasibleColor");
        }

        int min;
        if (best_num_colors_partial_solution + 1 < best_num_colors - 1)
            min = best_num_colors_partial_solution + 1;
        else
            min = best_num_colors - 1;


        int color;
        list<int> * nodeAllowedColors = allowedColors[node_label - 1];
        bool used_colors[min];
        for (int i = 0; i < min; i++)
            used_colors[i] = false;

        vector<GraphNode*> * adjacentNodes =
                adjacencyArray[greedyOrdering[node_label - 1]-> GetLabel() - 1];

        for (unsigned int i = 0; i < adjacentNodes->size(); i++) {
            color = ((*adjacentNodes)[i])->GetColor();
            if (((*adjacentNodes)[i]->GetRank() < node_label - 1) && color > 0
                    && color - 1 < min)
                used_colors[color - 1] = true;
        }

        nodeAllowedColors->clear();
        for (int i = 0; i < min; i++)
            if (used_colors[i] == false)
                nodeAllowedColors->push_back(i + 1);
    }

    //Asigna una coloracion inicial factible al grafo, que será empleada en el
    //algoritmo de Brown

    int initialColoration(int bestPartialNumColors[]) {
        int color = 0;
        greedyOrdering[0]->SetColor(1);
        bestPartialNumColors[0] = 1;

        int initialNumColors = 1;

        for (int i = 1; i < numNodes; i++) {
            GetFeasibleColorsBrown(i + 1, numNodes + 1,
                    bestPartialNumColors[i - 1]);
            if (allowedColors[i]->empty())
                throw string("No se pudo completar la coloracion inicial");
            //CUIDADO
            color = allowedColors[i]->front();
            allowedColors[i]->pop_front();
            greedyOrdering[i]->SetColor(color);
            if (color > initialNumColors)
                initialNumColors = color;
            if (color > bestPartialNumColors[i - 1])
                bestPartialNumColors[i] = color;
            else
                bestPartialNumColors[i] = bestPartialNumColors[i - 1];
        }
        copyNodesArray();
        return initialNumColors;
    }

    //Retorna la etiqueta del primer nodo con el color pasado como parametro.

    int findByColor(int color) {
        for (int i = 0; i < numNodes; i++) {
            if (greedyOrdering[i]->GetColor() == color)
                return i + 1;
        }
        throw string("No se encontro el color"
                " especificado en el metodo findByColor");
    }

    //Recolorea con 0 los nodos a partir del nodo con la etiqueta pasada como
    //parametro

    void resetColoration(int initialPosition) {
        for (int i = initialPosition - 1; i < numNodes; i++) {
            greedyOrdering[i]->SetColor(0);
        }
    }

    //Retorna true si y solo si se encontro un color alternativo factible, mayor
    //que el color actual del nodo con etiqueta node_label, y menor que el color
    //bestNumColors, pasado como parametro

    bool tryNewColor(int nodeLabel, int bestNumColors) {
        int alternativeColor;
        if (!(allowedColors[nodeLabel - 1]->empty())) {
            alternativeColor = allowedColors[nodeLabel - 1]->front();
            allowedColors[nodeLabel - 1]->pop_front();
            if (alternativeColor < bestNumColors) {
                greedyOrdering[nodeLabel - 1]->SetColor(alternativeColor);
                return true;
            }
        }
        return false;
    }

    //Intenta colorear a partir del nodo con etiqueta nodeLabel. Retorna la
    //etiqueta del ultimo nodo que se pudo colorear exitosamente (i.e. con un
    //color menor al numero de colores de la mejor solucion obtenida hasta el
    //momento por el algoritmo de Brown). Si logra colorear todos los nodos
    //exitosamente, actualiza el valor del parametro bestNumColors, para reflejar
    //que se encontro una nueva y mejor solución.

    int colorForward(int node_label, int* bestNumColors, int bestPartialNumColors[]) {
        int color = 0;
        int i;
        int solutionNumColors = bestPartialNumColors[node_label - 2];

        for (i = node_label - 1; i < numNodes; i++) {
            GetFeasibleColorsBrown(i + 1, *bestNumColors,
                    bestPartialNumColors[i - 1]);
            if (allowedColors[i]->empty())
                break;

            color = allowedColors[i]->front();
            allowedColors[i]->pop_front();
            greedyOrdering[i]->SetColor(color);
            if (color > solutionNumColors)
                solutionNumColors = color;
            if (color > bestPartialNumColors[i - 1])
                bestPartialNumColors[i] = color;
            else
                bestPartialNumColors[i] = bestPartialNumColors[i - 1];
        }

        if (i == numNodes) {
            *bestNumColors = solutionNumColors;
            copyNodesArray();
            return numNodes;
        }
        return i;
    }

    //Metodo helper para el algoritmo de Brown.
    //Copia el contenido del arreglo de nodos principal (nodesArray), al
    //arreglo finalColorationBrown, para almacenar un nueva mejor solucion
    //encontrada por el algoritmo de Brown.

    void copyNodesArray() {
        for (int i = 0; i < numNodes; i++) {
            *(finalColorationBrown[i]) = *(greedyOrdering[i]);
        }
    }

    //Metodo helper para el algoritmo de Brelaz.
    //Copia el contenido del arreglo de nodos principal (nodesArray), al
    //arreglo finalColorationBrown, para almacenar un nueva mejor solucion
    //encontrada por el algoritmo de Brelaz.

    void copyNodesArrayBrelaz() {
        for (int i = 0; i < numNodes; i++) {
            *(finalColorationBrown[i]) = *(colorationOrder[i]);
        }
    }

    //Metodo helper para el algoritmo de Brown.
    //Copia el contenido del arreglo finalColorationBrown, al
    //arreglo nodesArray, para almacenar la mejor solucion final encontrada por
    //el algoritmo de Brown.

    void copyFinalToNodesArray() {
        for (int i = 0; i < numNodes; i++) {
            *(greedyOrdering[i]) = *(finalColorationBrown[i]);
        }
    }

    //Metodo helper para el algoritmo de Brown.
    //Copia el contenido del arreglo finalColorationBrown, al
    //arreglo nodesArray, para almacenar la mejor solucion final encontrada por
    //el algoritmo de Brown.

    void copyFinalToNodesArrayBrelaz() {
        for (int i = 0; i < numNodes; i++) {
            *(colorationOrder[i]) = *(finalColorationBrown[i]);
        }
    }

    // Establece un orden de coloración para el algoritmo de Brélaz, basado en
    // el orden en el que se colorearon los nodos en la ejecución del algoritmo 
    // Dsatur.

    void setColorationOrderBrelaz(Graph& grafo2, int w) {
        for (int i = 0; i < numNodes; i++) {
            colorationOrder[i] = nodesArray[grafo2.colorationOrder[i]->GetLabel() - 1];
            colorationOrder[i]->SetRank(i);
            *(finalColorationBrown[i]) = *(grafo2.colorationOrder[i]);
        }
    }

    // Este método busca los colores permitidos para el node_xk tomando en 
    // cuenta como cota superior el min(uk, q), estos colores los almacena en el
    // arreglo de allowedColors en la lista correspondiente al nodo en la posi-
    // ción teórica k en el orden de coloración. Recibe el nodo, uk = número de  
    // colores de la solución parcial hasta k-1, q que es la mínima cantidad de 
    // colores encontrada en una solución completa hasta el momento y el k  

    void GetFeasibleColors(GraphNode* node_xk, int uk, int q, int k) {
        int min;
        if (uk + 1 <= q - 1) {
            min = uk + 1;
        } else {
            min = q - 1;
        }
        bool usedColors[min];
        for (int i = 0; i < min; i++) {
            usedColors[i] = false;
        }
        const vector<GraphNode*> *adyacents = neighbors(node_xk->GetLabel());
        int rank_xk = node_xk->GetRank();
        for (int i = 0; i < adyacents->size(); i++) {
            if ((*adyacents)[i]->GetColor() != 0 && (*adyacents)[i]->GetRank() < rank_xk) {
                usedColors[(*adyacents)[i]->GetColor() - 1] = true;
            }
        }
        for (int i = 0; i < min; i++) {
            if (usedColors[i] == false) {
                allowedColors[k - 1]->push_back(i + 1);
            }
        }
    }

    void initializeBrownGreedyOrderArray() {
        for (int i = 0; i < numNodes; i++) {
            //            greedyOrdering[i] = nodesDegreeSortedArray[i];
            greedyOrdering[i] = nodesArray[i];
            greedyOrdering[i]->SetRank(i);
        }
        //        printNodesArray(greedyOrdering, numNodes);
        //        int bestNumConnected;
        //        int numConnected;
        //        for (int i = 1; i < numNodes; i++) {
        //            bestNumConnected = -1;
        //            for (int j = i; j < numNodes; j++) {
        //                numConnected = 0;
        //                for (int k = 0; k < i; k++) {
        //                    if (isAdyacent(greedyOrdering[k], greedyOrdering[j]))
        //                        numConnected++;
        //                }
        //                if (numConnected > bestNumConnected) {
        //                    swap(greedyOrdering, j, i);
        //                    bestNumConnected = numConnected;
        //                } else if (numConnected == bestNumConnected) {
        //                    if ((greedyOrdering[i]->GetDegree()) < (greedyOrdering[j]->GetDegree()))
        //                        swap(greedyOrdering, j, i);
        //                }
        //            }
        //            greedyOrdering[i]->SetRank(i);
        //        }
    }

    void swap(GraphNode** nodesArray, int indexNode1, int indexNode2) {
        GraphNode* temp = nodesArray[indexNode1];
        nodesArray[indexNode1] = nodesArray[indexNode2];
        nodesArray[indexNode2] = temp;
    }

    void printNodesArray(GraphNode** array, int size) {
        for (int i = 0; i < size; i++)
            cout << "Nodo " << i + 1 << ": " <<
                array[i]->GetLabel() << " " << array[i]->GetDegree() << " " << array[i]->GetRank() << endl;
    }
};
#endif	/* GRAPH_H */

