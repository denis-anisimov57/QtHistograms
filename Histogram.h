#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <QDebug>
#include <QObject>
#include <map>
#include <set>
#include "qcustomplot/qcustomplot.h"

/**
 * @file Histogram.h
 * @author Anisimov D
 * @brief Заголовочный файл для описания класса гистограмм
 * @date 25.04.2024
 */



//! \brief Определение вспомогательного типа данных для номеров источников и сообщений
//! \details Этот тип данных представляет собой std::map.
//! Ключ - номер источника, значение - множество номеров сообщений.
using MsgNumbersMap = std::map<int, std::set<int>>;
Q_DECLARE_METATYPE(MsgNumbersMap);

//! \brief Пространство имён для классов и структур, необходимых для построения гистограмм.
namespace hst {

const bool showDebugMessages = true;    //!< Когда включена выводит дополнительную информацию в qDebug()
const int legendRowCount = 10;          //!< Максимальное количество элементов в легенде в одном столбце
const double moveDelta = 0.01;          //!< Отображает, насколько подвинется график при передвижении, с помощью стрелочек
const double scaleCoeff = 1.01;         //!< Отображает, во сколько раз изменится диапазон осей при уменьшении масштаба
const double standartScale = 1.05;      //!< Начальный масштаб графика
const int statusNumberAccuracy = 2;     //!< Количество цифр после запятой в числах, отображаемых в полоске состояние(statusbar)
const double yTickStep = 1.0;           //!< Шаг меток на оси Oy

const int rangeR = 128;                 //!< Диапазон значений красного цвета
const int startR = 128;                 //!< Минимальное значение красного цвета
const int rangeG = 50;                  //!< Диапазон значение зелёного цвета
const int startG = 128;                 //!< Минимальное значение зелёного цвета
const int barAlpha = 170;               //!< Значение непрозрачности

const int penAlpha = 255;               //!< Значение непрозрачности границы
const double penWidth = 1.5;            //!< Ширина границы

//! \brief Структура, описывающая одно сообщение
struct Message {
    double value;   //!< Значение, полученное в сообщении
    int msgnum;     //!< Номер сообщения
};

//! \brief Структура, описывающая данные полученные из источника
struct Plot {
    std::vector<Message> messages;  //!< messages Сообщения из источника
    int sourcenum;                  //!< sourcenum Номер источника
};

//! \brief Структура, описывающая данные со всех полученных источников, а также значения для интервалов
struct AllPlotInfo
{
    double start = 0;                               //!< Начало интервалов
    double interval = 0;                            //!< Длина одного интервала

    std::vector<Plot> plots = std::vector<Plot>();  //!< Данные со всех полученных источников
};

//! \brief Класс, описывающий один интервал на графике гистограммы
class Interval {
    public:
        //! Конструктор по умолчанию
        Interval() = default;
        //! Конструктор с параметрами
        //! \param point1 Значение одной границы интервала
        //! \param point2 Значение другой границы интервала
        //! \details Создаст интервал с границами point1 и point2
        //! \note Порядок параметров не важен
        Interval(double point1, double point2);
        //! Деструктор по умолчанию
        ~Interval() = default;

        //! Метод проверки нахождения значения в интервале
        //! \param val Значение
        //! \return Если значение находится в инервале возвращает true, иначе false
        //! \note При проверке левая граница интервала включается, правая - нет
        bool inInterval(double val) const;
        //! Метод добавления сообщения в интервал
        //! \param msg Сообщение
        //! \param sourcenum Источник сообщения
        //! \return Возвращает успешное или неуспешное добавление сообщения
        //! \details Добавляет сообщение в интервал, если значение Message::value в сообщении находится в границах интервала
        //! \note При попытке добавить сообщение с номером сообщения, который уже был ранее успешно добавлен метод ничего не сделает.
        //! Информация об этом выведется в qDebug.
        bool addMsg(Message msg, int sourcenum);
        //! Метод получения левой границы интервала
        //! \return Левая граница интервала
        double getStart() const;
        //! Метод получения правой границы интервала
        //! \return Правая граница интервала
        double getEnd() const;
        //! Метод получения длины интервала
        //! \return Длина интервала, то есть конец интервала - начало интервала
        double length() const;
        //! Метод получения количества сообщений
        //! \return Количество сообщений по всем источникам в интервале
        unsigned long long msgCount() const;
        //! Метод получения данных о номерах источников и номерах сообщений в интервале
        //! \return Номера источников и соответствующие им номера сообщений
        MsgNumbersMap getIntervalData() const;
    private:
        double start = 0;
        double end = 0;
        MsgNumbersMap msgnumbers;
};

//! \brief Класс, описывающий график гистограммы
class Histogram : public QWidget {
    Q_OBJECT
public:
        explicit Histogram(QWidget* parent = nullptr);
        //! Конструктор с параметрами
        //! \param customPlot Указатель на виджет, на котором будет отображаться график
        //! \param statusbar Указатель на полоску состояния, на которой будет отображаться динамическая информация. Необязательный параметр.
        //! \note Полоска состояния будет отображать информацию о выбранном интервале.
        //! \exception std::runtime_error Если customPlot является пустым указателем, класс выбросит исключение.
        Histogram(QCustomPlot* customPlot, QStatusBar* statusbar = nullptr);

        //! Метод задания интервалов
        //! \param start Начало интервалов
        //! \param interval Длина одного интервала
        //! \details Задаёт значения интервалов на графике.
        //! Если в графике уже есть данные, перестроит график с новыми интервалами
        //! \exception std::runtime_error При неположительном значении длины интервала класс выбросит исключение
        void setIntervals(const double start, const double interval);

        //! Метод добавления данных из источника
        //! \param plotData Данные из источника
        //! \details Добавит новые данные и, если график уже построен, перестроит его
        //! \exception std::runtime_error Если не заданы значения интервалов, выбросит исключение
        void addPlot(const Plot plotData);

        //! Метод построения графика гистограммы
        //! \details Строит график гистограммы по построенным данным.
        //! Высота столбца отображает количество попавших в интервал значений сообщений.
        //! У столбцов генерируются случайные цвета.
        //! Метки на оси Ox соответствуют границами интервалов.
        //! В легенде имена отображают середину интервала.
        //! Добавляет взаимодействия с графиков: перетаскивание и масштабирование с помощью мыши,
        //! выбор элементов графика, выбор нескольких элементов графика при зажатой клавише Ctrl
        void drawHistogram();

        //! Метод для обработки ввода с клавиатуры
        //! \param key Значение клавиши из перечисления Qt::Key
        //! \details Позволяет двигать график с помощью стрелок или клавиш WASD.
        //! Позволяет масштабировать графиг с помощью кнопок +, -. Сброс масштаба на клавишу 0.
        //! Позволяет перегенерировать цвета столбцов на клавишу R.
        //! \note Вероятно, изменение цветов это временная тестовая возможность.
        void keyPressed(int key);

        //! Метод для переопределения цветов случайным образом
        //! \note Вероятно, это временный тестовый метод.
        void regenerateColors();

        //! Деструктор
        virtual ~Histogram() Q_DECL_OVERRIDE;
    private:
        void calculateIntervals(const Plot);

        bool isDrawn = false;
        QVector<int> selectedBars = QVector<int>();
        AllPlotInfo allData;
        std::vector<Interval> intervals;
        QCustomPlot* customPlot = nullptr;
        QLabel* statusLabel = nullptr;
        QStatusBar* statusbar = nullptr;
        QVBoxLayout* layout;
    public slots:
        //! Слот для сброса масштаба
        void resetScale();

        //! Слот для получения данных о выбранных столбцах
        //! \details Посылает сигнал dataSignal с данными о выбранных столбцах
        void getData();

        //! Слот, обрабатывающий изменение выбранных элементов графика
        //! \details При выбранных столбцах или элементах легенды
        //! добавляет в выбранные соответствующие им столбцы или элементы легенды.
        //! Если при создании класса был указан указатель на полоску состояния,
        //! то в ней отобразится информация о выбранном интервале
        //! Вызывается по сигналу QCustomPlot::selectionChangedByUser
        void selectionChanged();

        //! Слот, показывающий контекстное меню
        //! \param pos Позиция, на которой отобразится контекстное меню
        //! \details В контекстном меню, если выбраны какие-то столбцы,
        //! можно выбрать "Show table", который вызовет слот getData.
        //! Также можно выбрать "Reset scale", который сбросит масштаб графика, вызовом слота resetScale.
        //! Вызывается по сигналу QCustomPlot::customContextMenuRequested
        //! \note Вероятно временно: сигнал с данными ловится с помощью QSignalSpy и
        //! полученная информация выводится в qDebug
        void showMenu(const QPoint& pos);
    private slots:
        void keyPressEvent(QKeyEvent* event) override;
    signals:
        //! Сигнал, отправляющий данные о источниках и номерах сообщений
        //! \param msgnumbers Хранит источники сообщений и соответствующие им номера сообщений
        //! \details Посылается слотом getData.
        void dataSignal(MsgNumbersMap msgnumbers);
};

}

#endif // HISTOGRAM_H
