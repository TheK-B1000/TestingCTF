#include "Agent.h"
#include "Pathfinder.h"
#include <QtMath>
#include <QBrush>
#include <QGraphicsScene>
#include <Brain.h>
#include <QRandomGenerator>

Agent::Agent(QColor color, QPointF flagPos, QPointF basePos, QGraphicsScene* scene)
    : QGraphicsEllipseItem(0, 0, 20, 20, nullptr),
    blueFlagPos(blueFlagPos),
    redFlagPos(redFlagPos),
    basePos(basePos),
    isCarryingFlag(false),
    currentPathIndex(0),
    pathfinder(std::make_unique<Pathfinder>(scene->sceneRect().width(), scene->sceneRect().height())),
    currentTarget(0, 0),
    brain(std::make_unique<Brain>()),
    gameFieldWidth(scene->sceneRect().width()),
    gameFieldHeight(scene->sceneRect().height()),
    movementSpeed(10.0f),
    isTagged(false),
    isTagging(false)
{
    if (color == Qt::blue) {
        blueBasePos = basePos;
        redFlagPos = flagPos;
    }
    else {
        redBasePos = basePos;
        blueFlagPos = flagPos;
    }

    setRect(0, 0, 20, 20);
    setBrush(color);

    side = (color == Qt::blue) ? "blue" : "red";
}

void Agent::update(const std::vector<std::pair<int, int>>& otherAgentsPositions, std::vector<Agent*>& otherAgents, int elapsedTime) {
    QPointF agentPos(pos().x(), pos().y());

    float distanceToFlag = calculateDistance(agentPos, flagPos);
    float distanceToEnemy = distanceToNearestEnemy(otherAgentsPositions);
    bool enemyHasFlag = isOpponentCarryingFlag(otherAgentsPositions);

    BrainDecision decision = brain->makeDecision(isCarryingFlag, checkInTeamZone(this->blueFlagPos, this->redFlagPos), distanceToFlag, isTagged, enemyHasFlag, distanceToEnemy, isTagging);

    switch (decision) {
    case BrainDecision::Explore:
        qDebug() << "Exploring field";
        exploreField();
        break;
    case BrainDecision::GrabFlag:
        qDebug() << "Moving towards enemy flag";
        moveTowardsFlag();
        break;
    case BrainDecision::CaptureFlag:
        qDebug() << "Moving towards home zone";
        moveTowardsBase();
        break;
    case BrainDecision::RecoverFlag:
        qDebug() << "Chasing opponent with flag";
        chaseOpponentWithFlag(otherAgentsPositions);
        break;
    case BrainDecision::TagEnemy:
        qDebug() << "Tagging enemy";
        tagEnemy(otherAgents);
        break;
    case BrainDecision::ReturnToHomeZone:
        qDebug() << "Returning to home zone";
        moveTowardsBase();
        break;
    default:
        qDebug() << "Exploring field";
        exploreField();
        break;
    }
}

float Agent::calculateDistance(const QPointF& pos1, const QPointF& pos2) const {
    QPointF diff = pos1 - pos2;
    return std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
}


void Agent::moveTowardsFlag() {
    qreal speed = movementSpeed;

    QPointF targetFlagPos = (side == "blue") ? redFlagPos : blueFlagPos;

    if (path.empty()) {
        path = pathfinder->findPath(pos().x(), pos().y(), targetFlagPos.x(), targetFlagPos.y());
        currentPathIndex = 0;
    }

    QPointF target;
    if (currentPathIndex < path.size()) {
        target = QPointF(path[currentPathIndex].first, path[currentPathIndex].second);
    }
    else {
        target = targetFlagPos;
    }

    QPointF direction = target - pos();
    qreal distance = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

    if (distance > 0) {
        direction /= distance; // Normalize the direction vector

        // Move the agent towards the target
        setPos(pos() + direction * std::min(distance, speed));

        // Check if the agent has reached the target or a path point
        if (distance <= speed) {
            if (currentPathIndex < path.size() - 1) {
                currentPathIndex++; // Set the next point as the current target for the next update
            }
            else {
                setIsCarryingFlag(true); // The agent reaches the flag
                path.clear(); // Clear path data
                currentPathIndex = 0; // Reset path index
            }
        }
    }
    else {
        // The agent has reached the flag or the next point in the path
        if (currentPathIndex < path.size() - 1) {
            currentPathIndex++; // Set the next point as the current target for the next update
        }
        else {
            setIsCarryingFlag(true); // The agent reaches the flag
            path.clear(); // Clear path data
            currentPathIndex = 0; // Reset path index
        }

        // Move the agent towards the new target
        setPos(pos() + direction * speed);
    }
}

void Agent::moveTowardsBase() {
    qreal speed = movementSpeed;

    QPointF targetBasePos = (side == "blue") ? blueBasePos : redBasePos;

    // Check if a new path needs to be calculated
    if (path.empty()) {
        path = pathfinder->findPath(pos().x(), pos().y(), targetBasePos.x(), targetBasePos.y());
        currentPathIndex = 0;
    }

    // Only proceed if there is a path
    if (!path.empty()) {
        // Set the next waypoint in the path as the target
        QPointF target = QPointF(path[currentPathIndex].first, path[currentPathIndex].second);
        QPointF direction = target - pos();
        qreal distance = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

        // Normalize the direction vector and move the agent towards the target
        if (distance > 0) {
            direction /= distance;
            setPos(pos() + direction * std::min(distance, speed));
        }

        // Check if the agent has reached the current target or the path point
        if (distance <= speed) {
            currentPathIndex++; // Prepare for the next waypoint

            // Check if the agent has reached the end of the path (base position)
            if (currentPathIndex >= path.size()) {
                setIsCarryingFlag(false); // The agent reaches the base and drops the flag
                path.clear(); // Clear the path
                currentPathIndex = 0; // Reset the path index
            }
        }
    }
    else {
        // Edge case: If the distance is zero (agent is already at the base), drop the flag
        setIsCarryingFlag(false);
    }
}


void Agent::exploreField() {
    qreal speed = movementSpeed;

    // Define a target position for exploration
    QPointF explorationTarget;

    // Generate a random target position within the game field
    explorationTarget.setX(QRandomGenerator::global()->bounded(0, gameFieldWidth));
    explorationTarget.setY(QRandomGenerator::global()->bounded(0, gameFieldHeight));

    // Check if a new path needs to be calculated
    if (path.empty()) {
        path = pathfinder->findPath(pos().x(), pos().y(), explorationTarget.x(), explorationTarget.y());
        currentPathIndex = 0;
    }

    // Only proceed if there is a path
    if (!path.empty()) {
        // Set the next waypoint in the path as the target
        QPointF target = QPointF(path[currentPathIndex].first, path[currentPathIndex].second);
        QPointF direction = target - pos();
        qreal distance = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

        // Normalize the direction vector and move the agent towards the target
        if (distance > 0) {
            direction /= distance;
            setPos(pos() + direction * std::min(distance, speed));
        }

        // Check if the agent has reached the current target or the path point
        if (distance <= speed) {
            currentPathIndex++; // Prepare for the next waypoint

            // Check if the agent has reached the end of the path (exploration target)
            if (currentPathIndex >= path.size()) {
                // Reached the exploration target, generate a new random target
                path.clear();
                currentPathIndex = 0;
                exploreField();
            }
        }
    }
    else {
        // Edge case: If the distance is zero (agent has reached the exploration target), generate a new target
        path.clear();
        currentPathIndex = 0;
        exploreField();
    }
}

void Agent::tagEnemy(std::vector<Agent*>& otherAgents) {
    qreal speed = movementSpeed;
    Agent* closestEnemy = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    // Find the closest enemy that can be tagged
    for (Agent* enemy : otherAgents) {
        if (enemy->side != side && !enemy->isTagged && canTagEnemy(enemy)) {
            float distance = calculateDistance(pos(), enemy->pos());
            if (distance < minDistance) {
                minDistance = distance;
                closestEnemy = enemy;
            }
        }
    }

    // If a valid enemy is found, move towards and tag them
    if (closestEnemy != nullptr) {
        isTagging = true; // Set isTagging to true when starting to tag an enemy

        if (path.empty()) {
            path = pathfinder->findPath(pos().x(), pos().y(), closestEnemy->pos().x(), closestEnemy->pos().y());
            currentPathIndex = 0;
        }

        QPointF target;
        if (currentPathIndex < path.size()) {
            target = QPointF(path[currentPathIndex].first, path[currentPathIndex].second);
        }
        else {
            target = closestEnemy->pos();
        }

        QPointF direction = target - pos();
        qreal distance = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

        if (distance > 0) {
            direction /= distance; // Normalize the direction vector

            // Move the agent towards the target
            setPos(pos() + direction * std::min(distance, speed));

            // Check if the agent has reached the target or a path point
            if (distance <= speed) {
                if (currentPathIndex < path.size() - 1) {
                    currentPathIndex++; // Set the next point as the current target for the next update
                }
                else {
                    closestEnemy->isTagged = true; // Tag the enemy
                    isTagging = false; // Set isTagging to false when the tagging behavior is completed
                    path.clear(); // Clear path data
                    currentPathIndex = 0; // Reset path index
                    // Add a cooldown timer if required
                }
            }
        }
        else {
            // The agent has reached the enemy
            closestEnemy->isTagged = true; // Tag the enemy
            isTagging = false; // Set isTagging to false when the tagging behavior is completed
            path.clear(); // Clear path data
            currentPathIndex = 0; // Reset path index
            // Add a cooldown timer if required
        }
    }
    else {
        isTagging = false; // Set isTagging to false when no valid enemy is found
    }
}

void Agent::chaseOpponentWithFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    qreal speed = movementSpeed;
    std::pair<int, int> opponentWithFlagPos;
    bool opponentFound = false;

    // Find the position of the opponent carrying the flag
    for (const auto& pos : otherAgentsPositions) {
        if ((side == "blue" && qFuzzyCompare(static_cast<double>(pos.first), redFlagPos.x()) &&
            qFuzzyCompare(static_cast<double>(pos.second), redFlagPos.y())) ||
            (side == "red" && qFuzzyCompare(static_cast<double>(pos.first), blueFlagPos.x()) &&
                qFuzzyCompare(static_cast<double>(pos.second), blueFlagPos.y()))) {
            opponentWithFlagPos = pos;
            opponentFound = true;
            break;
        }
    }

    // If an opponent with the flag is found, move towards them
    if (opponentFound) {
        if (path.empty()) {
            path = pathfinder->findPath(pos().x(), pos().y(), opponentWithFlagPos.first, opponentWithFlagPos.second);
            currentPathIndex = 0;
        }

        QPointF target;
        if (currentPathIndex < path.size()) {
            target = QPointF(path[currentPathIndex].first, path[currentPathIndex].second);
        }
        else {
            target = QPointF(opponentWithFlagPos.first, opponentWithFlagPos.second);
        }

        QPointF direction = target - pos();
        qreal distance = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

        if (distance > 0) {
            direction /= distance; // Normalize the direction vector

            // Move the agent towards the target
            setPos(pos() + direction * std::min(distance, speed));

            // Check if the agent has reached the target or a path point
            if (distance <= speed) {
                if (currentPathIndex < path.size() - 1) {
                    currentPathIndex++; // Set the next point as the current target for the next update
                }
                else {
                    // Reached the opponent with the flag, add logic for tagging or capturing the flag
                    path.clear(); // Clear path data
                    currentPathIndex = 0; // Reset path index
                }
            }
        }
        else {
            // The agent has reached the opponent with the flag
            path.clear(); // Clear path data
            currentPathIndex = 0; // Reset path index
        }
    }
}

bool Agent::canTagEnemy(Agent* enemy) const {
    if (enemy == nullptr || enemy->side == side || isTagged || enemy->isTagged)
        return false;

    // Check if the enemy is on the opposite side of the field
    QPointF enemyPos(enemy->pos().x(), enemy->pos().y());
    bool isEnemyInOppositeSide = (side == "blue") ? enemyPos.x() > gameFieldWidth / 2 : enemyPos.x() < gameFieldWidth / 2;

    // Check if the tagging agent is in its home side of the field
    QPointF agentPos(pos().x(), pos().y());
    bool isAgentInHomeSide = (side == "blue") ? agentPos.x() < gameFieldWidth / 2 : agentPos.x() > gameFieldWidth / 2;

    // Check if the agent is within 10 meters of the enemy
    float distance = calculateDistance(agentPos, enemyPos);
    const float tagRange = 10.0f; // Adjust this value as needed

    return isEnemyInOppositeSide && isAgentInHomeSide && distance <= tagRange;
}

float Agent::distanceToNearestEnemy(const std::vector<std::pair<int, int>>& otherAgentsPositions) const {
    float minDistance = std::numeric_limits<float>::max();
    QPointF agentPos(pos().x(), pos().y());

    for (const auto& pos : otherAgentsPositions) {
        if (side != "blue" && pos.first < gameFieldWidth / 2) {
            // Enemy position is on the blue side
            float distance = calculateDistance(agentPos, QPointF(pos.first, pos.second));
            if (distance < minDistance) {
                minDistance = distance;
            }
        }
        else if (side != "red" && pos.first >= gameFieldWidth / 2) {
            // Enemy position is on the red side
            float distance = calculateDistance(agentPos, QPointF(pos.first, pos.second));
            if (distance < minDistance) {
                minDistance = distance;
            }
        }
    }

    return minDistance;
}

bool Agent::isPathEmpty() const {
    return path.empty();
}

bool Agent::checkInTeamZone(const QPointF& blueFlagPos, const QPointF& redFlagPos) const {
    QPointF agentPos(pos().x(), pos().y());
    float zoneRadius = 40.0f; // Radius of the team zones (adjust as needed)

    if (side == "blue") {
        // Blue team's zone is a circular area around the blue flag
        float distanceToBlueZone = calculateDistance(agentPos, blueFlagPos);
        return distanceToBlueZone <= zoneRadius;
    }
    else {
        // Red team's zone is a circular area around the red flag
        float distanceToRedZone = calculateDistance(agentPos, redFlagPos);
        return distanceToRedZone <= zoneRadius;
    }
}

bool Agent::isOpponentCarryingFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions) const {
    // Check if an opponent is carrying the flag based on the agent's side
    for (const auto& pos : otherAgentsPositions) {
        if (side == "blue" && qFuzzyCompare(static_cast<double>(pos.first), redFlagPos.x()) &&
            qFuzzyCompare(static_cast<double>(pos.second), redFlagPos.y())) {
            return true;
        }
        else if (side == "red" && qFuzzyCompare(static_cast<double>(pos.first), blueFlagPos.x()) &&
            qFuzzyCompare(static_cast<double>(pos.second), blueFlagPos.y())) {
            return true;
        }
    }
    return false;
}

