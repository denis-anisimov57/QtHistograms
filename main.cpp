#include "mainwindow.h"

#include <QApplication>
#include <QTest>
#include "test_histogram.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    hst::Test_Histogram* testH = new hst::Test_Histogram();
    QTest::qExec(testH, argc, argv);
    delete testH;

    MainWindow w;
    w.show();
    return a.exec();
}
