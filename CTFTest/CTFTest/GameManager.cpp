#include "GameManager.h"
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
        auto blueAgent = std::make_shared<Agent>(Qt::blue, redFlagPos, blueBasePos, scene);
        blueAgent->setPos(QRandomGenerator::global()->bounded(100), QRandomGenerator::global()->bounded(500));
        scene->addItem(blueAgent.get());
        blueAgents.push_back(blueAgent);

        auto redAgent = std::make_shared<Agent>(Qt::red, blueFlagPos, redBasePos, scene);
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
    timeRemainingTextItem->setPlainText("Time Remaining: 600");
    timeRemainingTextItem->setDefaultTextColor(Qt::black);
    timeRemainingTextItem->setFont(QFont("Arial", 16));
    timeRemainingTextItem->setPos(300, 10);
    scene->addItem(timeRemainingTextItem);


    // Start a timer to update agents
    int gameDuration = 600; // 10 minutes in seconds
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
