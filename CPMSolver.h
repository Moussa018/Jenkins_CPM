#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include "Graph.h"
#include "Node.h"

class CPMSolver {
private:
    Graph* graph;
    std::vector<Node*> order;   // ordre topologique, calculé une seule fois
    double projectDuration = 0.0;

    static constexpr double EPS = 1e-9;

public:
    explicit CPMSolver(Graph* graph) : graph(graph) {}

    void forwardPass() {
        projectDuration = 0.0;
        for (Node* node : order) {
            double earliest = 0.0;
            for (Node* dependency : node->getDirectDependencies()) {
                earliest = std::max(earliest,
                                    dependency->getEarliestStartTime() + dependency->getDuration());
            }
            node->setEarliestStartTime(earliest);
            projectDuration = std::max(projectDuration, earliest + node->getDuration());
        }
    }

    void backwardPass() {
        // Parcours dans l'ordre INVERSE de l'ordre topologique
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            Node* node = *it;
            const std::vector<Node*>& dependents = node->getDependents();

            if (dependents.empty()) {
                // Un stage terminal peut finir au plus tard à la fin du projet,
                // pas à son propre EST (sinon les branches courtes ont un slack nul).
                node->setLatestStartTime(projectDuration - node->getDuration());
            } else {
                double latest = std::numeric_limits<double>::max();
                for (Node* dependent : dependents) {
                    latest = std::min(latest, dependent->getLatestStartTime() - node->getDuration());
                }
                node->setLatestStartTime(latest);
            }
        }
    }

    void calculateSlack() {
        for (Node* node : graph->getNodes()) {
            node->setSlack(node->getLatestStartTime() - node->getEarliestStartTime());
        }
    }

    double getProjectDuration() const { return projectDuration; }

    // Les nœuds critiques renvoyés dans l'ordre topologique : c'est un vrai chemin
    std::vector<Node*> getCriticalPath() const {
        std::vector<Node*> criticalPath;
        for (Node* node : order) {
            if (std::fabs(node->getSlack()) < EPS) criticalPath.push_back(node);
        }
        return criticalPath;
    }

    void solve() {
        order = graph->getTopologicalOrder();
        forwardPass();
        backwardPass();
        calculateSlack();
    }
};
