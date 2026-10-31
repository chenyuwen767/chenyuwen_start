#include "weekly_chart.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>

#include <algorithm>

WeeklyChart::WeeklyChart(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(170);
}

void WeeklyChart::setValues(const std::array<int, 8>& values) {
    values_ = values;
    update();
}

QSize WeeklyChart::sizeHint() const {
    return {520, 190};
}

void WeeklyChart::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF plot = rect().adjusted(34, 18, -16, -30);
    if (plot.width() <= 0 || plot.height() <= 0) {
        return;
    }

    const int maxValue = std::max(1, *std::max_element(values_.begin(), values_.end()));
    const qreal slot = plot.width() / 8.0;
    const qreal barWidth = std::min<qreal>(34.0, slot * 0.52);
    const qreal baseline = plot.bottom();

    painter.setPen(QPen(QColor("#e3eaf4"), 1));
    painter.drawLine(QPointF(plot.left(), baseline), QPointF(plot.right(), baseline));

    QFont labelFont = font();
    labelFont.setPointSizeF(std::max(8.0, labelFont.pointSizeF() - 1.5));
    painter.setFont(labelFont);

    for (int index = 0; index < 8; ++index) {
        const int value = values_[static_cast<std::size_t>(index)];
        const qreal center = plot.left() + slot * (index + 0.5);
        const qreal height = plot.height() * 0.82 * value / maxValue;
        const QRectF bar(center - barWidth / 2.0, baseline - height, barWidth, height);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(value == 0 ? "#dbe6f5" : "#4d91e7"));
        painter.drawRoundedRect(bar, 4, 4);

        painter.setPen(QColor("#263a57"));
        const QString valueText = QString::number(value);
        painter.drawText(QRectF(center - slot / 2.0, baseline - height - 22, slot, 18),
                         Qt::AlignHCenter | Qt::AlignBottom,
                         valueText);

        painter.setPen(QColor("#73829a"));
        const QString weekText = index == 7 ? QString("本周") : QString("%1周前").arg(7 - index);
        painter.drawText(QRectF(center - slot / 2.0, baseline + 6, slot, 18),
                         Qt::AlignHCenter | Qt::AlignTop,
                         weekText);
    }
}
