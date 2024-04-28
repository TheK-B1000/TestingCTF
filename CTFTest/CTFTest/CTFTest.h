#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CTFTest.h"
#include <QGraphicsView>

class GameManager;

class CTFTest : public QMainWindow {
    Q_OBJECT

public:
    CTFTest(QWidget* parent = nullptr);
    ~CTFTest();

private:
    Ui::CTFTestClass ui;
    GameManager* gameManager;
};