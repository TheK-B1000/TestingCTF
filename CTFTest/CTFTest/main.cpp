#include "CTFTest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CTFTest w;
    w.show();
    return a.exec();
}
