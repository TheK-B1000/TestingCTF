#include "Pathfinder.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

Pathfinder::Pathfinder(int gameFieldWidth, int gameFieldHeight)
    : gameFieldWidth(gameFieldWidth), gameFieldHeight(gameFieldHeight) {
}

double Pathfinder::calculateHeuristic(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);
    return dx + dy;
}

std::vector<std::pair<int, int>> Pathfinder::findPath(int startX, int startY, int goalX, int goalY) {
    std::priority_queue<std::pair<double, std::pair<int, int>>,
        std::vector<std::pair<double, std::pair<int, int>>>,
        std::greater<std::pair<double, std::pair<int, int>>>> openSet;
    std::unordered_set<std::pair<int, int>, pair_hash> openSetElements;
    std::unordered_set<std::pair<int, int>, pair_hash> closedSet;
    std::unordered_map<std::pair<int, int>, std::pair<int, int>, pair_hash> cameFrom;

    openSet.push({ 0.0, {startX, startY} });
    openSetElements.insert({ startX, startY });

    while (!openSet.empty()) {
        std::pair<int, int> current = openSet.top().second;
        openSet.pop();
        openSetElements.erase(current);

        if (current.first == goalX && current.second == goalY) {
            std::vector<std::pair<int, int>> path;
            while (current != std::make_pair(startX, startY)) {
                path.push_back(current);
                current = cameFrom[current];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        closedSet.insert(current);

        for (const auto& neighbor : getNeighbors(current.first, current.second)) {
            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            double tentativeGScore = calculateHeuristic(startX, startY, neighbor.first, neighbor.second);
            double fScore = tentativeGScore + calculateHeuristic(neighbor.first, neighbor.second, goalX, goalY);

            if (openSetElements.find(neighbor) == openSetElements.end()) {
                openSet.push({ fScore, neighbor });
                openSetElements.insert(neighbor);
                cameFrom[neighbor] = current;
            }
        }
    }

    return std::vector<std::pair<int, int>>();
}

std::vector<std::pair<int, int>> Pathfinder::getNeighbors(int x, int y) {
    std::vector<std::pair<int, int>> neighbors;
    const int dx[] = { -1, 0, 1, 0 };
    const int dy[] = { 0, -1, 0, 1 };

    for (int i = 0; i < 4; ++i) {
        int newX = x + dx[i];
        int newY = y + dy[i];
        if (isValidPosition(newX, newY)) {
            neighbors.push_back({ newX, newY });
        }
    }
    return neighbors;
}

bool Pathfinder::isValidPosition(int x, int y) {
    return x >= 0 && x < gameFieldWidth && y >= 0 && y < gameFieldHeight;
}