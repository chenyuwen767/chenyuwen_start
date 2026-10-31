#pragma once

#include <QWidget>

#include <array>

class WeeklyChart : public QWidget {
public:
    explicit WeeklyChart(QWidget* parent = nullptr);

    void setValues(const std::array<int, 8>& values);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::array<int, 8> values_{};
};
