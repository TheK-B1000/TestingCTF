#include "Driver.h"
#include "GameManager.h"
#include <QMenuBar>
#include <QAction>
#include <QInputDialog>

Driver::Driver(QWidget* parent) : QMainWindow(parent) {
    gameManager = new GameManager(this);
    setCentralWidget(gameManager);

    // Create the menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // Create the "Test Cases" menu
    testCaseMenu = menuBar->addMenu("Test Cases");

    // Add actions for each test case
    QAction* testCase1Action = new QAction("Test Case 1: Default Setup", this);
    connect(testCase1Action, &QAction::triggered, this, &Driver::runTestCase1);
    testCaseMenu->addAction(testCase1Action);

    QAction* testCase2Action = new QAction("Test Case 2: Change Agent Count", this);
    connect(testCase2Action, &QAction::triggered, this, &Driver::runTestCase2);
    testCaseMenu->addAction(testCase2Action);

    QAction* testCase3Action = new QAction("Test Case 3: Change Team Zone Positions", this);
    connect(testCase3Action, &QAction::triggered, this, &Driver::runTestCase3);
    testCaseMenu->addAction(testCase3Action);
}

void Driver::runTestCase1() {
    gameManager->runTestCase1();
}

void Driver::runTestCase2() {
    bool ok;
    int agentCount = QInputDialog::getInt(this, "Test Case 2", "Enter the number of agents:", 8, 1, 100, 1, &ok);
    if (ok) {
        gameManager->runTestCase2(agentCount);
    }
}

void Driver::runTestCase3() {
    gameManager->runTestCase3();
}