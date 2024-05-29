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

#define DEFAULT_FREQUENCY              _MHZ(144)
#define DEFAULT_SAMPLE_RATE            _MHZ(2)
#define DEFAULT_AUDIO_SAMPLE_RATE      _KHZ(44.1)
#define DEFAULT_CUT_OFF                _KHZ(300)
#define HACKRF_TX_VGA_MAX_DB            47.0
#define HACKRF_RX_VGA_MAX_DB            40.0
#define HACKRF_RX_LNA_MAX_DB            40.0
#define HACKRF_AMP_MAX_DB               14.0
#define FRAMES_PER_BUFFER               512

class HackRfDevice: public QObject
{
    Q_OBJECT
public:
    explicit HackRfDevice(QObject *parent = nullptr);
    ~HackRfDevice();

    hackrf_device* m_device;

private:
    static int tx_callbackStream(hackrf_transfer* transfer);
    static int tx_callbackAdapter(hackrf_transfer *transfer, void *userData);
    void set_frequency(uint64_t freq);
    void set_sample_rate(uint64_t srate);
    bool force_sample_rate(double fs_hz);
    bool setupPortAudio(PaStream **stream, std::vector<float> &audioData);
    void applyPreEmphasis(std::vector<float> &audioData, const std::vector<float> &taps);
    void fmModulate(std::vector<float> &audioData, float sensitivity);

    int centerFrequency;
    int sampleRate;
    int audioSampleRate;
    float sensitivity;

    PaStream *stream;
    std::vector<float> audioData;
    std::vector<float> preEmphasisTaps = { /* your coefficients here */ };
    std::mutex audioDataMutex; // Mutex to protect audioData
};

#endif // HACKRFDEVICE_H
