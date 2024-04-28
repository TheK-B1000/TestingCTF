#ifndef FLAGMANAGER_H
#define FLAGMANAGER_H

#include <QGraphicsItem>

class Agent;

class FlagManager : public QGraphicsItem {
public:
    FlagManager(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setCarriedBy(Agent* agent);
    Agent* getCarriedBy() const;

private:
    Agent* carriedBy;
};

#endif 