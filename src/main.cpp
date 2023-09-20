#include <iostream>

#include <igraph/igraph.h>
#include <libleidenalg/GraphHelper.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>

#include "library.h"
#include "obj.h"

int main(int argc, char* argv[]) {
    std::cout<<"main"<<std::endl;
    /* Library* library = new Library(); */
    /* std::cout<<library->square(5)<<std::endl; */
    /* Obj* obj = new Obj(); */
    /* std::cout<<obj->test_obj()<<std::endl; */
    igraph_t g;
    igraph_famous(&g, "Zachary");

    Graph graph(&g);
    CPMVertexPartition part(&graph, 0.05);
    Optimiser o;
    o.optimise_partition(&part);
    std::cout << "Node Community" << std::endl;
    for(int i = 0; i < graph.vcount(); i++) {
        std::cout << i << " " << part.membership(i) << std::endl;
    }
    igraph_destroy(&g);
}
