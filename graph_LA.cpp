/*
Copyright (c) 2018 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/


#include "graph.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <assert.h>

using namespace std;

GraphFlow::GraphFlow()
{
}

void GraphFlow::setNbNodes(unsigned int nbNodes)
{
    nodes.resize(nbNodes+2);

    // If a 4-connected grid is used, we expect about 6 arcs per non-terminal nodes
    arcs.clear();
    arcs.reserve((nbNodes+2)*6);
}

unsigned int GraphFlow::getNbNodes() const
{
    if (nodes.size()>2)
        return nodes.size()-2;
    else
        return 0;
}

unsigned int GraphFlow::getNbArcs() const
{
    return arcs.size();
}

void GraphFlow::resetFlow()
{
    for (Arc &arc : arcs)
        arc.flow = 0.0;
}

void GraphFlow::connectNodesPrivate(Node *pNode1, Node *pNode2, float cap)
{
    list<Arc *>::iterator itAdj1, itAdj2;
    Arc *pArcOut=nullptr, *pArcIn=nullptr;

    itAdj1 = pNode1->arcsOut.begin();
    while (itAdj1!=pNode1->arcsOut.end() && (*itAdj1)->pEndNode!=pNode2)
        itAdj1++;
    if (itAdj1!=pNode1->arcsOut.end())
        pArcOut = *itAdj1;

    itAdj2 = pNode2->arcsIn.begin();
    while (itAdj2!=pNode2->arcsIn.end() && (*itAdj2)->pStartNode!=pNode1)
        itAdj2++;
    if (itAdj2!=pNode2->arcsIn.end())
        pArcIn = *itAdj2;

    assert(pArcIn==pArcOut);

    if (pArcOut!=nullptr && pArcIn!=nullptr)
        pArcOut->capacity = cap;
    else if (pArcOut==nullptr && pArcIn==nullptr)
    {
        assert(arcs.size()<arcs.capacity());

        arcs.push_back(Arc(pNode1, pNode2, cap));
        pNode1->arcsOut.push_back(&arcs.back());
        pNode2->arcsIn.push_back(&arcs.back());
    }
}

void GraphFlow::connectNodes(unsigned int idxNode1, unsigned int idxNode2, float cap)
{
    Node *pNode1, *pNode2;

    assert(idxNode1<nodes.size()-2 && idxNode2<nodes.size()-2);

    pNode1 = &nodes[idxNode1+2];
    pNode2 = &nodes[idxNode2+2];

    connectNodesPrivate(pNode1, pNode2, cap);
}

void GraphFlow::connectSourceToNode(unsigned int idxNode, float cap)
{
    Node *pNode1, *pNode2;

    assert(idxNode<nodes.size()-2);

    pNode1 = &nodes[0];
    pNode2 = &nodes[idxNode+2];

    connectNodesPrivate(pNode1, pNode2, cap);
}

void GraphFlow::connectNodeToSink(unsigned int idxNode, float cap)
{
    Node *pNode1, *pNode2;

    assert(idxNode<nodes.size()-2);

    pNode1 = &nodes[idxNode+2];
    pNode2 = &nodes[1];

    connectNodesPrivate(pNode1, pNode2, cap);
}

void GraphFlow::findAugmentingPath()
{
    // Breadth-first seach from source node
    vector<Node *> Q; // Queue of visited nodes
    Node *pCurrentNode;
    Q.reserve(nodes.size());
    unsigned int current = 0;

    for (Node &node: nodes)
    {
        node.reached = false;
        node.pAugPathArc = nullptr;
    }

    nodes[0].reached = true;
    Q.push_back(&nodes[0]);

    while (current<Q.size() && !nodes[1].reached)
    {
        pCurrentNode = Q[current];
        for (Arc *pArc : pCurrentNode->arcsOut)
        {
            if (pArc->flow<pArc->capacity && !pArc->pEndNode->reached)
            {
                // Arc is not saturated, so add its endnode to the queue
                Q.push_back(pArc->pEndNode);
                pArc->pEndNode->reached = true;
                pArc->pEndNode->pAugPathArc = pArc;
            }
        }
        current++;
    }
}

void GraphFlow::FordFulkerson()
{
    bool bPathFound;

    if (nodes.size()<=2)
        return;

    resetFlow();

    do {
        findAugmentingPath();
        bPathFound = nodes[1].reached;

        if (bPathFound)
        {
            // Ok, an augmenting path has just been found
            // It is stored using the pAugPathArc member in the nodes

            // We start from the sink
            Node *pCurrentNode = &nodes[1];

            // TODO 2 : Backtrack along augmenting path and get residual capacity
            // which is the minimum residual capacity of arcs taken.
            // Backtrack starts for the sink and iterates until the source is reached

            // Then, perform a second backtrack to update flows along the augmenting path
            // Add code here...
            float res_cap_arc;
            float min_res_cap = 0;

            cout<<"Do 1er backtrack"<<endl;

            do {
                res_cap_arc = pCurrentNode->pAugPathArc->capacity - pCurrentNode->pAugPathArc->flow;
                cout<<"cap : "<<res_cap_arc<<endl;
                if (res_cap_arc < min_res_cap) {
                    min_res_cap = res_cap_arc;
                }
                pCurrentNode = pCurrentNode->pAugPathArc->pStartNode;
            } while (pCurrentNode != &nodes[0]);

            cout<<"Fin de 1er backtrack"<<endl;

            pCurrentNode = &nodes[1];
            do {
                pCurrentNode->pAugPathArc->flow += min_res_cap;
                pCurrentNode = pCurrentNode->pAugPathArc->pStartNode;
            } while (pCurrentNode != &nodes[0]);

            cout<<"Fin de 2eme backtrack"<<endl;

        }
    } while (bPathFound);
}

void GraphFlow::cutFromSource(vector<unsigned int> &vectS)
{
    // Breadth-first seach from source node
    vector<Node *> Q; // Queue of visited nodes
    Node *pCurrentNode;
    Q.reserve(nodes.size());
    unsigned int current = 0;

    if (nodes.size()<=2)
        return;

    for (Node &node: nodes)
        node.reached = false;

    nodes[0].reached = true;
    Q.push_back(&nodes[0]);

    while (current<Q.size())
    {
        pCurrentNode = Q[current];
        for (Arc *pArc : pCurrentNode->arcsOut)
        {
            if (pArc->flow<pArc->capacity && !pArc->pEndNode->reached)
            {
                // Arc is not saturated, so add its endnode to the queue
                Q.push_back(pArc->pEndNode);
                pArc->pEndNode->reached = true;
            }
        }
        current++;
    }

    vectS.clear();
    vectS.reserve(nodes.size());

    unsigned int iNode;
    for (iNode=2; iNode<nodes.size(); iNode++)
    {
        if (nodes[iNode].reached)
            vectS.push_back(iNode-2);
    }
}

void GraphFlow::print() const
{
    for (const Node &node: nodes)
    {
        cout<<"Node "<<nodeName(&node)<<endl;
        for (const Arc *pArc: node.arcsOut)
        {
            cout<<"  ->"<<nodeName(pArc->pEndNode)<<" : "<<pArc->flow<<"/"<<pArc->capacity<<endl;

        }
        for (const Arc *pArc: node.arcsIn)
        {
            cout<<"  "<<nodeName(pArc->pStartNode)<<"-> : "<<pArc->flow<<"/"<<pArc->capacity<<endl;
        }
    }
}

string GraphFlow::nodeName(const Node *pNode) const
{
    string name;
    if (pNode==&nodes[0]) name = "s";
    else if (pNode==&nodes[1]) name = "t";
    else {
        stringstream ss;
        ss<<(int)(pNode-&nodes[0])-2;
        name = ss.str();
    }
    return name;
}
