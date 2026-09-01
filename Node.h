#pragma once
#include <string>
#include <vector>

class Node {
private:
    std::string name;
    int duration;
    std::vector<Node*> directDependencies;
    int earliestStartTime;
    int latestStartTime;
    int slack;

public:
    Node(std::string name, int duration) : name(name), duration(duration) {
        earliestStartTime = 0;
        latestStartTime = 0;
        slack = 0;
    }

    const std::string& getName() const { return name; }
    int getDuration() const { return duration; }
    const std::vector<Node*>& getDirectDependencies() const { return directDependencies; }
    int getEarliestStartTime() const { return earliestStartTime; }
    int getLatestStartTime() const { return latestStartTime; }
    int getSlack() const { return slack; }

    void addDirectDependency(Node* dependency) {
        directDependencies.push_back(dependency);
    }
    void setEarliestStartTime(int time) { earliestStartTime = time; }
    void setLatestStartTime(int time) { latestStartTime = time; }
    void setSlack(int s) { slack = s; }
};