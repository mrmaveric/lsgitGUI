#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString arg = "";
    if (a.arguments().length() > 1) {
        arg = a.arguments().at(1);
    }
    MainWindow w(0,arg);
    w.show();
    return a.exec();
}
