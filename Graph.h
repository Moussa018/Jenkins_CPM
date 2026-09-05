#pragma once
#include <vector>
#include <string>
#include <memory>
#include <deque>
#include <unordered_map>
#include <stdexcept>
#include "Node.h"

class Graph {
private:
    // owners détient la mémoire ; nodes expose des pointeurs bruts.
    // unique_ptr => copie interdite, déplacement automatique, pas de fuite.
    std::vector<std::unique_ptr<Node>> owners;
    std::vector<Node*> nodes;
    std::unordered_map<std::string, Node*> byName;

public:
    Graph() = default;

    void addStage(const std::string& name, double duration) {
        if (byName.count(name)) {
            throw std::runtime_error("Stage dupliqué : " + name);
        }
        owners.push_back(std::make_unique<Node>(name, duration));
        Node* node = owners.back().get();
        nodes.push_back(node);
        byName[name] = node;
    }

    void addDependency(const std::string& nodeName, const std::string& dependencyName) {
        Node* node = requireNode(nodeName);
        Node* dependency = requireNode(dependencyName);
        node->addDirectDependency(dependency);
        dependency->addDependent(node);
    }

    Node* getNode(const std::string& name) const {
        auto it = byName.find(name);
        return it == byName.end() ? nullptr : it->second;
    }

    // Échoue explicitement au lieu de renvoyer nullptr silencieusement
    Node* requireNode(const std::string& name) const {
        Node* node = getNode(name);
        if (!node) throw std::runtime_error("Stage inconnu : " + name);
        return node;
    }

    const std::vector<Node*>& getNodes() const { return nodes; }

    const std::vector<Node*>& getDependents(Node* target) const {
        return target->getDependents();
    }

    // Tri topologique (Kahn) en O(V+E) grâce à la liste de successeurs
    std::vector<Node*> getTopologicalOrder() const {
        std::unordered_map<Node*, std::size_t> inDegree;
        inDegree.reserve(nodes.size());
        for (Node* node : nodes) {
            inDegree[node] = node->getDirectDependencies().size();
        }

        std::deque<Node*> queue;
        for (Node* node : nodes) {
            if (inDegree[node] == 0) queue.push_back(node);
        }

        std::vector<Node*> order;
        order.reserve(nodes.size());
        while (!queue.empty()) {
            Node* current = queue.front();
            queue.pop_front();
            order.push_back(current);

            for (Node* dependent : current->getDependents()) {
                if (--inDegree[dependent] == 0) queue.push_back(dependent);
            }
        }

        if (order.size() != nodes.size()) {
            throw std::runtime_error("Cycle détecté dans le graphe : ordre topologique impossible.");
        }
        return order;
    }
};
