#pragma once

#include "Pathfinder.h"
#include <QGraphicsEllipseItem>
#include <QColor>
#include <QPointF>
#include <vector>
#include <utility>
#include <memory>

class Brain;

class Agent : public QGraphicsEllipseItem {
public:
    Agent(QColor color, QPointF flagPos, QPointF basePos, QGraphicsScene* scene);

    void update(const std::vector<std::pair<int, int>>& otherAgentsPositions, std::vector<Agent*>& otherAgents, int elapsedTime);

    float calculateDistance(const QPointF& pos1, const QPointF& pos2) const;
    void moveTowardsFlag();
    void moveTowardsBase();
    void setIsCarryingFlag(bool value) { isCarryingFlag = value; }
    void exploreField();
    bool isOpponentCarryingFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions) const;
    void chaseOpponentWithFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions);
    bool canTagEnemy(Agent* enemy) const;
    float distanceToNearestEnemy(const std::vector<std::pair<int, int>>& otherAgentsPositions) const;
    void tagEnemy(std::vector<Agent*>& otherAgents);
    bool isPathEmpty() const;
    bool checkInTeamZone(const QPointF& blueFlagPos, const QPointF& redFlagPos) const;

private:
    QPointF flagPos;
    QPointF blueFlagPos;
    QPointF redFlagPos;
    QPointF basePos;
    QPointF blueBasePos;
    QPointF redBasePos;
    QPointF currentTarget;
    qreal movementSpeed;
    bool isTagged;
    bool isTagging;
    bool isCarryingFlag;
    int currentPathIndex;
    int gameFieldWidth;
    int gameFieldHeight;
    std::unique_ptr<Pathfinder> pathfinder;
    std::unique_ptr<Brain> brain;
    std::vector<std::pair<int, int>> path;
    std::string side;
};