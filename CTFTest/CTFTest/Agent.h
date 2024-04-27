#pragma once

#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QColor>
#include <QPointF>

class Agent : public QGraphicsRectItem {
public:
    Agent(QColor color, QPointF flagPos, QPointF basePos);
    void moveTowardsFlag();
    void moveTowardsBase();
    bool isCarryingFlag() const { return isCarryingFlag_; }

private:
    QPointF flagPos;
    QPointF basePos;
    bool isCarryingFlag_;
    std::vector<std::pair<int, int>> agentPath;
};