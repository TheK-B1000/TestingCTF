#include "CTFTest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CTFTest w;
    w.show();

    // Display the menu and wait for the user to choose a test case
    QMenu* testCaseMenu = w.findChild<QMenu*>("Test Cases");
    if (testCaseMenu) {
        QAction* selectedAction = testCaseMenu->exec(w.mapToGlobal(QPoint(0, 0)));
        if (selectedAction) {
            selectedAction->trigger();
        }
    }
    return a.exec();
}
