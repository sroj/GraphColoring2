/*
 * File:   GraphNode.h
 * Author: simonr
 *
 * Created on March 17, 2012, 9:31 PM
 */

#ifndef GRAPHNODE_H
#define	GRAPHNODE_H

//Clase GraphNode
//Clase que representa un nodo de grafo. Almacena el color, el grado de
//saturacion, el grado y la etiqueta del nodo en cuestión.

class GraphNode {
public:

    GraphNode() : color(0), saturationDegree(0), degree(0), label(0),
    in_clique(false), label_brelaz(0), rank(0) {
    };

    //Copy constructor

    GraphNode(const GraphNode& orig) {
        color = orig.color;
        saturationDegree = orig.saturationDegree;
        degree = orig.degree;
        label = orig.label;
        label_brelaz = orig.label_brelaz;
        in_clique = orig.in_clique;
    };

    int GetSaturationDegree() const {
        return saturationDegree;
    }

    void SetSaturationDegree(int SaturationDegree) {
        this->saturationDegree = SaturationDegree;
    }

    void IncrementSaturationDegree() {
        this->saturationDegree++;
    }

    int GetColor() const {
        return color;
    }

    void SetColor(int color) {
        this->color = color;
    }

    int GetDegree() const {
        return degree;
    }

    void SetDegree(int degree) {
        this->degree = degree;
    }

    int GetLabel() const {
        return label;
    }

    void SetLabel(int label) {
        this->label = label;
    }

    void SetLabelBrelaz(int label) {
        this->label_brelaz = label;
    }

    int GetLabelBrelaz() const {
        return label_brelaz;
    }

    void SetInClique(bool in) {
        this->in_clique = in;
    }

    bool GetInClique() {
        return in_clique;
    }

    int GetRank() const {
        return rank;
    }

    void SetRank(int rank) {
        this->rank = rank;
    }

    //Operador ++
    //Se redefine para que incremente en 1 el grado del nodo

    GraphNode& operator++() {
        ++degree;
        return *this;
    }

private:

    //Color del nodo
    int color;

    //Grado de saturacion del nodo
    int saturationDegree;

    //Grado del nodo
    int degree;

    //Etiqueta del nodo
    int label;

    bool in_clique;

    int label_brelaz;

    int rank;
};
#endif	/* GRAPHNODE_H */

