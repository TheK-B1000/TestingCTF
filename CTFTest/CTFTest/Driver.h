#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include "GameManager.h"

class Driver : public QMainWindow {
    Q_OBJECT

public:
    Driver(QWidget* parent = nullptr);

private slots:
    void runTestCase1();
    void runTestCase2();
    void runTestCase3();

private:
    QMenu* testCaseMenu;
    std::shared_ptr<GameManager> gameManager;
};