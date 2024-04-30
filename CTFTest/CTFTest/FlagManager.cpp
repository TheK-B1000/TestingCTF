#include "FlagManager.h"
#include <QPainter>
#include "Agent.h"

FlagManager::FlagManager(QGraphicsItem* parent)
    : QGraphicsItem(parent), carriedBy(nullptr) {
}

void FlagManager::setCarriedBy(Agent* agent) {
    carriedBy = agent;
}

Agent* FlagManager::getCarriedBy() const {
    return carriedBy;
}