#include "Pathfinder.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

Pathfinder::Pathfinder(int gameFieldWidth, int gameFieldHeight)
    : gameFieldWidth(gameFieldWidth), gameFieldHeight(gameFieldHeight) {}

double Pathfinder::calculateHeuristic(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);
    return dx + dy;
}

std::vector<std::pair<int, int>> Pathfinder::findPath(int startX, int startY, int goalX, int goalY, const std::vector<std::pair<int, int>>& enemyPositions) {
    struct NodeComparator {
        bool operator()(const std::tuple<double, double, std::pair<int, int>>& a,
            const std::tuple<double, double, std::pair<int, int>>& b) {
            double f1 = std::get<0>(a);
            double f2 = std::get<0>(b);
            return f1 > f2;
        }
    };

    std::vector<std::tuple<double, double, std::pair<int, int>>> openSet;
    NodeComparator comparator;
    std::unordered_set<std::pair<int, int>, pair_hash> closedSet;
    std::unordered_map<std::pair<int, int>, std::pair<int, int>, pair_hash> cameFrom;
    std::unordered_map<std::pair<int, int>, double, pair_hash> gScore;

    auto heuristic = [&](int x, int y) {
        return std::abs(x - goalX) + std::abs(y - goalY);
        };

    gScore[{startX, startY}] = 0.0;
    openSet.emplace_back(heuristic(startX, startY), 0.0, std::make_pair(startX, startY));
    std::make_heap(openSet.begin(), openSet.end(), comparator);

    while (!openSet.empty()) {
        std::pop_heap(openSet.begin(), openSet.end(), comparator);
        auto [_, currentGScore, current] = openSet.back();
        openSet.pop_back();

        if (current == std::make_pair(goalX, goalY)) {
            std::vector<std::pair<int, int>> path;
            while (current != std::make_pair(startX, startY)) {
                path.push_back(current);
                current = cameFrom[current];
            }
            path.push_back({ startX, startY });
            std::reverse(path.begin(), path.end());
            return path;
        }

        if (closedSet.count(current) > 0) {
            continue;
        }

        closedSet.insert(current);

        for (const auto& neighbor : getNeighbors(current.first, current.second, enemyPositions)) {
            if (closedSet.count(neighbor) > 0) {
                continue;
            }

            double tentativeGScore = currentGScore + getCost(current, neighbor, enemyPositions);
            if (gScore.count(neighbor) == 0 || tentativeGScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentativeGScore;
                double fScore = tentativeGScore + heuristic(neighbor.first, neighbor.second);
                openSet.emplace_back(fScore, tentativeGScore, neighbor);
                std::push_heap(openSet.begin(), openSet.end(), comparator);
            }
        }
    }

    return std::vector<std::pair<int, int>>();
}

std::vector<std::pair<int, int>> Pathfinder::getNeighbors(int x, int y, const std::vector<std::pair<int, int>>& enemyPositions) {
    std::vector<std::pair<int, int>> neighbors;
    const int dx[] = { -1, 0, 1, 0 };
    const int dy[] = { 0, -1, 0, 1 };

    for (int i = 0; i < 4; ++i) {
        int newX = x + dx[i];
        int newY = y + dy[i];
        if (isValidPosition(newX, newY) && !isEnemyPosition(newX, newY, enemyPositions)) {
            neighbors.push_back({ newX, newY });
        }
    }

    return neighbors;
}

bool Pathfinder::isValidPosition(int x, int y) {
    return x >= 0 && x < gameFieldWidth && y >= 0 && y < gameFieldHeight;
}

bool Pathfinder::isEnemyPosition(int x, int y, const std::vector<std::pair<int, int>>& enemyPositions) {
    return std::find(enemyPositions.begin(), enemyPositions.end(), std::make_pair(x, y)) != enemyPositions.end();
}

double Pathfinder::getCost(const std::pair<int, int>& current, const std::pair<int, int>& neighbor, const std::vector<std::pair<int, int>>& enemyPositions) {
    double baseCost = 1.0;
    double enemyCost = 10.0;

    if (isEnemyPosition(neighbor.first, neighbor.second, enemyPositions)) {
        return baseCost + enemyCost;
    }

    return baseCost;
}