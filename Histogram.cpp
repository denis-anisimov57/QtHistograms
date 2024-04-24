#include "Histogram.h"
#include <QSignalSpy>
#include <QMenu>

hst::Interval::Interval(double start, double end) {
    if(start > end) {
        double tmp = end;
        end = start;
        start = tmp;
    }
    this->start = start;
    this->end = end;
}

bool hst::Interval::inInterval(double val) const {
    return ((val >= start) && (val < end));
}

bool hst::Interval::addMsg(Message msg, int sourcenum) {
    if(inInterval(msg.value)) {
        if(!msgnumbers.count(sourcenum)) {
            msgnumbers[sourcenum] = std::set<int>();
        }
        if(msgnumbers[sourcenum].count(msg.msgnum)) {
            qDebug() << "Interval[" << start << ", " << end << "]: Trying to add msgnum = " << msg.msgnum << " which already exists\n";
        }
        else {
            msgnumbers[sourcenum].insert(msg.msgnum);
        }
        return true;
    }
    return false;
}

double hst::Interval::getStart() const {
    return start;
}

double hst::Interval::getEnd() const {
    return end;
}

double hst::Interval::length() const {
    return end - start;
}

unsigned long long hst::Interval::msgCount() const {
    unsigned long long count = 0;
    for(auto it = msgnumbers.begin(); it != msgnumbers.end(); it++) {
        count += it->second.size();
    }
    return count;
}

MsgNumbersMap hst::Interval::getIntervalData() const {
    return msgnumbers;
}

hst::Histogram::Histogram(QCustomPlot* customPlot, QStatusBar* statusbar) {
    if(!customPlot) {
        throw std::runtime_error("Initialization Histogram with nullptr");
    }
    statusLabel = new QLabel();
    if(statusbar) {
        statusbar->insertWidget(0, statusLabel);
    }
    this->customPlot = customPlot;
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(hst::legendRowCount);
    allData = AllPlotInfo();
    intervals = std::vector<Interval>();

    connect(customPlot, &QCustomPlot::selectionChangedByUser, this, &hst::Histogram::selectionChanged);
    customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(customPlot, &QCustomPlot::customContextMenuRequested, this, &hst::Histogram::showMenu);
}

hst::Histogram::~Histogram() {
    delete statusLabel;
};

void hst::Histogram::keyPressed(int key) {
    QCPAxis* x = customPlot->xAxis;
    QCPAxis* y = customPlot->yAxis;

    if(key == Qt::Key_Left || key == Qt::Key_A) {
        x->moveRange(-hst::moveDelta * x->range().size());
    }
    if(key == Qt::Key_Right || key == Qt::Key_D) {
        x->moveRange(hst::moveDelta * x->range().size());
    }
    if(key == Qt::Key_Up || key == Qt::Key_W) {
        y->moveRange(hst::moveDelta * y->range().size() * customPlot->width() / customPlot->height());
    }
    if(key == Qt::Key_Down || key == Qt::Key_S) {
        y->moveRange(-hst::moveDelta * y->range().size() * customPlot->width() / customPlot->height());
    }
    if(key == Qt::Key_Minus) {
        x->scaleRange(hst::scaleCoeff);
        y->scaleRange(hst::scaleCoeff);
    }
    if(key == Qt::Key_Plus) {
        x->scaleRange(1 / hst::scaleCoeff);
        y->scaleRange(1 / hst::scaleCoeff);
    }
    if(key == Qt::Key_0) {
        resetScale();
    }
    if(key == Qt::Key_R) {
        regenerateColors();
    }
    customPlot->replot();
}

void hst::Histogram::selectionChanged() {
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        QCPPlottableLegendItem* item = customPlot->legend->itemWithPlottable(bar);
        if(item->selected() || bar->selected()) {
            item->setSelected(true);
            bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
            if(customPlot->selectedPlottables().size() == 1) {
                selectedBars.clear();
            }
            if(!selectedBars.count(i)) {
                selectedBars.push_back(i);
            }
        }
    }
    if(customPlot->selectedPlottables().empty()) {
        selectedBars.clear();
    }
    else {
        QCPBars* lastSelectedBar = dynamic_cast<QCPBars*>(customPlot->plottable(selectedBars.last()));
        double width = lastSelectedBar->width();
        double tick = lastSelectedBar->data()->at(0)->mainKey();
        double start = tick - width / 2, end = tick + width / 2;
        QString height = QString::number(lastSelectedBar->data()->at(0)->mainValue(), 'f', hst::statusNumberAccuracy);
        QString status = "Height: ";
        status += height;
        status += "; Interval: [";
        status += QString::number(start, 'f', hst::statusNumberAccuracy);
        status += ", ";
        status += QString::number(end, 'f', hst::statusNumberAccuracy);
        status += "]; Width: ";
        status += QString::number(width, 'f' , hst::statusNumberAccuracy);
        statusLabel->setText(status);
    }
}

void hst::Histogram::resetScale() {
    customPlot->rescaleAxes();
    customPlot->xAxis->scaleRange(hst::standartScale);
    customPlot->yAxis->scaleRange(hst::standartScale);
    customPlot->replot();
}

void hst::Histogram::showMenu(const QPoint& pos) {
    QMenu contextMenu("Context menu", customPlot);
    QAction act1("Show table", customPlot);
    act1.setEnabled(false);
    QAction act2("Reset scale", customPlot);
    contextMenu.addAction(&act1);
    contextMenu.addAction(&act2);

    QSignalSpy spy(this, &hst::Histogram::dataSignal);

    connect(&act1, &QAction::triggered, this, &hst::Histogram::getData);
    connect(&act2, &QAction::triggered, this, &hst::Histogram::resetScale);
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        if(bar->selected()) {
            act1.setEnabled(true);
            break;
        }
    }
    contextMenu.exec(customPlot->mapToGlobal(pos));

    //testing recieved data from signal
    if(spy.count()) {
        MsgNumbersMap args = spy.takeFirst().at(0).value<MsgNumbersMap>();
        if(args.empty()) {
            qDebug() << "No data selected";
        }
        for(auto& src : args) {
            for(auto& msgnum : src.second) {
                qDebug() << "[source: " << src.first << ", msgnum: " << msgnum << "]";
            }
        }
        qDebug() << "\n";
    }
}

void hst::Histogram::calculateIntervals(const Plot plotData) {
    std::vector<Message> data = plotData.messages;
    double max = data[0].value;
    for(Message msg : data) {
        if(msg.value > max) {
            max = msg.value;
        }
    }
    if(intervals.empty() || max > intervals[intervals.size() - 1].getEnd()) {
        int intCount = (max - allData.start) / allData.interval + 1;
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
            intervals[i] = Interval(intervals[i - 1].getEnd(), intervals[i - 1].getEnd() + allData.interval);
        }
        for(unsigned long long j = 0; j < data.size(); j++) {
            intervals[i].addMsg(data[j], plotData.sourcenum);
        }
    }
}

void hst::Histogram::addPlot(const Plot plotData) {
    if(allData.interval <= 0) {
        throw(std::runtime_error("Invalid interval length"));
    }
    allData.plots.push_back(plotData);
    calculateIntervals(plotData);
    if(isDrawn) {
        drawHistogram();
        customPlot->replot();
    }
}

void hst::Histogram::setIntervals(const double start, const double interval) {
    if(interval <= 0) {
        throw(std::runtime_error("Invalid interval length"));
    }
    allData.start = start;
    allData.interval = interval;
    if(!allData.plots.empty()) {
        intervals.clear();
        for(auto& plotData : allData.plots) {
            calculateIntervals(plotData);
        }
        if(isDrawn) {
            drawHistogram();
        }
    }
}

void hst::Histogram::regenerateColors() {
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        QColor color;

        unsigned char R = qrand() % hst::rangeR + hst::startR, G = qrand() % hst::rangeG + hst::startG, B = qrand() % R;
        color.setRgb(R, G, B, hst::barAlpha);

        bar->setBrush(QBrush(color));
        color.setAlpha(hst::penAlpha);
        bar->setPen(QPen(QBrush(color), hst::penWidth));
//        QCPSelectionDecorator* dec;
//        dec->setPen(...)
//        dec->setBrush(...)
//        bar->setSelectionDecorator(dec);
    }
}

void hst::Histogram::drawHistogram() {
    customPlot->clearPlottables();
    customPlot->clearItems();
    std::vector<QCPBars*> bars(intervals.size());
    std::vector<int> ticks(intervals.size());
    std::vector<int> values(intervals.size());
    for(unsigned long long i = 0; i < intervals.size(); i++) {
        double tick = 0;
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);

        /*R from 128 to 255
        G from 128 to 178
        B from 0 to R*/
        QColor color;
        unsigned char R = qrand() % hst::rangeR + hst::startR, G = qrand() % hst::rangeG + hst::startG, B = qrand() % R;
        color.setRgb(R, G, B, hst::barAlpha);

        bars[i]->setBrush(QBrush(color));
        color.setAlpha(hst::penAlpha);
        bars[i]->setPen(QPen(QBrush(color), hst::penWidth));

        bars[i]->setWidth(intervals[i].length());
        tick = (intervals[i].getStart() + intervals[i].getEnd()) / 2;
        bars[i]->setData({tick}, {double(intervals[i].msgCount())});
        bars[i]->setName(QString::number(tick));
    }

    QSharedPointer<QCPAxisTickerFixed> fixedXTicker(new QCPAxisTickerFixed);
    customPlot->xAxis->setTicker(fixedXTicker);
    fixedXTicker->setTickStep(intervals[0].length());
    fixedXTicker->setTickOrigin(intervals[0].getStart());
    fixedXTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

    customPlot->rescaleAxes();
    customPlot->xAxis->scaleRange(hst::standartScale);
    customPlot->yAxis->scaleRange(hst::standartScale);

    if(!isDrawn) {
        isDrawn = true;

        customPlot->xAxis->setLabel("x");
        customPlot->yAxis->setLabel("y");

        QSharedPointer<QCPAxisTickerFixed> fixedYTicker(new QCPAxisTickerFixed);
        customPlot->yAxis->setTicker(fixedYTicker);
        fixedYTicker->setTickStep(hst::yTickStep);
        fixedYTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

        customPlot->setInteractions(QCP::iRangeDrag |
                                    QCP::iRangeZoom |
                                    QCP::iSelectPlottables |
                                    QCP::iSelectLegend |
                                    QCP::iMultiSelect);
    }
}

void hst::Histogram::getData() {
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
