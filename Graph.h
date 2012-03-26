/* 
 * File:   Graph.h
 * Author: simonr
 *
 * Created on March 17, 2012, 9:30 PM
 */

#ifndef GRAPH_H
#define	GRAPH_H

#include <list>
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
                    initializeNodesArrays();
                    initializeAdjancencyArray();
                    primeraLineaLeida = true;
                    break;
                }
                token = strtok(NULL, " ");
            }
        }
        instancia.close();
        uncoloredNodes = new list<GraphNode*> (nodesArray, nodesArray + numNodes);
        sortNodesByDegree();
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
        output << "-----------------------------------";
        output << "\nTIEMPO DE EJECUCION: " << fixed << time << "\n";
        output << "NUMERO DE COLORES ENCONTRADOS: " << getNumberOfColors()
                << "\n";
        output << "COLORACION ENCONTRADA:\n";
        output << "Vertice\tColor\n";

        for (int i = 0; i < numNodes; i++) {
            output << i + 1 << "\t" << nodesArray[i]->GetColor() << "\n";
        }
        output << "-----------------------------------\n";
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

    //Ejecuta el algoritmo de coloracion de Brown sobre el grafo.
    //Recibe un parametro que indica el tiempo maximo, en segundos, durante el
    //el cual se permitira la ejecución del método. Retorna el tiempo de
    //ejecución del algoritmo, o -1 en caso de que se agote el tiempo máximo.

    double Brown(int tmax) {
        clock_t startTime = clock();
        int nodeLabel = numNodes;
        int bestNumColors = initialColoration();
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
            nodeLabel = colorForward(nodeLabel + 1, &bestNumColors);

            if (nodeLabel == numNodes) {
                backtracking = false;
                continue;
            } else {
                backtracking = true;
                continue;
            }
        }

        copyFinalToNodesArray();
        return (clock() - startTime) / (double) CLOCKS_PER_SEC;
    }
    
    list<GraphNode*> GetClique(Graph& grafo2){
        list<GraphNode*> clique;
        for(int i=0; i<numNodes; i++){            
            clique.push_back(nodesArray[grafo2.colorationOrder[i]->GetLabel() -1]);
            nodesArray[grafo2.colorationOrder[i]->GetLabel() - 1]->SetInClique(true);
            if(grafo2.colorationOrder[i+1]->GetColor() <= grafo2.colorationOrder[i]->GetColor()){
                break;
            }
        }
        return clique;
    }

    void setColorationOrderBrelaz(Graph& grafo2){
        for(int i=0;i<numNodes; i++){
            colorationOrder[i] = nodesArray[grafo2.colorationOrder[i]->GetLabel() - 1];
        }
    }
    
    void GetFeasibleColors(GraphNode* node_xk, int uk, int q, list<int> * colors){
        int min;
        if(uk + 1 <= q-1){
            min = uk +1;
        }
        else{
            min = q-1;
        }
        
        bool usedColors[min];
        for(int i=0; i<min; i++){
            usedColors[i]=false;
        }
        
        const vector<GraphNode*> *adyacents = neighbors(node_xk->GetLabel());
        for(int i=0; i<adyacents->size(); i++){
            if((*adyacents)[i]->GetColor() !=0){
                usedColors[(*adyacents)[i]->GetColor() - 1] = true;
            }
        }
        
        for(int i=0; i<min; i++){
            if(usedColors[i]==false){
                (*colors).push_back (i+1);//error
                
                cout << "error";
            }
        }
        
    }
    
    bool isAdyacent(GraphNode* node1, GraphNode* node2){
        const vector<GraphNode*> *adjacents = neighbors(node1->GetLabel());
        bool found=false;
        for(int i=0;i<adjacents->size() && !found;i++){
            if((*adjacents)[i]->GetLabel()== node2->GetLabel()){
                found = true;
            }
        }
        return found;
    }
    
    void Label(GraphNode* node_xk, int position){
        int numColors = getNumberOfColors();
        bool usedColors[numColors];
        for(int i=0; i<numColors; i++){
            usedColors[i]=false;
        }
        
        for(int i=0; i<position; i++){
            if(colorationOrder[i]->GetLabelBrelaz()==0 && isAdyacent(node_xk, colorationOrder[i])){
                if(usedColors[colorationOrder[i]->GetColor()-1]==false){
                    colorationOrder[i]->SetLabelBrelaz(position);
                    usedColors[colorationOrder[i]->GetColor()-1]=true;
                }
            }
        }
    }
    
    int maximalRankLabeled(){
        // puede mejorarse empezando en k
        for(int i=numNodes-1;i >= 0;i--){
            if(colorationOrder[i]->GetLabelBrelaz() != 0){
                return i+1;
            }
        }

    }
    
    double Brelaz(int tmax, Graph& grafo2) {
        grafo2.Dsatur(tmax);
        int q = grafo2.getNumberOfColors();
        list<GraphNode*> clique = GetClique(grafo2);
        int w = clique.size();
        setColorationOrderBrelaz(grafo2);
        bool back = false;
        int k = w+1;
        
        //medir tiempo
        
        for(int i=0;i < w;i++){
            colorationOrder[i]->SetColor(i+1);
            numColored++;
            colorationOrder[i]->SetLabelBrelaz(k);
        }
        
        while(true){
            list<int> * colors;
            if(!back){
                int uk=getNumberOfColors();
                cout << uk;
                GetFeasibleColors(colorationOrder[k-1], uk, q, colors);
            }
            else{
                GetFeasibleColors(colorationOrder[k-1], getNumberOfColors(), q, colors);
                colors->remove(colorationOrder[k-1]->GetColor());
                colorationOrder[k-1]->SetLabelBrelaz(0);
            }
            
            if(colors->size()!=0){
                colorationOrder[k-1]->SetColor(colors->front());
                k++;
                if(k > numNodes){
                    q=getNumberOfColors();
                    if(q==w){
                        //medir tiempo, restar
                        return(tmax);
                    }
             //Se puede mejorar colocando int i= w-1
                    bool found= false;
                    for(int i= 0;i<numNodes && !found; i++){
                        if(colorationOrder[i]->GetColor()==q){
                            k = i +1; //revisar
                            found=true;
                        }
                    }
                    for(int i= k-1; i<numNodes; i++){
                        colorationOrder[i]->SetLabelBrelaz(0);
                    }
                    back=true;
                }
                else{
                    back=false;
                }
            }
            else{
                back=true;
            }
            if(back){
                Label(colorationOrder[k-1], k-1);
                k= maximalRankLabeled();
                if(k<=w){
                    //Calcular tiempo
                    return(tmax);
                }
            }
            delete colors;
        }
        
        delete &clique;
        return tmax;
    }

    double BrownLookAhead(int tmax) {

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

        vector<GraphNode*> * adjacentNodes = adjacencyArray[node_label - 1];

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

    //Metodo para el algoritmo de Brown.
    //Busca el color minimo factible, mayor a su color actual, para el nodo con
    //etiqueta node_label.

    int GetMinimumAlternativeColor(int node_label) {
        if (node_label <= 0 || node_label > numNodes) {
            throw string("Etiqueta de nodo invalida en"
                    "Graph::GetMinimumAlternativeColor");
        }

        bool adjacentColors[numNodes];
        bool * colorMinimo;
        int color = 0;
        int currentColor = nodesArray[node_label - 1]->GetColor();

        for (int i = 0; i < numNodes; i++)
            adjacentColors[i] = false;

        vector<GraphNode*> * adjacentNodes = adjacencyArray[node_label - 1];

        for (unsigned int i = 0; i < adjacentNodes->size(); i++) {
            if (((*adjacentNodes)[i])->GetLabel() < node_label) {
                color = ((*adjacentNodes)[i])->GetColor();
                if (color > 0)
                    adjacentColors[color - 1] = true;
            }
        }

        if ((colorMinimo =
                find(adjacentColors + currentColor,
                adjacentColors + numNodes, false))
                < adjacentColors + numNodes)
            return colorMinimo - adjacentColors + 1;
        else
            return 0;
    }

    //Asigna una coloracion inicial factible al grafo, que será empleada en el
    //algoritmo de Brown

    int initialColoration() {
        int color = 0;
        nodesArray[0]->SetColor(1);
        int initialNumColors = 1;

        for (int i = 1; i < numNodes; i++) {
            if ((color = GetMinimumFeasibleColor(i + 1)) == 0)
                throw string("No se pudo completar la coloracion inicial");
            nodesArray[i]->SetColor(color);
            if (color > initialNumColors)
                initialNumColors = color;
        }
        copyNodesArray();
        return initialNumColors;
    }

    //Retorna la etiqueta del primer nodo con el color pasado como parametro.

    int findByColor(int color) {
        for (int i = 0; i < numNodes; i++) {
            if (nodesArray[i]->GetColor() == color)
                return i + 1;
        }
        throw string("No se encontro el color"
                " especificado en el metodo findByColor");
    }

    //Recolorea con 0 los nodos a partir del nodo con la etiqueta pasada como
    //parametro

    void resetColoration(int initialPosition) {
        for (int i = initialPosition - 1; i < numNodes; i++) {
            nodesArray[i]->SetColor(0);
        }
    }

    //Retorna true si y solo si se encontro un color alternativo factible, mayor
    //que el color actual del nodo con etiqueta node_label, y menor que el color
    //bestNumColors, pasado como parametro

    bool tryNewColor(int nodeLabel, int bestNumColors) {
        int alternativeColor = GetMinimumAlternativeColor(nodeLabel);
        if (alternativeColor < bestNumColors) {
            setColor(nodeLabel, alternativeColor);
            return true;
        }
        return false;
    }

    //Intenta colorear a partir del nodo con etiqueta nodeLabel. Retorna la
    //etiqueta del ultimo nodo que se pudo colorear exitosamente (i.e. con un
    //color menor al numero de colores de la mejor solucion obtenida hasta el
    //momento por el algoritmo de Brown). Si logra colorear todos los nodos
    //exitosamente, actualiza el valor del parametro bestNumColors, para reflejar
    //que se encontro una nueva y mejor solución.

    int colorForward(int nodeLabel, int* bestNumColors) {
        int color = 0;
        int i;
        for (i = nodeLabel - 1; i < numNodes; i++) {
            if ((color = GetMinimumFeasibleColor(i + 1)) == 0)
                throw string("No se pudo completar la coloracion");
            if (color >= *bestNumColors)
                break;
            nodesArray[i]->SetColor(color);
        }
        if (i == numNodes) {
            int max = 0;
            int currentColor;
            for (int j = 0; j < numNodes; j++) {
                currentColor = nodesArray[j]->GetColor();
                if (currentColor > max)
                    max = currentColor;
            }
            *bestNumColors = max;
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
            *(finalColorationBrown[i]) = *(nodesArray[i]);
        }
    }

    //Metodo helper para el algoritmo de Brown.
    //Copia el contenido del arreglo finalColorationBrown, al
    //arreglo nodesArray, para almacenar la mejor solucion final encontrada por
    //el algoritmo de Brown.

    void copyFinalToNodesArray() {
        for (int i = 0; i < numNodes; i++) {
            *(nodesArray[i]) = *(finalColorationBrown[i]);
        }
    }

};
#endif	/* GRAPH_H */

