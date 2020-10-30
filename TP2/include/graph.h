/*
Copyright (c) 2018 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#ifndef _GRAPH_H
#define _GRAPH_H

#include <vector>
#include <list>
#include <string>

// Create a graph with fixed number of nodes
// The maximum number of arcs is bounded by 4 times the number nodes
// (which is enough for grid-graphs)
// Reallocation of the vector of arcs is prevented
class GraphFlow
{
  // Types
  protected:
    class Node;

    class Arc
    {
      public:
        float capacity, flow;
        Node *pStartNode;
        Node *pEndNode;
      public:
        Arc(Node *ps=nullptr, Node *pe=nullptr, float cap=0.0)
        {
            pStartNode = ps;
            pEndNode = pe;
            capacity = cap;
            flow = 0.0;
        }
    };

    class Node
    {
      public:
        // Each arc is shared between two nodes, so these lists do not own the arcs.
        // They rather store pointers to arcs
        // Arcs themselves are actually stored the 'arcs' member of GraphFlow
        std::list<Arc *> arcsIn;
        std::list<Arc *> arcsOut;

        // Indicate if node has been reached in the breadth-first searches
        // (both for finding the augmenting path and the s/t-cut)
        bool reached;

        // Arc taken to reach the node (for the shortest path)
        Arc *pAugPathArc;

      public:
        Node() {reached=false; pAugPathArc=nullptr;}
    };

  // Member variables
  private:
    std::vector<Node> nodes;
    std::vector<Arc> arcs;

  // Member functions
  protected:
    void connectNodesPrivate(Node *, Node *, float);
    void resetFlow();

    // The augmenting path is chosen as the shortest path from the source to the sink,
    // taking non-saturated arcs only
    // We use Dijkstra's algorithm, the cost of non-saturated arcs being 1
    void findAugmentingPath();

  public:
    GraphFlow();

    // Create a graph, with a given number of non-terminal nodes i.e. apart from source and sink
    // (source and sink are automatically added)
    // Param: [in] number of non-terminal nodes
    void setNbNodes(unsigned int);

    // Return number of non-terminal nodes
    unsigned int getNbNodes() const;

    // Return total number of arcs (including arcs from source and to sink)
    unsigned int getNbArcs() const;

    // Connect to non-terminal nodes with an arc
    // Params: [in] index of start node, [in] index of end node, [in] capacity
    void connectNodes(unsigned int, unsigned int, float);

    // Connect source to a non-terminal node with an arc
    // Params: [in] index of end node, [in] capacity
    void connectSourceToNode(unsigned int, float);

    // Connect a non-terminal node to the sink with an arc
    // Params: [in] index of start node, [in] capacity
    void connectNodeToSink(unsigned int, float);

    // Maximize flow with Ford-Fulkerson method
    // Actually, since the augmenting path is always the shortest one (in terms of arcs taken),
    // this is Edmonds-Karp algorithm
    void FordFulkerson();

    // Build s/t cut by propagating, from source, along non-saturated arcs
    // Params: [out] vector containing indices of non-terminal nodes in the S set
    void cutFromSource(std::vector<unsigned int> &setS);

    void print() const;

    std::string nodeName(const Node *) const;
};


#endif
