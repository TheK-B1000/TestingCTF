#include "Agent.h"
#include "Pathfinder.h"
#include <QtMath>
#include <QBrush>
#include <QGraphicsScene>
#include <Brain.h>
#include <QRandomGenerator>

Agent::Agent(QColor color, QPointF flagPos, QPointF basePos, QGraphicsScene* scene)
    : QGraphicsEllipseItem(0, 0, 20, 20, nullptr), blueFlagPos(blueFlagPos),
    redFlagPos(redFlagPos), basePos(basePos), isCarryingFlag(false),
    currentPathIndex(0),
    pathfinder(std::make_unique<Pathfinder>(scene->sceneRect().width(), scene->sceneRect().height())),
    currentTarget(0, 0), brain(std::make_unique<Brain>()),
    gameFieldWidth(scene->sceneRect().width()),
    gameFieldHeight(scene->sceneRect().height())
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

    BrainDecision decision = brain->makeDecision(isCarryingFlag, checkInTeamZone(), distanceToFlag);

    switch (decision) {
    case BrainDecision::Explore:
        qDebug() << "Exploring field";
        moveTowardsFlag();
        break;
    case BrainDecision::GrabFlag:
        qDebug() << "Moving towards enemy flag";
        moveTowardsFlag();
        break;
    case BrainDecision::CaptureFlag:
        qDebug() << "Capturing flag";
        // Perform flag capture logic
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

float Agent::calculateDistance(const QPointF& pos1, const QPointF& pos2) {
    QPointF diff = pos1 - pos2;
    return std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
}


void Agent::moveTowardsFlag() {
    qreal speed = 10.0; // Adjust the speed as needed

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
    qreal speed = 10.0; // Adjust the speed as needed

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
    qreal speed = 10.0; // Adjust the speed as needed

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

// TODO- test and possibly remove or change
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

bool Agent::isPathEmpty() const {
    return path.empty();
}

bool Agent::checkInTeamZone() const {
    // check if the agent is in their team's zone
    return false;
}

bool Agent::isOpponentCarryingFlag() const {
    // check if an opponent is carrying the flag
    return false;
}