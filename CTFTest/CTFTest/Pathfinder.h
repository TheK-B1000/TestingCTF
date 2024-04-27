#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <utility>
#include <unordered_map>

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

class Pathfinder {
public:
    Pathfinder(int gameFieldWidth, int gameFieldHeight);
    std::vector<std::pair<int, int>> findPath(int startX, int startY, int goalX, int goalY);
    std::pair<int, int> getRandomFreePosition();
    bool isObstacle(int x, int y);

private:
    int gameFieldWidth;
    int gameFieldHeight;
    std::vector<std::pair<int, int>> dynamicObstacles;

    double calculateHeuristic(int x1, int y1, int x2, int y2);
    std::vector<std::pair<int, int>> getNeighbors(int x, int y);
    bool isValidPosition(int x, int y);
};

#endif 