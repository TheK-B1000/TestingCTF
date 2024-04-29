#include "GameManager.h"
#include "Agent.h"
#include <QPainter>
#include <QPolygon>
#include <QRandomGenerator>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QFont>
#include <QTimer>
#include <QRandomGenerator>
#include <memory>

int GameManager::blueScore = 0;
int GameManager::redScore = 0;

GameManager::GameManager(QWidget* parent) : QGraphicsView(parent), gameFieldWidth(800), gameFieldHeight(600) {
    setFixedSize(800, 600);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 800, 600);
    setScene(scene);

    setupScene();
    setupAgents();

    // Set up the score displays
    setupScoreDisplay();

    // Add time remaining display
    setupTimeDisplay();

    // Start a timer to update agents
    int gameDuration = 2000;
    timeRemaining = gameDuration;
    blueScore = 0;
    redScore = 0;

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameManager::gameLoop);
    gameTimer->start(33);
}

void GameManager::setupScene() {
    // Add team zones (circular areas around flags)
    blueZone = new QGraphicsEllipseItem(50, 260, 80, 80);
    QPen bluePen(Qt::blue);
    bluePen.setWidth(3);
    blueZone->setPen(bluePen);
    blueZone->setBrush(Qt::NoBrush);
    scene->addItem(blueZone);

    redZone = new QGraphicsEllipseItem(690, 260, 80, 80);
    QPen redPen(Qt::red);
    redPen.setWidth(3);
    redZone->setPen(redPen);
    redZone->setBrush(Qt::NoBrush);
    scene->addItem(redZone);

    blueFlagPos = blueZone->rect().center();
    redFlagPos = redZone->rect().center();
    blueBasePos = QPointF(50, 280);
    redBasePos = QPointF(750, 280);

    // Add team areas fields
    QGraphicsRectItem* blueArea = new QGraphicsRectItem(5, 10, 400, 580);
    blueArea->setPen(QPen(Qt::blue, 2));
    scene->addItem(blueArea);

    QGraphicsRectItem* redArea = new QGraphicsRectItem(410, 10, 390, 580);
    redArea->setPen(QPen(Qt::red, 2));
    scene->addItem(redArea);

    // Create flags
    QGraphicsPolygonItem* blueFlag = new QGraphicsPolygonItem();
    QPolygon blueTriangle;
    qreal blueFlagCenter = blueZone->rect().center().y();
    blueTriangle << QPoint(blueFlagPos.x() - 10, blueFlagCenter - 20)
        << QPoint(blueFlagPos.x(), blueFlagCenter)
        << QPoint(blueFlagPos.x() + 10, blueFlagCenter - 20);
    blueFlag->setPolygon(blueTriangle);
    blueFlag->setBrush(Qt::blue);
    scene->addItem(blueFlag);

    QGraphicsPolygonItem* redFlag = new QGraphicsPolygonItem();
    QPolygon redTriangle;
    qreal redFlagCenter = redZone->rect().center().y();
    redTriangle << QPoint(redFlagPos.x() - 10, redFlagCenter - 20)
        << QPoint(redFlagPos.x(), redFlagCenter)
        << QPoint(redFlagPos.x() + 10, redFlagCenter - 20);
    redFlag->setPolygon(redTriangle);
    redFlag->setBrush(Qt::red);
    scene->addItem(redFlag);
}

void GameManager::setupAgents() {
    // Create agents
    for (int i = 0; i < 4; ++i) {
        auto blueAgent = std::make_shared<Agent>(Qt::blue, redFlagPos, blueBasePos, gameFieldWidth, gameFieldHeight, this);
        blueAgent->setPos(QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(blueAgent.get());
        blueAgents.push_back(blueAgent);

        auto redAgent = std::make_shared<Agent>(Qt::red, blueFlagPos, redBasePos, gameFieldWidth, gameFieldHeight, this);
        redAgent->setPos(800 - QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(redAgent.get());
        redAgents.push_back(redAgent);
    }
}

void GameManager::setupScoreDisplay() {
    // Set up the score displays
    blueScoreTextItem = new QGraphicsTextItem();
    blueScoreTextItem->setPlainText("Blue Score: 0");
    blueScoreTextItem->setDefaultTextColor(Qt::blue);
    blueScoreTextItem->setFont(QFont("Arial", 16));
    blueScoreTextItem->setPos(10, 10);
    scene->addItem(blueScoreTextItem);

    redScoreTextItem = new QGraphicsTextItem();
    redScoreTextItem->setPlainText("Red Score: 0");
    redScoreTextItem->setDefaultTextColor(Qt::red);
    redScoreTextItem->setFont(QFont("Arial", 16));
    redScoreTextItem->setPos(600, 10);
    scene->addItem(redScoreTextItem);
}

void GameManager::setupTimeDisplay() {
    // Add time remaining display
    timeRemainingTextItem = new QGraphicsTextItem();
    timeRemainingTextItem->setPlainText("Time Remaining: 2000");
    timeRemainingTextItem->setDefaultTextColor(Qt::black);
    timeRemainingTextItem->setFont(QFont("Arial", 16));
    timeRemainingTextItem->setPos(300, 10);
    scene->addItem(timeRemainingTextItem);
}
void GameManager::gameLoop() {
    // Calculate the elapsed time since the last frame
    int elapsedTime = gameTimer->interval();

    // Update the remaining time and display
    timeRemaining -= elapsedTime / 1000.0;
    updateTimeDisplay();

    // Collect the positions of all agents
    std::vector<std::pair<int, int>> otherAgentsPositions;
    for (const auto& agent : blueAgents) {
        otherAgentsPositions.emplace_back(agent->pos().x(), agent->pos().y());
    }
    for (const auto& agent : redAgents) {
        otherAgentsPositions.emplace_back(agent->pos().x(), agent->pos().y());
    }

    // Update the agents
    std::vector<Agent*> allAgents;
    for (const auto& agent : blueAgents) {
        agent->update(otherAgentsPositions, allAgents, elapsedTime);
        allAgents.push_back(agent.get());
    }

    for (const auto& agent : redAgents) {
        agent->update(otherAgentsPositions, allAgents, elapsedTime);
        allAgents.push_back(agent.get());
    }


    // Update only the changed portions of the scene
    for (const auto& agent : blueAgents) {
        if (!agent->isPathEmpty()) {
            viewport()->update(agent->boundingRect().toRect());
        }
    }

    for (const auto& agent : redAgents) {
        if (!agent->isPathEmpty()) {
            viewport()->update(agent->boundingRect().toRect());
        }
    }

    // Check if the game has ended
    if (timeRemaining <= 0) {
        stopGame();
        declareWinner();
    }
}

void GameManager::stopGame() {
    gameTimer->stop();

    for (const auto& agent : blueAgents) {
        agent->setEnabled(false);
    }

    for (const auto& agent : redAgents) {
        agent->setEnabled(false);
    }
}

void GameManager::declareWinner() {
    QGraphicsTextItem* winnerText = new QGraphicsTextItem();
    winnerText->setFont(QFont("Arial", 24));
    winnerText->setPos(300, 250);

    if (blueScore > redScore) {
        winnerText->setPlainText("Game Over! Blue Team Wins!");
        winnerText->setDefaultTextColor(Qt::blue);
    }
    else if (redScore > blueScore) {
        winnerText->setPlainText("Game Over! Red Team Wins!");
        winnerText->setDefaultTextColor(Qt::red);
    }
    else {
        winnerText->setPlainText("Game Over! It's a Draw!");
        winnerText->setDefaultTextColor(Qt::black);
    }

    scene->addItem(winnerText);
}

void GameManager::updateScoreDisplay() {
    blueScoreTextItem->setPlainText("Blue Score: " + QString::number(blueScore));
    redScoreTextItem->setPlainText("Red Score: " + QString::number(redScore));
}

void GameManager::updateTimeDisplay() {
    timeRemainingTextItem->setPlainText("Time Remaining: " + QString::number(timeRemaining));
}

void GameManager::runTestCase1() {
    // Test case 1: Default game setup (4 blue agents, 4 red agents)
    // No changes needed, as the default setup is already handled in the constructor
}

void GameManager::runTestCase2(int agentCount) {
    // Clear the existing agents
    for (const auto& agent : blueAgents) {
        scene->removeItem(agent.get());
    }
    blueAgents.clear();

    for (const auto& agent : redAgents) {
        scene->removeItem(agent.get());
    }
    redAgents.clear();

    // Calculate the number of blue and red agents
    int blueCount = agentCount / 2;
    int redCount = agentCount - blueCount;

    // Set up the new agents
    for (int i = 0; i < blueCount; ++i) {
        auto blueAgent = std::make_shared<Agent>(Qt::blue, redFlagPos, blueBasePos, gameFieldWidth, gameFieldHeight, this);
        blueAgent->setPos(QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(blueAgent.get());
        blueAgents.push_back(blueAgent);
    }

    for (int i = 0; i < redCount; ++i) {
        auto redAgent = std::make_shared<Agent>(Qt::red, blueFlagPos, redBasePos, gameFieldWidth, gameFieldHeight, this);
        redAgent->setPos(800 - QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(redAgent.get());
        redAgents.push_back(redAgent);
    }

    // Reset the scores
    blueScore = 0;
    redScore = 0;
    updateScoreDisplay();

    // Reset the time remaining
    int gameDuration = 2000;
    timeRemaining = gameDuration;
    updateTimeDisplay();

    // Stop the current game timer
    gameTimer->stop();

    // Start a new game timer
    gameTimer->start(33);
}

void GameManager::runTestCase3() {
    // Test case 3: Change the position of team zones and flags

    // Remove the old flags
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (item->type() == QGraphicsPolygonItem::Type) {
            scene->removeItem(item);
            delete item;
        }
    }

    // Create new flags
    QGraphicsPolygonItem* blueFlag = new QGraphicsPolygonItem();
    QGraphicsPolygonItem* redFlag = new QGraphicsPolygonItem();

    // Move the blue team zone to the top-left corner
    QRectF blueZoneRect(0, 0, 100, 100);
    blueZone->setRect(blueZoneRect);

    // Move the blue flag to the center of the new team zone position
    QPointF blueFlagPos = blueZoneRect.center();
    QPolygon blueTriangle;
    blueTriangle << QPoint(blueFlagPos.x() - 10, blueFlagPos.y() - 20)
        << QPoint(blueFlagPos.x(), blueFlagPos.y())
        << QPoint(blueFlagPos.x() + 10, blueFlagPos.y() - 20);
    blueFlag->setPolygon(blueTriangle);
    blueFlag->setBrush(Qt::blue);
    scene->addItem(blueFlag);

    // Move the red team zone to the bottom-right corner
    QRectF redZoneRect(700, 500, 100, 100);
    redZone->setRect(redZoneRect);

    // Move the red flag to the center of the new team zone position
    QPointF redFlagPos = redZoneRect.center();
    QPolygon redTriangle;
    redTriangle << QPoint(redFlagPos.x() - 10, redFlagPos.y() - 20)
        << QPoint(redFlagPos.x(), redFlagPos.y())
        << QPoint(redFlagPos.x() + 10, redFlagPos.y() - 20);
    redFlag->setPolygon(redTriangle);
    redFlag->setBrush(Qt::red);
    scene->addItem(redFlag);

    // Update the flag positions
    this->blueFlagPos = blueFlagPos;
    this->redFlagPos = redFlagPos;

    // Update the base positions
    blueBasePos = QPointF(50, 50);
    redBasePos = QPointF(750, 550);

    // Update the agent positions and paths
    updateAgentPositions();
    for (const auto& agent : blueAgents) {
        agent->updatePath();
    }
    for (const auto& agent : redAgents) {
        agent->updatePath();
    }

    // Reset the scores
    blueScore = 0;
    redScore = 0;
    updateScoreDisplay();

    // Reset the time remaining
    int gameDuration = 2000;
    timeRemaining = gameDuration;
    updateTimeDisplay();

    // Stop the current game timer
    gameTimer->stop();

    // Start a new game timer
    gameTimer->start(33);
}

void GameManager::updateAgentPositions() {
    for (const auto& agent : blueAgents) {
        agent->setFlagPosition(redFlagPos);
        agent->setBasePosition(blueBasePos);
    }

    for (const auto& agent : redAgents) {
        agent->setFlagPosition(blueFlagPos);
        agent->setBasePosition(redBasePos);
    }
}

void GameManager::incrementBlueScore() {
    blueScore++;
    updateScoreDisplay();
}

void GameManager::incrementRedScore() {
    redScore++;
    updateScoreDisplay();
}