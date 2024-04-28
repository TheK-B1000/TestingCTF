#include "FlagManager.h"
#include <QPainter>
#include "Agent.h"

FlagManager::FlagManager(QGraphicsItem* parent)
    : QGraphicsItem(parent), carriedBy(nullptr) {
}

QRectF FlagManager::boundingRect() const {
    // Return the bounding rectangle of the flag item
    // You'll need to adjust this based on your flag's size and shape
    return QRectF(-10, -20, 20, 40);
}

void FlagManager::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Paint the flag item on the graphics scene
    // You'll need to implement this based on your flag's appearance
    painter->setBrush(Qt::blue);
    painter->drawPolygon(QPolygon() << QPoint(-10, -20) << QPoint(0, 0) << QPoint(10, -20));
}

void FlagManager::setCarriedBy(Agent* agent) {
    carriedBy = agent;
}

Agent* FlagManager::getCarriedBy() const {
    return carriedBy;
}