#include "fftplotter.h"
#include <QPainter>
#include <QPen>
#include <QDebug>
#include <algorithm>

FFTPlotter::FFTPlotter(QWidget *parent)
    : QFrame(parent), m_centerFrequency(0.0), m_sampleRate(0.0),
    m_textColor(Qt::white), m_drawColor(Qt::darkCyan), m_backgroundColor(Qt::black) // Initial colors
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
}

void FFTPlotter::setNewFftData(const float *data, int size)
{
    m_fftData.resize(size);
    for (int i = 0; i < size; ++i)
    {
        m_fftData[i] = data[i];
    }
    applyFilter();  // Apply filter before updating the plot
    update();
}

void FFTPlotter::setCenterFrequency(double freq)
{
    m_centerFrequency = freq;
    update();
}

void FFTPlotter::setSampleRate(double rate)
{
    m_sampleRate = rate;
    update();
}

void FFTPlotter::setTextColor(const QColor& color)
{
    m_textColor = color;
    update();
}

void FFTPlotter::setDrawColor(const QColor& color)
{
    m_drawColor = color;
    update();
}

void FFTPlotter::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
    update();
}

void FFTPlotter::applyFilter()
{
    // Implement a simple moving average filter
    QVector<float> filteredData(m_fftData.size());
    int filterSize = 5; // Adjust the filter size as needed
    for (int i = 0; i < m_fftData.size(); ++i)
    {
        float sum = 0.0;
        int count = 0;
        for (int j = -filterSize; j <= filterSize; ++j)
        {
            int index = i + j;
            if (index >= 0 && index < m_fftData.size())
            {
                sum += m_fftData[index];
                count++;
            }
        }
        filteredData[i] = sum / count;
    }
    m_fftData = filteredData;
}

void FFTPlotter::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    if (m_fftData.isEmpty() || m_sampleRate == 0)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = this->width();
    int height = this->height();
    int size = m_fftData.size();
    int padding = 20; // Padding for text

    // Fill background
    painter.fillRect(rect(), m_backgroundColor);

    // Draw gridlines
    painter.setPen(QPen(Qt::gray, 0.5));
    for (int i = 0; i <= 10; ++i)
    {
        int y = i * (height - padding * 2) / 10 + padding;
        painter.drawLine(0, y, width, y);
    }
    for (int i = 0; i <= 10; ++i)
    {
        int x = i * (width - padding * 2) / 10 + padding;
        painter.drawLine(x, 0, x, height);
    }

    // Draw frequency labels on x-axis gridlines
    painter.setPen(QPen(m_textColor, 1));
    QFont font = painter.font();
    font.setPointSize(8); // Adjust text size
    painter.setFont(font);

    double frequencyStep = m_sampleRate / 5e6; // Calculate frequency step in MHz
    double centerFreqMHz = m_centerFrequency / 1e6;
    double leftFreq = centerFreqMHz - (m_sampleRate / 2 / 1e6);
    double rightFreq = centerFreqMHz + (m_sampleRate / 2 / 1e6);

    for (int i = 0; i <= 10; ++i)
    {
        double freq = leftFreq + i * frequencyStep;
        int x = i * (width - padding * 2) / 5 + padding;
        painter.drawText(QRect(x - 25, height - padding, 50, 20), Qt::AlignCenter, QString::number(freq, 'f', 1) + " MHz");
    }

    // Draw red bar at center frequency
    painter.setPen(QPen(Qt::red, 2));
    int centerX = width / 2;
    painter.drawLine(centerX, padding, centerX, height - padding);

    // Draw center frequency at the red bar side
    painter.setPen(QPen(m_textColor, 1));
    painter.drawText(QRect(centerX - 50, height - padding - 20, 100, 20), Qt::AlignCenter, QString::number(centerFreqMHz, 'f', 2) + " MHz");

    // Scale the FFT data to fit within the frame
    float maxVal = *std::max_element(m_fftData.begin(), m_fftData.end());
    float minVal = *std::min_element(m_fftData.begin(), m_fftData.end());

    for (int i = 1; i < size; ++i)
    {
        float freq1 = static_cast<float>(i - 1) / (size - 1) * (m_sampleRate / 2);
        float freq2 = static_cast<float>(i) / (size - 1) * (m_sampleRate / 2);

        float x1 = (freq1 / (m_sampleRate / 2)) * (width - padding * 2) + padding;
        float y1 = (m_fftData[i - 1] - minVal) / (maxVal - minVal) * (height - padding * 2) + padding;
        float x2 = (freq2 / (m_sampleRate / 2)) * (width - padding * 2) + padding;
        float y2 = (m_fftData[i] - minVal) / (maxVal - minVal) * (height - padding * 2) + padding;

        // Set draw color
        painter.setPen(QPen(m_drawColor, 1)); // Updated draw color
        painter.drawLine(QPointF(x1, height - y1), QPointF(x2, height - y2));
    }

    // Draw dB level markings as a bar on the left side with padding
    painter.setPen(QPen(m_textColor, 1)); // Updated text color
    for (int i = 0; i <= 10; ++i)
    {
        int y = i * (height - padding * 2) / 10 + padding;
        int dB = -100 + i * 10;
        painter.drawText(QRect(padding, height - y - 10, 50, 20), Qt::AlignLeft | Qt::AlignVCenter, QString::number(dB));
    }

    // Draw sample rate with padding
    painter.drawText(QRect(padding + 50, padding + 20, 200, 20), Qt::AlignLeft | Qt::AlignTop, "Sample Rate: " + QString::number(m_sampleRate / 1e6, 'f', 2) + " MHz");
}

