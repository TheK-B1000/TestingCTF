#include "Agent.h"
#include "GameManager.h"
#include "Pathfinder.h"
#include <QDateTime>
#include <QtMath>
#include <QBrush>
#include <QGraphicsScene>
#include <Brain.h>
#include <QRandomGenerator>

Agent::Agent(const QColor& color, const QPointF& flagPos, const QPointF& basePos, int sceneWidth, int sceneHeight, GameManager* gameManager)
    : QGraphicsEllipseItem(0, 0, 20, 20, nullptr),
    blueFlagPos(blueFlagPos),
    redFlagPos(redFlagPos),
    basePos(basePos),
    isCarryingFlag(false),
    currentPathIndex(0),
    pathfinder(std::make_unique<Pathfinder>(sceneWidth, sceneHeight)),
    currentTarget(0, 0),
    brain(std::make_unique<Brain>()),
    gameFieldWidth(sceneWidth),
    gameFieldHeight(sceneHeight),
    movementSpeed(2000.0f),
    isTagged(false),
    isTagging(false),
    agentColor(color),
    gameManager(gameManager),
    tagProximityThreshold(200.0f),
    proximityThreshold(250.0f)
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

    // Check if the agent is in the middle of the field
    bool isInMiddle = isInMiddleOfField();
    if (isInMiddle) {
        middleStuckTime += elapsedTime;
    }
    else {
        middleStuckTime = 0;
    }
    bool isStuckInMiddle = middleStuckTime > 5000; // Adjust the threshold value as needed

    BrainDecision decision = brain->makeDecision(isCarryingFlag, checkInTeamZone(this->blueFlagPos, this->redFlagPos), distanceToFlag, isTagged, enemyHasFlag, distanceToEnemy, isTagging, isStuckInMiddle);

    if (isTagged) {
        // Change the agent's color to pink
        agentColor = Qt::magenta;
        setPen(QPen(agentColor, 2));
    }
    else if (isCarryingFlag) {
        // Change the agent's color to gold
        agentColor = Qt::yellow;
        setPen(QPen(agentColor, 2));
    }
    else {
        // Restore the agent's original color
        agentColor = (side == "blue") ? Qt::blue : Qt::red;
        setPen(QPen(agentColor, 2));
    }

    // Prioritize grabbing the flag if the agent is close to it or there are no enemies nearby
    if (!isCarryingFlag && !isTagged && (distanceToFlag <= 250.0f || distanceToEnemy > 100.0f)) {
        decision = BrainDecision::GrabFlag;
    }

    switch (decision) {
    case BrainDecision::Explore:
        qDebug() << "Exploring field";
        exploreField(otherAgentsPositions);
        break;
    case BrainDecision::GrabFlag:
        qDebug() << "Moving towards enemy flag";
        moveTowardsFlag(otherAgentsPositions);
        break;
    case BrainDecision::CaptureFlag:
        qDebug() << "Moving towards home zone";
        moveTowardsBase(otherAgentsPositions);
        break;
    case BrainDecision::AvoidEnemy:
        qDebug() << "Avoiding enemy";
        avoidEnemy(otherAgents, otherAgentsPositions);
        break;
    case BrainDecision::RecoverFlag:
        qDebug() << "Chasing opponent with flag";
        chaseOpponentWithFlag(otherAgentsPositions);
        break;
    case BrainDecision::DefendFlag:
        qDebug() << "Defending flag";
        defendFlag(otherAgents, otherAgentsPositions);
        break;
    case BrainDecision::TagEnemy:
        qDebug() << "Tagging enemy";
        tagEnemy(otherAgents, otherAgentsPositions);
        break;
    case BrainDecision::ReturnToHomeZone:
        qDebug() << "Returning to home zone";
        moveTowardsBase(otherAgentsPositions);
        break;
    default:
        qDebug() << "Exploring field";
        exploreField(otherAgentsPositions);
        break;
    }
}

float Agent::calculateDistance(const QPointF& pos1, const QPointF& pos2) const {
    QPointF diff = pos1 - pos2;
    return std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
}


void Agent::moveTowardsFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    qreal speed = movementSpeed;

    QPointF targetFlagPos = (side == "blue") ? redFlagPos : blueFlagPos;

    if (path.empty()) {
        std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
        path = pathfinder->findPath(pos().x(), pos().y(), targetFlagPos.x(), targetFlagPos.y(), otherAgentsPositions, agentPositions);
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
                // Check if the agent is within the specified zone around the enemy flag
                float distanceToEnemyFlag = calculateDistance(pos(), targetFlagPos);
                const float flagZoneRadius = 10.0f; // Specify the radius of the flag zone (e.g., 10 meters)

                if (distanceToEnemyFlag <= flagZoneRadius) {
                    setIsCarryingFlag(true); // The agent reaches the flag and captures it
                    hideFlag(); // Hide the flag when the agent picks it up
                    setPen(QPen(Qt::yellow, 2)); // Set the agent's outline color to gold
                }
                else {
                    // The agent is not within the flag zone, continue moving towards the flag
                    path.clear();
                    currentPathIndex = 0;
                    std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                    path = pathfinder->findPath(pos().x(), pos().y(), targetFlagPos.x(), targetFlagPos.y(), otherAgentsPositions, agentPositions);
                }

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
            // Check if the agent is within the specified zone around the enemy flag
            float distanceToEnemyFlag = calculateDistance(pos(), targetFlagPos);
            const float flagZoneRadius = 10.0f; // Specify the radius of the flag zone (e.g., 10 meters)

            if (distanceToEnemyFlag <= flagZoneRadius) {
                setIsCarryingFlag(true); // The agent reaches the flag and captures it
                hideFlag(); // Hide the flag when the agent picks it up
            }
            else {
                // The agent is not within the flag zone, continue moving towards the flag
                path.clear();
                currentPathIndex = 0;
                std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                path = pathfinder->findPath(pos().x(), pos().y(), targetFlagPos.x(), targetFlagPos.y(), otherAgentsPositions, agentPositions);
            }
        }

        // Move the agent towards the new target
        setPos(pos() + direction * speed);
    }
}

void Agent::moveTowardsBase(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    qreal speed = movementSpeed;

    QPointF targetBasePos = (side == "blue") ? blueBasePos : redBasePos;

    // Check if a new path needs to be calculated
    if (path.empty()) {
        std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
        path = pathfinder->findPath(pos().x(), pos().y(), targetBasePos.x(), targetBasePos.y(), otherAgentsPositions, agentPositions);
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
                showFlagAtStartingPosition(); // Show the flag at its starting position
                incrementScore(); // Increment the score for the agent's team
                setPen(QPen(agentColor, 2)); // Restore the agent's original outline color
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

void Agent::exploreField(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    qreal speed = movementSpeed;

    // Define a target position for exploration
    QPointF explorationTarget;

    // Generate a random target position within the game field
    explorationTarget.setX(QRandomGenerator::global()->bounded(0, gameFieldWidth));
    explorationTarget.setY(QRandomGenerator::global()->bounded(0, gameFieldHeight));

    // Check if a new path needs to be calculated
    if (path.empty()) {
        std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
        path = pathfinder->findPath(pos().x(), pos().y(), explorationTarget.x(), explorationTarget.y(), otherAgentsPositions, agentPositions);
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
                // Check if the agent is close to the flag or if there are no enemies nearby
                float distanceToFlag = calculateDistance(pos(), flagPos);
                float distanceToEnemy = distanceToNearestEnemy(otherAgentsPositions);

                if (distanceToFlag <= proximityThreshold || distanceToEnemy > tagProximityThreshold) {
                    // Cancel exploration and move towards the flag
                    path.clear();
                    currentPathIndex = 0;
                    moveTowardsFlag(otherAgentsPositions);
                    return;
                }

                // Reached the exploration target, generate a new random target
                path.clear();
                currentPathIndex = 0;
                exploreField(otherAgentsPositions);
            }
        }
    }
}

std::vector<std::pair<int, int>> Agent::getOtherAgentPositions(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    std::vector<std::pair<int, int>> agentPositions;

    for (const auto& pos : otherAgentsPositions) {
        if (pos != std::make_pair(static_cast<int>(this->pos().x()), static_cast<int>(this->pos().y()))) {
            agentPositions.push_back(pos);
        }
    }

    return agentPositions;
}

void Agent::updatePath(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    QPointF targetPos;
    if (isCarryingFlag) {
        // If carrying the flag, set the target position to the base
        targetPos = (side == "blue") ? blueBasePos : redBasePos;
    }
    else {
        // If not carrying the flag, set the target position to the enemy flag
        targetPos = (side == "blue") ? redFlagPos : blueFlagPos;
    }

    // Check if a new path needs to be calculated
    if (path.empty()) {
        std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
        path = pathfinder->findPath(pos().x(), pos().y(), targetPos.x(), targetPos.y(), otherAgentsPositions, agentPositions);
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
            setPos(pos() + direction * std::min(distance, movementSpeed));
        }

        // Check if the agent has reached the current target or the path point
        if (distance <= movementSpeed) {
            currentPathIndex++; // Prepare for the next waypoint

            // Check if the agent has reached the end of the path (target position)
            if (currentPathIndex >= path.size()) {
                // Reached the target position
                if (isCarryingFlag) {
                    // If carrying the flag, drop it at the base
                    setIsCarryingFlag(false);
                    showFlagAtStartingPosition();
                    incrementScore();
                }
                else {
                    // If not carrying the flag, grab the enemy flag
                    setIsCarryingFlag(true);
                    hideFlag();
                }

                // Clear the path and reset the path index
                path.clear();
                currentPathIndex = 0;
            }
        }
    }
    else {
        // Edge case: If there is no path, the agent has reached the target position
        if (isCarryingFlag) {
            // If carrying the flag, drop it at the base
            setIsCarryingFlag(false);
            showFlagAtStartingPosition();
            incrementScore();
        }
        else {
            // If not carrying the flag, grab the enemy flag
            setIsCarryingFlag(true);
            hideFlag();
        }
    }
}

void Agent::tagEnemy(std::vector<Agent*>& otherAgents, const std::vector<std::pair<int, int>>& otherAgentsPositions) {
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
        // Check if the agent is allowed to tag (cooldown period has elapsed)
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - lastTagTime >= tagCooldownPeriod) {
            isTagging = true; // Set isTagging to true when starting to tag an enemy

            if (path.empty()) {
                std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                path = pathfinder->findPath(pos().x(), pos().y(), closestEnemy->pos().x(), closestEnemy->pos().y(), otherAgentsPositions, agentPositions);
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
                        if (isCarryingFlag) {
                            setIsCarryingFlag(false); // Drop the flag if the agent is carrying it
                            showFlagAtStartingPosition(); // Show the flag at its starting position
                            setPen(QPen(agentColor, 2)); // Restore the agent's original outline color
                        }

                        lastTagTime = currentTime; // Update the lastTagTime when a tag is made

                        // Continue the agent's movement after tagging the enemy
                        if (path.size() > currentPathIndex + 1) {
                            currentPathIndex++; // Set the next point as the current target for the next update
                        }
                        else {
                            // Reached the end of the path, generate a new exploration target
                            path.clear();
                            currentPathIndex = 0;
                            exploreField(otherAgentsPositions);
                        }
                        isTagging = false; // Set isTagging to false when the tagging behavior is completed
                    }
                }
            }
            else {
                // The agent has reached the enemy
                closestEnemy->isTagged = true; // Tag the enemy
                lastTagTime = currentTime; // Update the lastTagTime when a tag is made

                // Continue the agent's movement after tagging the enemy
                if (path.size() > currentPathIndex + 1) {
                    currentPathIndex++; // Set the next point as the current target for the next update
                }
                else {
                    // Reached the end of the path, generate a new exploration target
                    path.clear();
                    currentPathIndex = 0;
                    exploreField(otherAgentsPositions);
                }
                isTagging = false; // Set isTagging to false when the tagging behavior is completed
            }
        }
    }
    else {
        isTagging = false; // Set isTagging to false when no valid enemy is found
        exploreField(otherAgentsPositions);
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
            std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
            path = pathfinder->findPath(pos().x(), pos().y(), opponentWithFlagPos.first, opponentWithFlagPos.second, otherAgentsPositions, agentPositions);
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

    // Check if the agent is on its side of the field
    QPointF agentPos(pos().x(), pos().y());
    bool isAgentOnHomeSide = (side == "blue") ? agentPos.x() < gameFieldWidth / 2 : agentPos.x() >= gameFieldWidth / 2;

    if (!isAgentOnHomeSide)
        return false; // Agent cannot tag enemies when on the enemy side

    // Check if the enemy is on the opposite side of the field
    QPointF enemyPos(enemy->pos().x(), enemy->pos().y());
    bool isEnemyInOppositeSide = (side == "blue") ? enemyPos.x() >= gameFieldWidth / 2 : enemyPos.x() < gameFieldWidth / 2;

    // Check if the agent is within the tag range of the enemy
    float distance = calculateDistance(agentPos, enemyPos);
    const float tagRange = 200.0f;

    return isEnemyInOppositeSide && distance <= tagRange;
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

void Agent::hideFlag() {
    QGraphicsScene* scene = this->scene();
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (item->type() == QGraphicsItem::UserType + 1) { // Assuming the flag item has a unique user type
            item->setVisible(false); // Hide the flag item
            break;
        }
    }
}

void Agent::showFlagAtStartingPosition() {
    QGraphicsScene* scene = this->scene();
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (item->type() == QGraphicsItem::UserType + 1) { // Assuming the flag item has a unique user type
            item->setVisible(true); // Show the flag item
            item->setPos((side == "blue") ? redFlagPos : blueFlagPos); // Set the flag position based on the agent's side
            break;
        }
    }
}

void Agent::avoidEnemy(std::vector<Agent*>& otherAgents, const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    qreal speed = movementSpeed;
    Agent* closestEnemy = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    // Find the nearest enemy
    for (Agent* enemy : otherAgents) {
        if (enemy->side != side && !enemy->isTagged && canTagEnemy(enemy)) {
            float distance = calculateDistance(pos(), enemy->pos());
            if (distance < minDistance) {
                minDistance = distance;
                closestEnemy = enemy;
            }
        }
    }

    // Calculate the direction away from the nearest enemy
    QPointF awayDirection;
    if (closestEnemy != nullptr) {
        awayDirection = pos() - closestEnemy->pos();
    }
    else {
        // If no closest enemy found, generate a random direction
        awayDirection = QPointF(QRandomGenerator::global()->generateDouble() * 2 - 1,
            QRandomGenerator::global()->generateDouble() * 2 - 1);
    }

    qreal distance = std::sqrt(awayDirection.x() * awayDirection.x() + awayDirection.y() * awayDirection.y());
    if (distance > 0) {
        awayDirection /= distance;
    }

    // Move the agent away from the enemy
    setPos(pos() + awayDirection * std::min(distance, speed));

    // Check if it's safe to explore or go after the flag
    float distanceToFlag = calculateDistance(pos(), flagPos);
    float distanceToEnemy = distanceToNearestEnemy(otherAgentsPositions);
    if (distanceToEnemy > tagProximityThreshold) {
        if (distanceToFlag <= proximityThreshold) {
            // If the flag is nearby and no enemy is close, go after the flag
            path.clear();
            currentPathIndex = 0;
            std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
            path = pathfinder->findPath(pos().x(), pos().y(), flagPos.x(), flagPos.y(), otherAgentsPositions, agentPositions);
        }
        else {
            // If no enemy is close and the flag is not nearby, explore
            path.clear();
            currentPathIndex = 0;
            std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
            QPointF explorationTarget(QRandomGenerator::global()->bounded(0, gameFieldWidth),
                QRandomGenerator::global()->bounded(0, gameFieldHeight));
            path = pathfinder->findPath(pos().x(), pos().y(), explorationTarget.x(), explorationTarget.y(), otherAgentsPositions, agentPositions);
        }
    }
}

void Agent::defendFlag(std::vector<Agent*>& otherAgents, const std::vector<std::pair<int, int>>& otherAgentsPositions) {
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
        // Check if the agent is allowed to tag (cooldown period has elapsed)
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - lastTagTime >= tagCooldownPeriod) {
            isTagging = true; // Set isTagging to true when starting to tag an enemy

            if (path.empty()) {
                std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                path = pathfinder->findPath(pos().x(), pos().y(), closestEnemy->pos().x(), closestEnemy->pos().y(), otherAgentsPositions, agentPositions);
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
                        lastTagTime = currentTime; // Update the lastTagTime when a tag is made

                        // Continue the agent's movement after tagging the enemy
                        if (path.size() > currentPathIndex + 1) {
                            currentPathIndex++; // Set the next point as the current target for the next update
                        }
                        else {
                            // Reached the end of the path, generate a new exploration target
                            path.clear();
                            currentPathIndex = 0;
                            std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                            QPointF explorationTarget(QRandomGenerator::global()->bounded(0, gameFieldWidth),
                                QRandomGenerator::global()->bounded(0, gameFieldHeight));
                            path = pathfinder->findPath(pos().x(), pos().y(), explorationTarget.x(), explorationTarget.y(), otherAgentsPositions, agentPositions);
                        }
                        isTagging = false; // Set isTagging to false when the tagging behavior is completed
                    }
                }
            }
            else {
                // The agent has reached the enemy
                closestEnemy->isTagged = true; // Tag the enemy
                lastTagTime = currentTime; // Update the lastTagTime when a tag is made

                // Continue the agent's movement after tagging the enemy
                if (path.size() > currentPathIndex + 1) {
                    currentPathIndex++; // Set the next point as the current target for the next update
                }
                else {
                    // Reached the end of the path, generate a new exploration target
                    path.clear();
                    currentPathIndex = 0;
                    std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                    QPointF explorationTarget(QRandomGenerator::global()->bounded(0, gameFieldWidth),
                        QRandomGenerator::global()->bounded(0, gameFieldHeight));
                    path = pathfinder->findPath(pos().x(), pos().y(), explorationTarget.x(), explorationTarget.y(), otherAgentsPositions, agentPositions);
                }
                isTagging = false; // Set isTagging to false when the tagging behavior is completed
            }
        }
    }
    else {
        isTagging = false; // Set isTagging to false when no valid enemy is found

        // Check if there are no more enemies nearby
        if (distanceToNearestEnemy(otherAgentsPositions) > tagProximityThreshold) {
            // If no enemies are nearby, either explore the field or move towards the enemy flag
            if (QRandomGenerator::global()->generateDouble() < 0.5) {
                path.clear();
                currentPathIndex = 0;
                std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                QPointF explorationTarget(QRandomGenerator::global()->bounded(0, gameFieldWidth),
                    QRandomGenerator::global()->bounded(0, gameFieldHeight));
                path = pathfinder->findPath(pos().x(), pos().y(), explorationTarget.x(), explorationTarget.y(), otherAgentsPositions, agentPositions);
            }
            else {
                path.clear();
                currentPathIndex = 0;
                std::vector<std::pair<int, int>> agentPositions = getOtherAgentPositions(otherAgentsPositions);
                path = pathfinder->findPath(pos().x(), pos().y(), flagPos.x(), flagPos.y(), otherAgentsPositions, agentPositions);
            }
        }
    }
}

bool Agent::isInMiddleOfField() const {
    QPointF fieldCenter(gameFieldWidth / 2, gameFieldHeight / 2);
    float distanceToCenter = calculateDistance(pos(), fieldCenter);
    return distanceToCenter < 100.0f;
}

void Agent::setFlagPosition(const QPointF& position) {
    if (side == "blue") {
        redFlagPos = position;
    }
    else {
        blueFlagPos = position;
    }
}

void Agent::setBasePosition(const QPointF& position) {
    if (side == "blue") {
        blueBasePos = position;
    }
    else {
        redBasePos = position;
    }
}

void Agent::incrementScore() {
    if (side == "blue") {
        gameManager->incrementBlueScore();
    }
    else {
        gameManager->incrementRedScore();
    }
}

bool Agent::getIsCarryingFlag() const {
    return isCarryingFlag;
}