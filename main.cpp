#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include "Node.h"
#include "Graph.h"
#include "CPMSolver.h"
#include "JenkinsfileGenerator.h"
#include "include/json.hpp"

using json = nlohmann::json;

// Charge un pipeline depuis un JSON au format de exemple.json :
// { "stages": [ {"name": ..., "duration": ..., "dependencies": [...]} ] }
Graph parseFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + path);
    }

    json data = json::parse(file);
    if (!data.contains("stages") || !data["stages"].is_array()) {
        throw std::runtime_error("JSON invalide : clé \"stages\" (tableau) attendue.");
    }

    Graph graph;

    // 1re passe : créer tous les stages
    for (const auto& stage : data["stages"]) {
        graph.addStage(stage.at("name").get<std::string>(),
                       stage.at("duration").get<double>());
    }

    // 2e passe : relier les dépendances (elles peuvent référencer un stage défini plus loin)
    for (const auto& stage : data["stages"]) {
        const std::string name = stage.at("name").get<std::string>();
        if (!stage.contains("dependencies")) continue;
        for (const auto& dep : stage["dependencies"]) {
            graph.addDependency(name, dep.get<std::string>());
        }
    }

    return graph;
}

int main(int argc, char* argv[]) {
    const std::string path = (argc > 1) ? argv[1] : "exemple.json";
    // 2e argument optionnel : chemin du Jenkinsfile parallelise a generer
    const std::string jenkinsfilePath = (argc > 2) ? argv[2] : "";

    try {
        Graph graph = parseFile(path);

        CPMSolver solver(&graph);
        solver.solve();

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "=== Resultats CPM (" << path << ") ===\n";
        std::cout << "Stage\t\tDuration\tEST\tLST\tSlack\n";
        for (Node* node : graph.getNodes()) {
            std::cout << node->getName() << "\t\t"
                      << node->getDuration() << "\t\t"
                      << node->getEarliestStartTime() << "\t"
                      << node->getLatestStartTime() << "\t"
                      << node->getSlack() << "\n";
        }

        std::cout << "\nDuree totale du pipeline : " << solver.getProjectDuration() << "\n";

        std::cout << "\n=== Chemin critique ===\n";
        for (Node* node : solver.getCriticalPath()) {
            std::cout << node->getName() << " ";
        }
        std::cout << "\n";

        JenkinsfileGenerator generator(&graph);
        generator.computeLevels();

        std::cout << "\n=== Plan de parallelisation ===\n";
        const auto& levels = generator.getLevels();
        for (std::size_t i = 0; i < levels.size(); ++i) {
            std::cout << "Etape " << (i + 1) << " : ";
            for (Node* node : levels[i]) std::cout << node->getName() << " ";
            std::cout << (levels[i].size() > 1 ? "  (en parallele)" : "") << "\n";
        }

        const double sequential = generator.getSequentialDuration();
        const double parallel = generator.getParallelDuration();
        std::cout << "\nSequentiel : " << sequential
                  << "  ->  Parallelise : " << parallel;
        if (sequential > 0.0) {
            std::cout << "  (-" << (100.0 * (sequential - parallel) / sequential) << " %)";
        }
        std::cout << "\n";

        if (!jenkinsfilePath.empty()) {
            generator.writeToFile(jenkinsfilePath);
            std::cout << "\nJenkinsfile ecrit dans : " << jenkinsfilePath << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << "\n";
        return 1;
    }

    return 0;
}
