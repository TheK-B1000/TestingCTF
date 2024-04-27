#include "CTFTest.h"
#include "Agent.h"
#include "GameManager.h"

CTFTest::CTFTest(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    GameManager* gameManager = new GameManager(this);
    setCentralWidget(gameManager);
}

CTFTest::~CTFTest()
{
    delete centralWidget();
}