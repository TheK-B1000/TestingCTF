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
    isCarryingBlueFlag(false),
    isCarryingRedFlag(false),
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
    tagProximityThreshold(250.0f),
    proximityThreshold(400.0f)
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
    bool inSide = isOnOwnSide();

    // Check if the agent is in the middle of the field
    bool isInMiddle = isInMiddleOfField();
    if (isInMiddle) {
        middleStuckTime += elapsedTime;
    }
    else {
        middleStuckTime = 0;
    }
    bool isStuckInMiddle = middleStuckTime > 5000; // Adjust the threshold value as needed

    BrainDecision decision = brain->makeDecision(isCarryingBlueFlag, isCarryingRedFlag, checkInTeamZone(this->blueFlagPos, this->redFlagPos), distanceToFlag, isTagged, enemyHasFlag, distanceToEnemy, isTagging, isStuckInMiddle, inSide);

    if (isTagged) {
        // Change the agent's color to pink
        agentColor = Qt::magenta;
        setPen(QPen(agentColor, 2));
    }
    else if (isCarryingBlueFlag) { // Should be isCarryingBlueFlag
        // Change the blue agent's color to gold when carrying the blue flag
        agentColor = Qt::yellow;
        setPen(QPen(agentColor, 2));
    }
    else if (isCarryingRedFlag) {
        // Change the red agent's color to gold when carrying the red flag
        agentColor = Qt::yellow;
        setPen(QPen(agentColor, 2));
    }
    else {
        // Restore the agent's original color
        agentColor = (side == "blue") ? Qt::blue : Qt::red;
        setPen(QPen(agentColor, 2));
    }

    // Prioritize grabbing the flag if the agent is close to it or there are no enemies nearby
    if (!isCarryingBlueFlag && !isCarryingRedFlag && !isTagged && (distanceToFlag <= 250.0f || distanceToEnemy > 100.0f)) { // Should be isCarryingBlueFlag
        decision = BrainDecision::GrabFlag;
    }
    switch (decision) {
    case BrainDecision::Explore:
        qDebug() << "Exploring field";
        exploreField(otherAgentsPositions, otherAgents);
        break;
    case BrainDecision::GrabFlag:
        qDebug() << "Moving towards enemy flag";
        moveTowardsFlag(otherAgentsPositions);
        break;
    case BrainDecision::CaptureFlag:
        qDebug() << "Moving towards home zone";
        moveTowardsBase(otherAgentsPositions, otherAgents);
        break;
    case BrainDecision::AvoidEnemy:
        qDebug() << "Avoiding enemy";
        exploreField(otherAgentsPositions, otherAgents);
        break;
    case BrainDecision::RecoverFlag:
        qDebug() << "Chasing opponent with flag";
        moveTowardsBase(otherAgentsPositions, otherAgents);
        break;
    case BrainDecision::DefendFlag:
        qDebug() << "Defending flag";
        exploreField(otherAgentsPositions, otherAgents);
        break;
    case BrainDecision::TagEnemy:
        qDebug() << "Tagging enemy";
        exploreField(otherAgentsPositions, otherAgents);
        break;
    case BrainDecision::ReturnToHomeZone:
        qDebug() << "Returning to home zone";
        moveTowardsBase(otherAgentsPositions, otherAgents);
        break;
    default:
        qDebug() << "Exploring field";
        exploreField(otherAgentsPositions, otherAgents);
        break;
    }
}

float Agent::calculateDistance(const QPointF& pos1, const QPointF& pos2) const {
    QPointF diff = pos1 - pos2;
    return std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
}


void Agent::moveTowardsTarget(const QPointF& targetPos, const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    qreal speed = movementSpeed;

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
            setPos(pos() + direction * std::min(distance, speed));
        }

        // Check if the agent has reached the current target or the path point
        if (distance <= speed) {
            currentPathIndex++; // Prepare for the next waypoint

            // Check if the agent has reached the end of the path (target position)
            if (currentPathIndex >= path.size()) {
                // Reached the target position
                path.clear();
                currentPathIndex = 0;
            }
        }
    }
}



void Agent::moveTowardsFlag(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    QPointF targetFlagPos = (side == "blue") ? redFlagPos : blueFlagPos;
    moveTowardsTarget(targetFlagPos, otherAgentsPositions);

    // Check if the agent has reached the coordinates of the enemy team's flag
    QPointF agentPos = pos();
    qreal distance = QLineF(agentPos, targetFlagPos).length();
    qreal threshold = 20.0; // Adjust this threshold value as needed

    if (distance <= threshold) {
        setIsCarryingFlag(true); // The agent reaches the flag and captures it
        hideFlag(); // Hide the flag when the agent picks it up
        setPen(QPen(Qt::yellow, 2)); // Set the agent's outline color to gold
    }
}

void Agent::moveTowardsBase(const std::vector<std::pair<int, int>>& otherAgentsPositions, std::vector<Agent*>& otherAgents) {
    QPointF targetBasePos = (side == "blue") ? blueBasePos : redBasePos;
    moveTowardsTarget(targetBasePos, otherAgentsPositions);

    // Check if the agent has reached the end of the path (base position)
    if (currentPathIndex >= path.size()) {
        if (isTagged || checkInTeamZone(this->blueFlagPos, this->redFlagPos)) {
            // If the agent is tagged or in its team zone, reset the tagged status
            isTagged = false;
        }

        if (side == "blue" && isCarryingBlueFlag && !isTagged) { // Replace isCarryingFlag with isCarryingBlueFlag
            setIsCarryingFlag(false); // The blue agent reaches the base and drops the blue flag
            showFlagAtStartingPosition(); // Show the blue flag at its starting position
            incrementScore(); // Increment the score for the blue team
        }
        else if (side == "red" && isCarryingRedFlag && !isTagged) { // Replace isCarryingFlag with isCarryingRedFlag
            setIsCarryingFlag(false); // The red agent reaches the base and drops the red flag
            showFlagAtStartingPosition(); // Show the red flag at its starting position
            incrementScore(); // Increment the score for the red team
        }
        else {
            if (side == "blue") {
                isCarryingBlueFlag = false; // The blue agent reaches the base but is tagged, so it drops the blue flag without scoring
            }
            else {
                isCarryingRedFlag = false; // The red agent reaches the base but is tagged, so it drops the red flag without scoring
            }
        }

        setPen(QPen(agentColor, 2)); // Restore the agent's original outline color

        // Determine the next action for the agent
        if (isTagged) {
            // If the agent is tagged, explore the field
            exploreField(otherAgentsPositions, otherAgents);
        }
        else {
            if (QRandomGenerator::global()->generateDouble() < 0.5) {
                // 50% chance to explore the field
                exploreField(otherAgentsPositions, otherAgents);
            }
            else {
                // 50% chance to move towards the flag
                moveTowardsFlag(otherAgentsPositions);
            }
        }
    }
}
void Agent::updatePath(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    QPointF targetPos;

    if (side == "blue" && isCarryingBlueFlag) {
        // If a blue agent is carrying the blue flag, set the target position to the blue base
        targetPos = blueBasePos;
    }
    else if (side == "red" && isCarryingRedFlag) {
        // If a red agent is carrying the red flag, set the target position to the red base
        targetPos = redBasePos;
    }
    else {
        // If not carrying a flag, set the target position to the enemy flag
        targetPos = (side == "blue") ? redFlagPos : blueFlagPos;
    }

    moveTowardsTarget(targetPos, otherAgentsPositions);

    // Check if the agent has reached the end of the path (target position)
    if (currentPathIndex >= path.size()) {
        if (side == "blue" && isCarryingBlueFlag) {
            // If a blue agent is carrying the blue flag and reaches the blue base
            if (checkInTeamZone(this->blueFlagPos, this->redFlagPos)) {
                isCarryingBlueFlag = false;
                showFlagAtStartingPosition();
                incrementScore();
            }
            else {
                // If not in the team zone, drop the blue flag without scoring
                isCarryingBlueFlag = false;
                showFlagAtStartingPosition();
            }
        }
        else if (side == "red" && isCarryingRedFlag) {
            // If a red agent is carrying the red flag and reaches the red base
            if (checkInTeamZone(this->blueFlagPos, this->redFlagPos)) {
                isCarryingRedFlag = false;
                showFlagAtStartingPosition();
                incrementScore();
            }
            else {
                // If not in the team zone, drop the red flag without scoring
                isCarryingRedFlag = false;
                showFlagAtStartingPosition();
            }
        }
        else {
            // If not carrying a flag, check if the agent is at the opposite color flag position
            if (side == "blue" && qFuzzyCompare(pos().x(), redFlagPos.x()) && qFuzzyCompare(pos().y(), redFlagPos.y())) {
                setIsCarryingFlag(true);
                hideFlag();
            }
            else if (side == "red" && qFuzzyCompare(pos().x(), blueFlagPos.x()) && qFuzzyCompare(pos().y(), blueFlagPos.y())) {
                setIsCarryingFlag(true);
                hideFlag();
            }
        }
    }
}

void Agent::exploreField(const std::vector<std::pair<int, int>>& otherAgentsPositions, std::vector<Agent*>& otherAgents) {
    // Define a target position for exploration
    QPointF explorationTarget;

    // Generate a random target position within the game field
    explorationTarget.setX(QRandomGenerator::global()->bounded(0, gameFieldWidth));
    explorationTarget.setY(QRandomGenerator::global()->bounded(0, gameFieldHeight));

    moveTowardsTarget(explorationTarget, otherAgentsPositions);

    // Check if the agent has reached the end of the path (exploration target)
    if (currentPathIndex >= path.size()) {
        // Check if the agent is on the opposite side
        if (!isOnOwnSide()) {
            // Check if the agent can be tagged by an enemy
            for (Agent* enemy : otherAgents) {
                if (enemy->side != side && !enemy->isTagged && enemy->canTagEnemy(this)) {
                    isTagged = true;
                    if (isCarryingBlueFlag || isCarryingRedFlag) {
                        setIsCarryingFlag(false); // Drop the flag if the agent is tagged while carrying it
                        showFlagAtStartingPosition(); // Show the flag at its starting position
                    }
                    handleTagged(otherAgentsPositions, otherAgents);
                    return;
                }
            }
        }

        // Check if the agent is on its own side
        if (isOnOwnSide()) {
            // Find the closest enemy that can be tagged
            Agent* closestEnemy = nullptr;
            float minDistance = std::numeric_limits<float>::max();
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

                    if (distance >= 0) {
                        direction /= distance; // Normalize the direction vector

                        // Move the agent towards the target
                        setPos(pos() + direction * std::min(distance, movementSpeed));

                        // Check if the agent has reached the target or a path point
                        if (distance <= tagProximityThreshold) {
                            closestEnemy->isTagged = true; // Tag the enemy
                            if (isCarryingBlueFlag || isCarryingRedFlag) {
                                if (side == "blue") {
                                    isCarryingBlueFlag = false; // Drop the blue flag if the blue agent is tagged while carrying it
                                }
                                else {
                                    isCarryingRedFlag = false; // Drop the red flag if the red agent is tagged while carrying it
                                }
                                showFlagAtStartingPosition(); // Show the flag at its starting position
                            }
                            lastTagTime = currentTime; // Update the lastTagTime when a tag is made
                            isTagging = false; // Set isTagging to false when the tagging behavior is completed

                            // Move towards the flag or explore the field after tagging the enemy
                            if (isCarryingBlueFlag || isCarryingRedFlag) {
                                moveTowardsBase(otherAgentsPositions, otherAgents);
                            }
                            else {
                                moveTowardsFlag(otherAgentsPositions);
                            }
                        }
                    }
                }
            }
        }
        else {
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
        }

        // Reached the exploration target, generate a new random target
        path.clear();
        currentPathIndex = 0;
        exploreField(otherAgentsPositions, otherAgents);
    }
}
void Agent::handleTagged(const std::vector<std::pair<int, int>>& otherAgentsPositions, std::vector<Agent*>& otherAgents) {
    // Set the target position to the agent's color base
    QPointF targetBasePos = (side == "blue") ? blueBasePos : redBasePos;

    moveTowardsTarget(targetBasePos, otherAgentsPositions);

    // Check if the agent has reached the end of the path (base position)
    if (currentPathIndex >= path.size()) {
        isTagged = false; // Reset the tagged status when the agent reaches its base

        // Determine the next action for the agent
        if (QRandomGenerator::global()->generateDouble() < 0.5) {
            // 50% chance to explore the field
            exploreField(otherAgentsPositions, otherAgents);
        }
        else {
            // 50% chance to move towards the flag
            moveTowardsFlag(otherAgentsPositions);
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


std::vector<std::pair<int, int>> Agent::getOtherAgentPositions(const std::vector<std::pair<int, int>>& otherAgentsPositions) {
    std::vector<std::pair<int, int>> agentPositions;

    for (const auto& pos : otherAgentsPositions) {
        if (pos != std::make_pair(static_cast<int>(this->pos().x()), static_cast<int>(this->pos().y()))) {
            agentPositions.push_back(pos);
        }
    }

    return agentPositions;
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

bool Agent::isOnOwnSide() const {
    QPointF agentPos(pos().x(), pos().y());

    if (side == "blue") {
        // Blue team's side is the left half of the field
        return agentPos.x() < gameFieldWidth / 2;
    }
    else {
        // Red team's side is the right half of the field
        return agentPos.x() >= gameFieldWidth / 2;
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


bool Agent::isInMiddleOfField() const {
    QPointF fieldCenter(gameFieldWidth / 2, gameFieldHeight / 2);
    float distanceToCenter = calculateDistance(pos(), fieldCenter);
    return distanceToCenter < 100.0f;
}

void Agent::hideFlag() {
    QGraphicsScene* scene = this->scene();
    QList<QGraphicsItem*> items = scene->items();

    for (QGraphicsItem* item : items) {
        QVariant userData = item->data(QGraphicsItem::UserType);
        if (userData.typeId() == QMetaType::Int) {
            if (side == "blue" && userData.toInt() == QGraphicsItem::UserType + 2) {
                item->setVisible(false); // Hide the red flag
            }
            else if (side == "red" && userData.toInt() == QGraphicsItem::UserType + 1) {
                item->setVisible(false); // Hide the blue flag
            }
        }
    }
}

void Agent::showFlagAtStartingPosition() {
    QGraphicsScene* scene = this->scene();
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if ((side == "blue" && item->data(QGraphicsItem::UserType) == QGraphicsItem::UserType + 2) ||
            (side == "red" && item->data(QGraphicsItem::UserType) == QGraphicsItem::UserType + 1)) {
            item->setVisible(true); // Show the flag item

            // Set the flag position to the center of the team zone
            QPointF teamZoneCenter;
            if (side == "blue") {
                teamZoneCenter = GameManager::getBlueZoneCenter();
            }
            else {
                teamZoneCenter = GameManager::getRedZoneCenter();
            }
            item->setPos(teamZoneCenter);

            break;
        }
    }
}

void Agent::setIsCarryingFlag(bool isCarrying) {
    if (side == "blue") {
        isCarryingRedFlag = isCarrying;
    }
    else {
        isCarryingBlueFlag = isCarrying;
    }
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
    if (side == "blue") {
        return isCarryingRedFlag;
    }
    else {
        return isCarryingBlueFlag;
    }
}