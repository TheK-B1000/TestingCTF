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

GameManager::GameManager(QWidget* parent) : QGraphicsView(parent) {
    setFixedSize(800, 600);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 800, 600);
    setScene(scene);

    // Add team zones (circular areas around flags)
    QGraphicsEllipseItem* blueZone = new QGraphicsEllipseItem(50, 260, 80, 80);
    QPen bluePen(Qt::blue);
    bluePen.setWidth(3);
    blueZone->setPen(bluePen);
    blueZone->setBrush(Qt::NoBrush);
    scene->addItem(blueZone);

    QGraphicsEllipseItem* redZone = new QGraphicsEllipseItem(690, 260, 80, 80);
    QPen redPen(Qt::red);
    redPen.setWidth(3);
    redZone->setPen(redPen);
    redZone->setBrush(Qt::NoBrush);
    scene->addItem(redZone);

    QPointF blueFlagPos = blueZone->rect().center();
    QPointF redFlagPos = redZone->rect().center();
    QPointF blueBasePos(50, 280);
    QPointF redBasePos(750, 280);

    // Create agents
    for (int i = 0; i < 4; ++i) {
        auto blueAgent = std::make_shared<Agent>(Qt::blue, redFlagPos, blueBasePos, scene, this);
        blueAgent->setPos(QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(blueAgent.get());
        blueAgents.push_back(blueAgent);

        auto redAgent = std::make_shared<Agent>(Qt::red, blueFlagPos, redBasePos, scene, this);
        redAgent->setPos(800 - QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(redAgent.get());
        redAgents.push_back(redAgent);
    }

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

    // Set up the score displays
    QGraphicsTextItem* blueScoreText = new QGraphicsTextItem();
    blueScoreTextItem = blueScoreText;
    blueScoreTextItem->setPlainText("Blue Score: 0");
    blueScoreTextItem->setDefaultTextColor(Qt::blue);
    blueScoreTextItem->setFont(QFont("Arial", 16));
    blueScoreTextItem->setPos(10, 10);
    scene->addItem(blueScoreTextItem);

    QGraphicsTextItem* redScoreText = new QGraphicsTextItem();
    redScoreTextItem = redScoreText;
    redScoreTextItem->setPlainText("Red Score: 0");
    redScoreTextItem->setDefaultTextColor(Qt::red);
    redScoreTextItem->setFont(QFont("Arial", 16));
    redScoreTextItem->setPos(600, 10);
    scene->addItem(redScoreTextItem);

    // Add time remaining display
    QGraphicsTextItem* timeRemainingText = new QGraphicsTextItem();
    timeRemainingTextItem = timeRemainingText;
    timeRemainingTextItem->setPlainText("Time Remaining: 2000");
    timeRemainingTextItem->setDefaultTextColor(Qt::black);
    timeRemainingTextItem->setFont(QFont("Arial", 16));
    timeRemainingTextItem->setPos(300, 10);
    scene->addItem(timeRemainingTextItem);


    // Start a timer to update agents
    int gameDuration = 2000;
    timeRemaining = gameDuration;
    blueScore = 0;
    redScore = 0;

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameManager::gameLoop);
    gameTimer->start(33);
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
}

void GameManager::runTestCase2(int agentCount) {
    // Clear the existing agents
    blueAgents.clear();
    redAgents.clear();

    int blueCount = agentCount / 2;
    int redCount = agentCount - blueCount;

    // Set up the new agents
    for (int i = 0; i < blueCount; ++i) {
        auto blueAgent = std::make_shared<Agent>(Qt::blue, redFlagPos, blueBasePos, scene, this);
        blueAgent->setPos(QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(blueAgent.get());
        blueAgents.push_back(blueAgent);
    }

    for (int i = 0; i < redCount; ++i) {
        auto redAgent = std::make_shared<Agent>(Qt::red, blueFlagPos, redBasePos, scene, this);
        redAgent->setPos(800 - QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(redAgent.get());
        redAgents.push_back(redAgent);
    }

    // Set up the score displays
    blueScoreTextItem->setPlainText("Blue Score: 0");
    redScoreTextItem->setPlainText("Red Score: 0");

    // Add time remaining display
    timeRemainingTextItem->setPlainText("Time Remaining: 2000");

    // Reset the game state
    timeRemaining = 2000;
    blueScore = 0;
    redScore = 0;

    // Start the game loop
    gameTimer->start(33);
}
void GameManager::runTestCase3() {
    // Test case 3: Change the position of team zones and flags
    QGraphicsPolygonItem* blueFlag = new QGraphicsPolygonItem();
    QGraphicsPolygonItem* redFlag = new QGraphicsPolygonItem();

    // Find the blue team zone
    QGraphicsEllipseItem* blueZone = nullptr;
    for (QGraphicsItem* item : scene->items()) {
        if (QGraphicsEllipseItem* ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            QPen pen = ellipseItem->pen();
            if (pen.color() == Qt::blue && pen.width() == 3) {
                blueZone = ellipseItem;
                break;
            }
        }
    }

    if (blueZone) {
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
        scene->addItem(blueFlag);
    }

    // Find the red team zone
    QGraphicsEllipseItem* redZone = nullptr;
    for (QGraphicsItem* item : scene->items()) {
        if (QGraphicsEllipseItem* ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            QPen pen = ellipseItem->pen();
            if (pen.color() == Qt::red && pen.width() == 3) {
                redZone = ellipseItem;
                break;
            }
        }
    }

    if (redZone) {
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
        scene->addItem(redFlag);
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