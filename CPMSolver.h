#pragma once
#include <vector>
#include <algorithm>
#include <climits>
#include "Graph.h"
#include "Node.h"

class CPMSolver {
private:
    Graph* graph;

public:
    CPMSolver(Graph* graph) : graph(graph) {}

    void forwardPass() {
        std::vector<Node*> order = graph->getTopologicalOrder();
        for (Node* node : order) {
            int earliest = 0;
            for (Node* dependency : node->getDirectDependencies()) {
                earliest = std::max(earliest, dependency->getEarliestStartTime() + dependency->getDuration());
            }
            node->setEarliestStartTime(earliest);
        }
    }

    void backwardPass() {
        std::vector<Node*> order = graph->getTopologicalOrder();
        // On parcourt dans l'ordre INVERSE de l'ordre topologique
        std::reverse(order.begin(), order.end());

        for (Node* node : order) {
            std::vector<Node*> dependents = graph->getDependents(node);

            if (dependents.empty()) {
                // Pas de successeur : le LST = EST (pas de marge imposée par la suite)
                node->setLatestStartTime(node->getEarliestStartTime());
            } else {
                int latest = INT_MAX;
                for (Node* dependent : dependents) {
                    latest = std::min(latest, dependent->getLatestStartTime() - node->getDuration());
                }
                node->setLatestStartTime(latest);
            }
        }
    }

    void calculateSlack() {
        for (Node* node : graph->getNodes()) {
            int slack = node->getLatestStartTime() - node->getEarliestStartTime();
            node->setSlack(slack);
        }
    }

    std::vector<Node*> getCriticalPath() {
        std::vector<Node*> criticalPath;
        for (Node* node : graph->getNodes()) {
            if (node->getSlack() == 0) {
                criticalPath.push_back(node);
            }
        }
        return criticalPath;
    }

    void solve() {
        forwardPass();
        backwardPass();
        calculateSlack();
    }
};