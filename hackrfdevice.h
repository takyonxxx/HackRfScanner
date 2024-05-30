#ifndef HACKRFDEVICE_H
#define HACKRFDEVICE_H

#include <QObject>
#include <QDebug>
#include <iostream>
#include <vector>
#include <cmath>
#include <libhackrf/hackrf.h>
#include <portaudio.h>
#include <mutex>

#define _GHZ(x) ((uint64_t)(x) * 1000000000)
#define _MHZ(x) ((x) * 1000000)
#define _KHZ(x) ((x) * 1000)
#define _HZ(x) ((x) * 1)

#define DEFAULT_FREQUENCY              _MHZ(144.5)
#define DEFAULT_SAMPLE_RATE            _MHZ(2)
#define DEFAULT_AUDIO_SAMPLE_RATE      _KHZ(44.1)
#define DEFAULT_CUT_OFF                _KHZ(300)
#define HACKRF_TX_VGA_MAX_DB            47.0
#define HACKRF_RX_VGA_MAX_DB            40.0
#define HACKRF_RX_LNA_MAX_DB            40.0
#define HACKRF_AMP_MAX_DB               14.0
#define FRAMES_PER_BUFFER               512

class LowPassFilter {
private:
    double alpha;
    double y_prev;

public:
    LowPassFilter(double sampleRate, double cutoffFreq) {
        double dt = 1.0 / sampleRate;
        double RC = 1.0 / (2 * M_PI * cutoffFreq);
        alpha = dt / (RC + dt);
        y_prev = 0.0;
    }

    double filter(double x) {
        double y = alpha * x + (1 - alpha) * y_prev;
        y_prev = y;
        return y;
    }
};

class HackRfDevice: public QObject
{
    Q_OBJECT
public:
    explicit HackRfDevice(QObject *parent = nullptr);
    ~HackRfDevice();

    hackrf_device* m_device;

private:
    static int tx_callbackStream(hackrf_transfer* transfer);
    int tx_callback(int8_t* buffer, uint32_t length);
    void set_frequency(uint64_t freq);
    void set_sample_rate(uint64_t srate);
    bool force_sample_rate(double fs_hz);
    bool setupPortAudio(std::vector<float> &audioData);
    void apply_modulation(int8_t* buffer, uint32_t length);

    int centerFrequency;
    int sampleRate;
    int audioSampleRate;
    float sensitivity;
    int current_tx_sample = 0;

    PaStream *stream;
    std::vector<float> audioData;
    std::vector<float> preEmphasisTaps = { /* your coefficients here */ };
    std::mutex audioDataMutex; // Mutex to protect audioData
};

#endif // HACKRFDEVICE_H
