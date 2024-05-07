#include "test_histogram.h"
#include <QTest>
#include <QSignalSpy>

hst::Test_Histogram::Test_Histogram(QObject* parent): QObject(parent) {}

// Инициализация данных для тестов
void hst::Test_Histogram::initTestCase() {
    qDebug() << "Starting tests for Histogram";
    qDebug() << "Initializing values...";

    H = new hst::Histogram();
    qcp = H->customPlot;

    qDebug() << "Done!";
}

// Тест метода установки интервалов
void hst::Test_Histogram::setIntervals() {
    std::vector<hst::Message> vec = {{1, 100}, {2, 200}, {3, 300}};

    // Тест добавления данных без установленных интервалов
    QVERIFY_EXCEPTION_THROWN(H->addPlot({vec, 1}), std::runtime_error);
    // Тест установки интервалов с отрицательной шириной
    QVERIFY_EXCEPTION_THROWN(H->setIntervals(0, -10), std::runtime_error);

    // Тест установки правильных интервалов
    H->setIntervals(-10, 30);
    try {
        H->addPlot({vec, 1});
    }
    catch(std::runtime_error& ex) {
        QFAIL("Incorrect excpetion: interval length is positive");
    }
}

// Тест добавления источника
void hst::Test_Histogram::addPlot() {
    std::vector<hst::Message> valVec =
    {{101, 1}, {243, 2}, {5, 3}, {67.3, 4}, {34, 5},
     {123, 6}, {-45, 7}, {-10, 8}, {10.8, 9}, {531, 10}};
    hst::Plot p = {valVec, 23};
    H->addPlot(p);
    H->drawHistogram();
    QSignalSpy spy(H, &hst::Histogram::dataSignal);
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
    }
    H->getData();
    // Тест полученных данных из сигнала
    MsgNumbersMap args = spy.takeFirst().at(0).value<MsgNumbersMap>();
    QVERIFY(!args.empty());
    QCOMPARE(args.size(), 2ul);
    std::set<int> msgnums1 = args[23];
    QCOMPARE(msgnums1.size(), 9ul);
    QCOMPARE(msgnums1.count(8), 1ul);
    QCOMPARE(msgnums1.count(10), 1ul);
    QCOMPARE(msgnums1.count(7), 0ul);
    QCOMPARE(msgnums1, std::set<int>({1, 2, 3, 4, 5, 6, 8, 9, 10}));
    std::set<int> msgnums2 = args[1];
    QCOMPARE(msgnums2, std::set<int>({100, 200, 300}));
}

// Тест выделения столбцов
void hst::Test_Histogram::selectBars() {
    qcp->deselectAll();
    // Тест количества столбцов
    QCOMPARE(qcp->plottableCount(), 19);
    // Выбор случайных столбцов
    for(int i = 0; i < 3; i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(qrand() % qcp->plottableCount()));
        bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
    }
    emit qcp->selectionChangedByUser();
    // Тест соответствия выделения легенды и столбцов
    for(int i = 0; i < 10; i++) {
        int barIndex = qrand() % qcp->plottableCount();
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(barIndex));
        QCPPlottableLegendItem* item = qcp->legend->itemWithPlottable(bar);
        QCOMPARE(bar->selected(), item->selected());
    }
}

// Тест информации в полоске состояния
void hst::Test_Histogram::statusbar() {
    std::vector<int> barIndexes = {0, 2, 6, 15, 18, 3, 4, 1};
    qcp->deselectAll();
    // Тест нахождения QLabel в полоске состояния
    QCOMPARE(H->statusbar, H->statusLabel->parentWidget());
    QLabel* label = H->statusLabel;

    // Формат вывода: "Height: 0.00; Interval: [0.00, 0.00]; Width: 0.00"
    // Тест информации о последнем выделенном столбце в полоске состояния
    for(unsigned long long i = 0; i < barIndexes.size(); i++) {
        if(i == 6) {
            qcp->deselectAll();
        }
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(barIndexes[i]));
        bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
        emit qcp->selectionChangedByUser();
        double tick = bar->data()->at(0)->mainKey();
        double width = 30;
        double start = tick - width / 2, end = tick + width / 2;
        QString height = QString::number(bar->data()->at(0)->mainValue(), 'f', hst::statusNumberAccuracy);
        QString status = "Height: ";
        status += height;
        status += "; Interval: [";
        status += QString::number(start, 'f', hst::statusNumberAccuracy);
        status += ", ";
        status += QString::number(end, 'f', hst::statusNumberAccuracy);
        status += "]; Width: ";
        status += QString::number(width, 'f' , hst::statusNumberAccuracy);
        QCOMPARE(label->text(), status);
    }
}

// Тест данных в столбцах
void hst::Test_Histogram::dataInIntervals() {
    std::vector<int> intHeights = {6, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    std::map<double, int> tickHeights;
    for(unsigned long long i = 0; i < intHeights.size(); i++) {
        tickHeights[5 + i * 30] = intHeights[i];
    }
    // Тест корректности абсциссы и высоты столбцов
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        double tick = bar->data()->at(0)->mainKey();
        try {
            tickHeights.at(tick);
        }
        catch(std::out_of_range) {
            std::string errStr = "Invalid bar tick: " + std::to_string(tick);
            QFAIL(errStr.c_str());
        }
        QCOMPARE(bar->data()->at(0)->mainValue(), tickHeights[tick]);
    }
}

// Тест смены интервалов с добавленными данными
void hst::Test_Histogram::changeIntervals() {
    H->setIntervals(0, 100);
    // Тест количества столбцов после изменения интервалов
    QCOMPARE(qcp->plottableCount(), 6);
    std::map<double, int> tickHeights = {{50, 7}, {150, 2}, {250, 1}, {350, 0}, {450, 0}, {550, 1}};
    // Тест корректности абсциссы и высоты столбцов после изменения интервалов
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        double tick = bar->data()->at(0)->mainKey();
        try {
            tickHeights.at(tick);
        }
        catch(std::out_of_range) {
            std::string errStr = "Invalid bar tick: " + std::to_string(tick);
            QFAIL(errStr.c_str());
        }
        QCOMPARE(bar->data()->at(0)->mainValue(), tickHeights[tick]);
    }
}

// Добавления ещё одного источника
void hst::Test_Histogram::addOtherPlot() {
    std::vector<hst::Message> valVec =
    {{105, 2}, {234, 10}, {5, 5}, {156, 6}};
    hst::Plot p = {valVec, 24};
    H->addPlot(p);
    H->drawHistogram();
    QSignalSpy spy(H, &hst::Histogram::dataSignal);
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
    }
    H->getData();
    // Тест полученных данных из сигнала
    MsgNumbersMap args = spy.takeFirst().at(0).value<MsgNumbersMap>();
    QVERIFY(!args.empty());
    QCOMPARE(args.size(), 3ul);
    std::set<int> msgnums1 = args[24];
    QCOMPARE(msgnums1, std::set<int>({2, 10, 5, 6}));
    std::set<int> msgnums2 = args[1];
    QCOMPARE(msgnums2, std::set<int>({100, 200, 300}));

    std::map<double, int> tickHeights = {{50, 8}, {150, 4}, {250, 2}, {350, 0}, {450, 0}, {550, 1}};
    // Тест корректности абсциссы и высоты столбцов
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        double tick = bar->data()->at(0)->mainKey();
        try {
            tickHeights.at(tick);
        }
        catch(std::out_of_range) {
            std::string errStr = "Invalid bar tick: " + std::to_string(tick);
            QFAIL(errStr.c_str());
        }
        QCOMPARE(bar->data()->at(0)->mainValue(), tickHeights[tick]);
    }
}

// Очищение данных после тестов
void hst::Test_Histogram::cleanupTestCase() {
    qDebug() << "Tests ended";
    qDebug() << "Deinitializing values...";

    delete H;

    qDebug() << "Done";
}
