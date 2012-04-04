/*
 * File:   CompareUnusedAttFunctor.h
 * Author: simonr
 *
 * Created on April 3, 2012, 2:45 AM
 */

#ifndef COMPAREUNUSEDATTFUNCTOR_H
#define	COMPAREUNUSEDATTFUNCTOR_H

#include "GraphNode.h"

class CompareUnusedAttFunctor {
public:

    bool operator() (const GraphNode* node_1, const GraphNode* node_2) {
        return (node_1->GetUnusedAttributes())>(node_2->GetUnusedAttributes());
    }

};

#endif	/* COMPAREUNUSEDATTFUNCTOR_H */

