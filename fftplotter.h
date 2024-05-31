#ifndef FFTPLOTTER_H
#define FFTPLOTTER_H

#include <QFrame>
#include <QColor>
#include <QVector>

class FFTPlotter : public QFrame
{
    Q_OBJECT

public:
    explicit FFTPlotter(QWidget *parent = nullptr);

    void setNewFftData(const float *data, int size);
    void setCenterFrequency(double freq);
    void setSampleRate(double rate);
    void setTextColor(const QColor& color);
    void setDrawColor(const QColor& color);
    void setBackgroundColor(const QColor& color); // New function to set background color

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void applyFilter(); // New function to apply filter

    QVector<float> m_fftData;
    double m_centerFrequency;
    double m_sampleRate;
    QColor m_textColor;
    QColor m_drawColor;
    QColor m_backgroundColor; // Member variable to store background color
};

#endif // FFTPLOTTER_H
