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
#include <fstream>
#include <sstream>

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
    void runTestCase4();
    void runTestCase5();
    void updateAgentPositions();
    bool isFlagCaptured(const std::string& side) const;
    void resetSimulation();
    void resetScoreAndGameOverText();
    void incrementBlueScore();
    void incrementRedScore();
    void incrementTaggingFrequency(Agent* agent);
    void gameLoop();
    void stopGame();
    void declareWinner();
    void updateScoreDisplay();
    void updateTimeDisplay();
    void writeScoreHistoryToCSV(const std::string& filename);
    void writeTaggingFrequencyToCSV(const std::string& filename);
    void writeCollisionHeatmapToCSV(const std::string& filename);
    QGraphicsScene* getScene() const { return scene; }
    std::vector<std::shared_ptr<Agent>>& getBlueAgents() { return blueAgents; }
    std::vector<std::shared_ptr<Agent>>& getRedAgents() { return redAgents; }

    static int blueScore;
    static int redScore;

private:
    std::vector<std::shared_ptr<Agent>> blueAgents;
    std::vector<std::shared_ptr<Agent>> redAgents;
    std::vector<std::pair<int, int>> scoreHistory;
    std::unordered_map<Agent*, int> taggingFrequency;
    std::vector<std::vector<int>> collisionHeatmap;
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