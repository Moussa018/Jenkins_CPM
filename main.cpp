#include <iostream>
#include "Node.h"
#include "Graph.h"
#include "CPMSolver.h"


int main() {
  Graph graph;

    graph.addStage("checkout", 2);
    graph.addStage("build", 5);
    graph.addStage("test", 8);
    graph.addStage("lint", 3);
    graph.addStage("deploy", 4);  
    
    graph.addDependency("build", "checkout");   
    graph.addDependency("lint", "checkout");    
    graph.addDependency("test", "build");     
    graph.addDependency("deploy", "test");      
    graph.addDependency("deploy", "lint"); 
    


    CPMSolver solver(&graph);
    solver.solve();

     std::cout << "=== Résultats CPM ===\n";
    std::cout << "Stage\t\tDuration\tEST\tLST\tSlack\n";
    for (Node* node : graph.getNodes()) {
        std::cout << node->getName() << "\t\t"
                   << node->getDuration() << "\t\t"
                   << node->getEarliestStartTime() << "\t"
                   << node->getLatestStartTime() << "\t"
                   << node->getSlack() << "\n";
    }

    // 5. Affichage du chemin critique
    std::cout << "\n=== Chemin critique ===\n";
    for (Node* node : solver.getCriticalPath()) {
        std::cout << node->getName() << " ";
    }
    std::cout << "\n";
    
    return 0;

};