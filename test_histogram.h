#ifndef TEST_HISTOGRAM_H
#define TEST_HISTOGRAM_H
#include <QObject>
#include "Histogram.h"

namespace hst {

class Test_Histogram: public QObject {
    Q_OBJECT
    public:
        Test_Histogram(QObject* parent = 0);
    private:
        hst::Histogram* H = nullptr;
        QCustomPlot* qcp = nullptr;
    private slots:
        void initTestCase();    //!< Инициализация данных для тестов
        void setIntervals();    //!< Тест метода установки интервалов
        void addPlot();         //!< Тест добавления нового источника
        void selectBars();      //!< Тест выделения столбцов и элементов легенды
        void statusbar();       //!< Тест вывода информации в полоску состояния
        void dataInIntervals(); //!< Тест корректности данных в интервалах
        void changeIntervals(); //!< Тест смены интервалов с добавленными данными
        void addOtherPlot();    //!< Тест добавления ещё одного источника
        void cleanupTestCase(); //!< Очищения данных после тестов
};

}

#endif // TEST_HISTOGRAM_H
