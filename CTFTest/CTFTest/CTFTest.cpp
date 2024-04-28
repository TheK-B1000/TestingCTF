#include "CTFTest.h"
#include "Agent.h"
#include "GameManager.h"

CTFTest::CTFTest(QWidget* parent)
    : QMainWindow(parent),
    gameManager(new GameManager(this)) {
    ui.setupUi(this);
    setCentralWidget(gameManager);
}

CTFTest::~CTFTest() {
    delete centralWidget();
}