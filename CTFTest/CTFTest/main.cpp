#include "CTFTest.h"
#include "Driver.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    Driver w;
    w.show();

    // Display the menu and wait for the user to choose a test case
    QMenuBar* menuBar = w.menuBar();
    if (menuBar) {
        QMenu* testCaseMenu = menuBar->findChild<QMenu*>("testCaseMenu");
        if (testCaseMenu) {
            QAction* selectedAction = testCaseMenu->exec(w.mapToGlobal(QPoint(0, 0)));
            if (selectedAction) {
                selectedAction->trigger();
            }
        }
    }

    return a.exec();
}