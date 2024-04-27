#include "Agent.h"
#include "Pathfinder.h"
#include <QtMath>
#include <QBrush>
#include <QGraphicsScene>

Agent::Agent(QColor color, QPointF flagPos, QPointF basePos)
    : QGraphicsRectItem(nullptr), flagPos(flagPos), basePos(basePos), isCarryingFlag_(false)
{
    setRect(0, 0, 20, 20);
    setBrush(color);
}

void Agent::moveTowardsFlag() {
    if (!isCarryingFlag_) {
        QGraphicsScene* scene = this->scene();
        if (!scene) return; // Check if the scene is null

        // Assuming Pathfinder is correctly included and implemented
        Pathfinder pathfinder(scene->sceneRect().width(), scene->sceneRect().height());

        std::vector<std::pair<int, int>> path = pathfinder.findPath(
            pos().x(), pos().y(),
            flagPos.x(), flagPos.y()
        );

        if (!path.empty()) {
            // Move towards the next position in the path
            QPointF nextPos(path[0].first, path[0].second);
            QPointF direction = nextPos - pos();
            qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
            if (length > 0) {
                direction *= 10 / length; // Normalize and set speed
                setPos(pos() + direction);
            }
        }
    }
    else {
        moveTowardsBase();
    }
}

void Agent::moveTowardsBase() {
    QPointF direction = basePos - pos();
    qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length > 0) {
        direction *= 10 / length; // Normalize and set speed
        setPos(pos() + direction);
    }
}