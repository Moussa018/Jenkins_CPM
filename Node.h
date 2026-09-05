#pragma once
#include <string>
#include <vector>

class Node {
private:
    std::string name;
    double duration;
    std::vector<Node*> directDependencies;  // prédécesseurs
    std::vector<Node*> dependents;          // successeurs (maintenu par Graph)
    double earliestStartTime;
    double latestStartTime;
    double slack;

public:
    Node(std::string name, double duration)
        : name(std::move(name)), duration(duration),
          earliestStartTime(0.0), latestStartTime(0.0), slack(0.0) {}

    const std::string& getName() const { return name; }
    double getDuration() const { return duration; }
    const std::vector<Node*>& getDirectDependencies() const { return directDependencies; }
    const std::vector<Node*>& getDependents() const { return dependents; }
    double getEarliestStartTime() const { return earliestStartTime; }
    double getLatestStartTime() const { return latestStartTime; }
    double getSlack() const { return slack; }

    void addDirectDependency(Node* dependency) { directDependencies.push_back(dependency); }
    void addDependent(Node* dependent) { dependents.push_back(dependent); }
    void setEarliestStartTime(double time) { earliestStartTime = time; }
    void setLatestStartTime(double time) { latestStartTime = time; }
    void setSlack(double s) { slack = s; }
};
