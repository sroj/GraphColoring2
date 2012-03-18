/* 
 * File:   CompareLabelFunctor.h
 * Author: simonr
 *
 * Created on March 17, 2012, 9:28 PM
 */

#ifndef COMPARELABELFUNCTOR_H
#define	COMPARELABELFUNCTOR_H

#include "GraphNode.h"

//Clase CompareLabelFunctor
//Esta clase representa un functor que permite comparar dos objetos de la clase
//GraphNode de acuerdo a su etiqueta, para implementar un ordenamiento debil
//entre los mismos.

class CompareLabelFunctor {
public:

    bool operator() (const GraphNode* node_1, const GraphNode* node_2) {
        return (node_1->GetLabel())<(node_2->GetLabel());
    }
};


#endif	/* COMPARELABELFUNCTOR_H */

