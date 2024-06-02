#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <fftw3.h>
#include "sdrdevice.h"
#include "fftplotter.h"
#include "cplotter.h"
#include "circular_buffer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    typedef std::complex<float> Complex;
    typedef std::vector<Complex> CArray;

    // FFT function
    void fft(CArray &x) {
        const size_t N = x.size();
        if (N <= 1) return;

        // divide
        CArray even = CArray(N / 2);
        CArray odd = CArray(N / 2);
        for (size_t i = 0; i < N / 2; ++i) {
            even[i] = x[i * 2];
            odd[i] = x[i * 2 + 1];
        }

        // conquer
        fft(even);
        fft(odd);

        // combine
        for (size_t k = 0; k < N / 2; ++k) {
            Complex t = std::polar<float>(1.0f, -2.0f * M_PI * k / N) * odd[k];
            x[k] = even[k] + t;
            x[k + N / 2] = even[k] - t;
        }
    }

    void resampleData(const std::vector<float>& in, std::vector<float>& out, int targetSize)
    {
        int originalSize = in.size();
        out.resize(targetSize);

        for (int i = 0; i < targetSize; ++i)
        {
            float index = static_cast<float>(i) * (originalSize - 1) / (targetSize - 1);
            int lowIndex = static_cast<int>(std::floor(index));
            int highIndex = static_cast<int>(std::ceil(index));
            float weight = index - lowIndex;

            if (lowIndex == highIndex)
            {
                out[i] = in[lowIndex];
            }
            else
            {
                out[i] = (1 - weight) * in[lowIndex] + weight * in[highIndex];
            }
        }
    }


    void processRxBuffer(const float* in, int size, float* fft_output, float& sum_signal_power, int& num_iterations) {
        int fft_size = size / 2;  // Since the input is IQ, we divide by 2 for FFT size

        // Allocate array for FFT
        CArray data(fft_size);

        // Convert interleaved IQ data to std::complex format
        for (int i = 0; i < fft_size; i++) {
            data[i] = Complex(in[2 * i], in[2 * i + 1]);
        }

        // Execute FFT
        fft(data);

        float maxPower = 0.0f;
        for (int i = 0; i < fft_size; i++) {
            maxPower = std::max(maxPower, norm(data[i]));
        }
        for (int i = 0; i < fft_size; i++) {
            fft_output[i] = 1.0 * log10(norm(data[i]) / maxPower + 1.0e-20f);
        }

        float amplificationFactor = 20.0f; // Example amplification factor
        for (int i = 0; i < fft_size; i++) {
            fft_output[i] *= amplificationFactor;
        }

        // Calculate signal power
        float sum_power = 0.0f;
        for (int i = 0; i < fft_size; i++) {
            sum_power += norm(data[i]);
        }
        sum_signal_power += sum_power;
        num_iterations++;
    }

private slots:
    void infoFrequency(int f);
    void loadSettings();
    void saveSettings();
    void onFreqCtrl_setFrequency(qint64 freq);
    void on_plotter_newFilterFreq(int low, int high);
    void on_plotter_newDemodFreq(qint64 , qint64 );
    void fetchFFtData();

    void on_pushToggleSdr_clicked();
    void on_pushExit_clicked();   
    void on_m_pIncFreq_clicked();
    void on_m_pDecFreq_clicked();
    void on_m_cFreqType_currentIndexChanged(int index);
    void on_m_cDemod_currentIndexChanged(int index);    
    void on_m_pBPtt_clicked();
    void getRxBuffer(const float* in, int size);
private:

    int m_LowCutFreq = -120e3;
    int m_HiCutFreq = 120e3;
    int flo = -5000;
    int fhi = 5000;
    int click_res = 100;
    int fftrate = 25;

    FFTPlotter *fftPlotter;
    CPlotter *cPlotter;
    std::vector<float> d_iqFftData;   
    float               d_fftAvg;

    SdrDevice *sdrDevice{};
    FreqMod currentFreqMod;
    Demod currentDemod;
    double currentFrequency;
    double sampleRate;
    bool m_ptt;
    bool m_stop;

    QString m_sSettingsFile;
    int freq_type_index;
    int demod_index;

    CircularBuffer circular_buffer_;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
