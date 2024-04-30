#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QPointer>
#include "Agent.h"
#include "GameManager.h"
#include "Pathfinder.h"
#include <QList>

class GameManager : public QGraphicsView {
public:
    GameManager(QWidget* parent = nullptr);

    void setupScene();
    void setupAgents();
    void setupScoreDisplay();
    void setupTimeDisplay();
    void runTestCase1();
    void runTestCase2(int agentCount);
    void runTestCase3();
    void updateAgentPositions();
    bool isFlagCaptured(const std::string& side) const;
    void resetSimulation();
    void incrementBlueScore();
    void incrementRedScore();
    void gameLoop();
    void stopGame();
    void declareWinner();
    void updateScoreDisplay();
    void updateTimeDisplay();
    QGraphicsScene* getScene() const { return scene; }

    static int blueScore;
    static int redScore;

private:
    std::vector<std::shared_ptr<Agent>> blueAgents;
    std::vector<std::shared_ptr<Agent>> redAgents;
    QGraphicsScene* scene;
    QGraphicsEllipseItem* blueZone;
    QGraphicsEllipseItem* redZone;
    QGraphicsTextItem* timeRemainingTextItem;
    QPointer<QGraphicsTextItem> blueScoreTextItem;
    QPointer<QGraphicsTextItem> redScoreTextItem;
    QPointF redFlagPos;
    QPointF blueBasePos;
    QPointF blueFlagPos;
    QPointF redBasePos;
    QTimer* gameTimer;
    int timeRemaining;
    int gameFieldWidth;
    int gameFieldHeight;

};