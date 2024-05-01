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
    void runTestCase4();
    void runTestCase5();

private:
    QMenu* testCaseMenu;
    GameManager* gameManager;
};