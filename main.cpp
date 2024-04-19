#include "mainwindow.h"

#include <QApplication>
#include <QTest>
#include "test_histogram.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTest::qExec(new Test_Histogram, argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
