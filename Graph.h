#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "Node.h"

class Graph {
private:
    std::vector<Node*> nodes;

public:
    Graph() {}

    void addStage(std::string name, int duration) {
        Node* node = new Node(name, duration);
        nodes.push_back(node);
    }

    void addDependency(std::string nodeName, std::string dependencyName) {
        Node* node = getNode(nodeName);
        Node* dependency = getNode(dependencyName);
        node->addDirectDependency(dependency);
    }

    Node* getNode(std::string name) {
        for (Node* node : nodes) {
            if (node->getName() == name) {
                return node;
            }
        }
        return nullptr;
    }

    std::vector<Node*> getNodes() const {
        return nodes;
    }

    // Renvoie, pour un node donné, la liste des nodes qui dépendent DIRECTEMENT de lui
    // (l'inverse de getDirectDependencies)
    std::vector<Node*> getDependents(Node* target) const {
        std::vector<Node*> dependents;
        for (Node* node : nodes) {
            for (Node* dep : node->getDirectDependencies()) {
                if (dep == target) {
                    dependents.push_back(node);
                    break;
                }
            }
        }
        return dependents;
    }

    // Tri topologique (algorithme de Kahn) : garantit qu'un node
    // n'apparaît qu'après toutes ses dépendances
    std::vector<Node*> getTopologicalOrder() const {
        std::unordered_map<Node*, int> inDegree;
        for (Node* node : nodes) {
            inDegree[node] = node->getDirectDependencies().size();
        }

        std::vector<Node*> queue;
        for (Node* node : nodes) {
            if (inDegree[node] == 0) {
                queue.push_back(node);
            }
        }

        std::vector<Node*> order;
        while (!queue.empty()) {
            Node* current = queue.back();
            queue.pop_back();
            order.push_back(current);

            for (Node* node : nodes) {
                for (Node* dep : node->getDirectDependencies()) {
                    if (dep == current) {
                        inDegree[node]--;
                        if (inDegree[node] == 0) {
                            queue.push_back(node);
                        }
                    }
                }
            }
        }

        if (order.size() != nodes.size()) {
            throw std::runtime_error("Cycle détecté dans le graphe : ordre topologique impossible.");
        }
        return order;
    }
};