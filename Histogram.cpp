#include "Histogram.h"
#include <QSignalSpy>
#include <QMenu>

Interval::Interval(double start, double end) {
    if(start > end) {
        double tmp = end;
        end = start;
        start = tmp;
    }
    this->start = start;
    this->end = end;
}

bool Interval::inInterval(double val) const {
    return ((val > start) && (val <= end));
}

bool Interval::addMsg(Val val, int sourcenum) {
    if(inInterval(val.val)) {
        if(!msgnumbers.count(sourcenum)) {
            msgnumbers[sourcenum] = std::set<int>();
        }
        if(msgnumbers[sourcenum].count(val.msgnum)) {
            qDebug() << "Interval[" << start << ", " << end << "]: Trying to add msgnum = " << val.msgnum << " which already exists\n";
        }
        else {
            msgnumbers[sourcenum].insert(val.msgnum);
        }
        return true;
    }
    return false;
}

double Interval::length() const {
    return end - start;
}

unsigned long long Interval::msgCount() const {
    unsigned long long count = 0;
    for(auto it = msgnumbers.begin(); it != msgnumbers.end(); it++) {
        count += it->second.size();
    }
    return count;
}

MsgNumbersMap Interval::getIntervalData() const {
    return msgnumbers;
}

Histogram::Histogram(QCustomPlot* customPlot) {
    this->customPlot = customPlot;
    allData = AllPlotInfo();
    intervals = std::vector<Interval>();
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(10);
}

Histogram::~Histogram() {};

void Histogram::move(int key) {
    QCPAxis* x = customPlot->xAxis;
    QCPAxis* y = customPlot->yAxis;
    double moveSpeed = 0.01;
    double scale = 1.01;
    if(key == Qt::Key_Left || key == Qt::Key_A) {
        x->moveRange(-moveSpeed * x->range().size());
    }
    if(key == Qt::Key_Right || key == Qt::Key_D) {
        x->moveRange(moveSpeed * x->range().size());
    }
    if(key == Qt::Key_Up || key == Qt::Key_W) {
        y->moveRange(moveSpeed * y->range().size());
    }
    if(key == Qt::Key_Down || key == Qt::Key_S) {
        y->moveRange(-moveSpeed * y->range().size());
    }
    if(key == Qt::Key_Minus) {
        x->scaleRange(scale);
        y->scaleRange(scale);
    }
    if(key == Qt::Key_Plus) {
        x->scaleRange(1 / scale);
        y->scaleRange(1 / scale);
    }
    if(key == Qt::Key_0) {
        customPlot->rescaleAxes();
    }
    if(key == Qt::Key_R) {
        regenerateColors(0);
    }
    if(key == Qt::Key_T) {
        regenerateColors(1);
    }
    if(key == Qt::Key_Q) {
        setIntervals(0, 5);
    }
    customPlot->replot();
}

void Histogram::setUIPlot(QCustomPlot *customPlot) {
    this->customPlot = customPlot;
    allData = AllPlotInfo();
    intervals = std::vector<Interval>();
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(10);

    connect(customPlot, &QCustomPlot::selectionChangedByUser, this, &Histogram::selectionChanged);

    customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(customPlot, &QCustomPlot::customContextMenuRequested, this, &Histogram::showMenu);
}

void Histogram::selectionChanged() {
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        QCPPlottableLegendItem* item = customPlot->legend->itemWithPlottable(bar);
        if(item->selected() || bar->selected()) {
            item->setSelected(true);
            bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
        }
    }
}

void Histogram::showMenu(const QPoint& pos) {
    QMenu contextMenu("Context menu", customPlot);
    QAction act1("Show table", customPlot);
    contextMenu.addAction(&act1);

    QSignalSpy spy(this, &Histogram::dataSignal);

    connect(&act1, &QAction::triggered, this, &Histogram::getData);
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        if(bar->selected()) {
            contextMenu.exec(customPlot->mapToGlobal(pos));
            break;
        }
    }

    //testing recieved data from signal
    if(spy.count()) {
        MsgNumbersMap args = spy.takeFirst().at(0).value<MsgNumbersMap>();
        for(auto& src : args) {
            for(auto& msgnum : src.second) {
                qDebug() << "[source: " << src.first << ", msgnum: " << msgnum << "]";
            }
        }
        qDebug() << "\n";
    }
}

void Histogram::calculateIntervals(const Plot plotData) {
    std::vector<Val> data = plotData.vec;
    double max = data[0].val;
    for(Val val : data) {
        if(val.val > max) {
            max = val.val;
        }
    }
    if(intervals.empty() || max > intervals[intervals.size() - 1].end) {
        double intCount = (max - allData.start) / allData.interval;
        if(intCount - int(intCount) != 0) intCount++;
        this->intervals.resize(intCount);
    }

    //first interval
    if(!intervals[0].length()) {
        intervals[0] = Interval(allData.start, allData.start + allData.interval);
    }
    for(unsigned long long j = 0; j < data.size(); j++) {
        intervals[0].addMsg(data[j], plotData.sourcenum);
    }

    //other intervals
    for(unsigned long long i = 1; i < intervals.size(); i++) {
        if(!intervals[i].length()) {
            intervals[i] = Interval(intervals[i - 1].end, intervals[i - 1].end + allData.interval);
        }
        for(unsigned long long j = 0; j < data.size(); j++) {
            intervals[i].addMsg(data[j], plotData.sourcenum);
        }
    }
}

void Histogram::addPlot(const Plot plotData) {
    if(allData.interval <= 0) {
        throw(std::runtime_error("Invalid interval length"));
    }
    allData.PlotVec.push_back(plotData);
    calculateIntervals(plotData);
}

void Histogram::setIntervals(const double start, const double interval) {
    if(interval <= 0) {
        throw(std::runtime_error("Invalid interval length"));
    }
    allData.start = start;
    allData.interval = interval; 
    if(!allData.PlotVec.empty()) {
        intervals.clear();
        for(auto& plotData : allData.PlotVec) {
            calculateIntervals(plotData);
        }
        if(isDrawn) {
            drawHistogram();
        }
    }
}

void Histogram::regenerateColors(int type) {
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        QColor color;
        if(type == 0) {
            unsigned char R = qrand() % 128 + 128, G = qrand() % 50 + 128, B = qrand() % R;
            color.setRgb(R, G, B, 170);
        }
        else if(type == 1) {
            int H = (qrand() % 120 + 300) % 360, S = qrand() % 56 + 200, L = 140;
            color.setHsl(H, S, L, 170);
        }

        bar->setBrush(QBrush(color));
        color.setAlpha(255);
        bar->setPen(QPen(color));
//        QCPSelectionDecorator* dec;
//        dec->setPen()
//        bar->setSelectionDecorator(dec);
    }
}

void Histogram::drawHistogram() {
    isDrawn = true;
    customPlot->clearPlottables();
    customPlot->clearItems();
    std::vector<QCPBars*> bars(intervals.size());
    std::vector<int> ticks(intervals.size());
    std::vector<int> values(intervals.size());
    for(unsigned long long i = 0; i < intervals.size(); i++) {
        double tick = 0;
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);

        /*R from 128 to 255
        G from 0 to 128
        B from 0 to R*/
        unsigned char R = qrand() % 128 + 128, G = qrand() % 50 + 128, B = qrand() % R;
        bars[i]->setBrush(QBrush(QColor(R, G, B, 170)));

        bars[i]->setWidth(intervals[i].length());
        tick = (intervals[i].start + intervals[i].end) / 2;
        bars[i]->setData({tick}, {intervals[i].msgCount() + 0.});
        bars[i]->setName(QString::number(tick));
    }
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");

    QSharedPointer<QCPAxisTickerFixed> fixedYTicker(new QCPAxisTickerFixed);
    customPlot->yAxis->setTicker(fixedYTicker);
    fixedYTicker->setTickStep(1.0);
    fixedYTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

    QSharedPointer<QCPAxisTickerFixed> fixedXTicker(new QCPAxisTickerFixed);
    customPlot->xAxis->setTicker(fixedXTicker);
    fixedXTicker->setTickStep(intervals[0].length());
    fixedXTicker->setTickOrigin(intervals[0].start);
    fixedXTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

    customPlot->rescaleAxes();

    customPlot->setInteractions(QCP::iRangeDrag |
                                QCP::iRangeZoom |
                                QCP::iSelectPlottables |
                                QCP::iSelectLegend |
                                QCP::iMultiSelect);
}

void Histogram::getData() {
    MsgNumbersMap plotData;
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        if(bar->selected()) {
            unsigned long long intervalIndex = int((bar->data()->at(0)->mainKey() - allData.start) / bar->width());
            auto data = intervals[intervalIndex].getIntervalData();
            for(auto& src : data) {
                for(auto& msgnum : src.second) {
                    plotData[src.first].insert(msgnum);
                }
            }
        }
    }
    emit dataSignal(plotData);
}
