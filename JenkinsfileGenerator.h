#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>
#include "Graph.h"
#include "Node.h"


class JenkinsfileGenerator {
private:
    Graph* graph;
    std::vector<std::vector<Node*>> levels;

    static std::string escape(const std::string& text) {
        std::string out;
        out.reserve(text.size());
        for (char c : text) {
            if (c == '\' || c == '\'') out.push_back('\');
            out.push_back(c);
        }
        return out;
    }
'
    static std::string indent(int depth) { return std::string(depth * 4, ' '); }

''
    void writeStage(std::ostringstream& out, Node* node, int depth) const {
        out << indent(depth) << "stage('" << escape(node->getName()) << "') {\n"
            << indent(depth + 1) << "steps {\n"
            << indent(depth + 2) << "// TODO: remplacer par les steps réels de ce stage\n"
            << indent(depth + 2) << "echo 'Stage " << escape(node->getName()) << "'\n"
            << indent(depth + 1) << "}\n"
            << indent(depth) << "}\n";
    }

public:
    explicit JenkinsfileGenerator(Graph* graph) : graph(graph) {
        if (!graph) throw std::runtime_error("JenkinsfileGenerator : graphe nul.");
    }

    void computeLevels() {
        levels.clear();
        std::unordered_map<Node*, std::size_t> levelOf;

        // L'ordre topologique garantit que les dépendances sont traitées avant le stage courant.
        for (Node* node : graph->getTopologicalOrder()) {
            std::size_t current = 0;
            for (Node* dependency : node->getDirectDependencies()) {
                current = std::max(current, levelOf.at(dependency) + 1);
            }
            levelOf[node] = current;
            if (levels.size() <= current) levels.resize(current + 1);
            levels[current].push_back(node);
        }
    }

    const std::vector<std::vector<Node*>>& getLevels() const { return levels; }

    double getSequentialDuration() const {
        double total = 0.0;
        for (Node* node : graph->getNodes()) total += node->getDuration();
        return total;
    }

    double getParallelDuration() const {
        double total = 0.0;
        for (const auto& level : levels) {
            double slowest = 0.0;
            for (Node* node : level) slowest = std::max(slowest, node->getDuration());
            total += slowest;
        }
        return total;
    }

    std::string generate(const std::string& agent = "any") const {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2);

        out << "// Jenkinsfile généré à partir de l'analyse CPM du pipeline.\n"
            << "// Sequentiel : " << getSequentialDuration()
            << "  ->  Parallelise : " << getParallelDuration() << "\n\n";

        out << "pipeline {\n"
            << indent(1) << "agent " << agent << "\n\n"
            << indent(1) << "stages {\n";

        for (std::size_t i = 0; i < levels.size(); ++i) {
            const std::vector<Node*>& level = levels[i];

            if (level.size() == 1) {
                writeStage(out, level.front(), 2);
            } else {
                out << indent(2) << "stage('Etape " << (i + 1) << "') {\n"
                    << indent(3) << "parallel {\n";
                for (Node* node : level) writeStage(out, node, 4);
                out << indent(3) << "}\n"
                    << indent(2) << "}\n";
            }
        }

        out << indent(1) << "}\n"
            << "}\n";

        return out.str();
    }

    void writeToFile(const std::string& path, const std::string& agent = "any") const {
        std::ofstream file(path);
        if (!file) throw std::runtime_error("Impossible d'écrire le fichier : " + path);
        file << generate(agent);
    }
};
