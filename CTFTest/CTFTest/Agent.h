#pragma once

#include "Pathfinder.h"
#include "FlagManager.h"
#include <QGraphicsEllipseItem>
#include <QColor>
#include <QPointF>
#include <vector>
#include <utility>
#include <memory>

class Brain;
class GameManager;

class Agent : public QGraphicsEllipseItem {
public:
    Agent(const QColor& color, const QPointF& flagPos, const QPointF& basePos, int sceneWidth, int sceneHeight, GameManager* gameManager);

    void update(const std::vector<std::pair<int, int>>& otherAgentsPositions, std::vector<Agent*>& otherAgents, int elapsedTime);

    float calculateDistance(const QPointF& pos1, const QPointF& pos2) const;
    float distanceToNearestEnemy(const std::vector<std::pair<int, int>>& otherAgentsPositions) const;
    float proximityThreshold;
    float tagProximityThreshold;
    void moveTowardsFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void moveTowardsBase(const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void hideFlag();
    void showFlagAtStartingPosition();
    void avoidEnemy(std::vector<Agent*>& otherAgents, const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void defendFlag(std::vector<Agent*>& otherAgents, const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void setIsCarryingFlag(bool value) { isCarryingFlag = value; }
    void setFlagPosition(const QPointF& position);
    void setBasePosition(const QPointF& position);
    void exploreField(const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void updatePath(const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void chaseOpponentWithFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void tagEnemy(std::vector<Agent*>& otherAgents, const std::vector<std::pair<int, int>>& otherAgentsPositions);
    void pickUpFlag(FlagManager* flag) { carriedFlag = flag; }
    void dropFlag() { carriedFlag = nullptr; }
    void incrementScore();
    bool canTagEnemy(Agent* enemy) const;
    bool isPathEmpty() const;
    bool checkInTeamZone(const QPointF& blueFlagPos, const QPointF& redFlagPos) const;
    bool isOpponentCarryingFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions) const;
    bool getIsCarryingFlag() const;

private:
    QPointF flagPos;
    QPointF blueFlagPos;
    QPointF redFlagPos;
    QPointF basePos;
    QPointF blueBasePos;
    QPointF redBasePos;
    QPointF currentTarget;
    QPointF flagPosition;
    QPointF basePosition;
    QColor agentColor;
    qreal movementSpeed;
    qint64 lastTagTime;
    const qint64 tagCooldownPeriod = 30000; // 30 seconds
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
    FlagManager* carriedFlag = nullptr;
    GameManager* gameManager;
};