/* 
 * File:   CompareDegreeFunctor.h
 * Author: simonr
 *
 * Created on March 17, 2012, 9:27 PM
 */

#ifndef COMPAREDEGREEFUNCTOR_H
#define	COMPAREDEGREEFUNCTOR_H

#include "GraphNode.h"

//Clase CompareDegreeFunctor
//Esta clase representa un functor que permite comparar dos objetos de la clase
//GraphNode de acuerdo a su grado, para implementar un ordenamiento debil entre
//los mismos.

class CompareDegreeFunctor {
public:

    bool operator() (const GraphNode* node_1, const GraphNode* node_2) {
        return (node_1->GetDegree())>(node_2->GetDegree());
    }
};

#endif	/* COMPAREDEGREEFUNCTOR_H */

