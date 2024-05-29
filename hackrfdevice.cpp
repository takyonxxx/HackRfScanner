#include "hackrfdevice.h"


static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData) {
    float *in = (float *)inputBuffer;
    std::vector<float> *audioData = (std::vector<float> *)userData;
    audioData->insert(audioData->end(), in, in + framesPerBuffer);
    return paContinue;
}

HackRfDevice::HackRfDevice(QObject *parent)
    : QObject(parent)
{
    if (!setupPortAudio(&stream, audioData)) {
        qDebug() << "Failed to setup PortAudio";
        return;
    }

    if (hackrf_init() != HACKRF_SUCCESS) {
        qDebug() << "can not init hackrf";
        return;
    }

    if (hackrf_open(&m_device) != HACKRF_SUCCESS) {
        qDebug() << "can not open hackrf device";
        return;
    }

    if (hackrf_set_antenna_enable(m_device, 1) != HACKRF_SUCCESS) {
        qDebug() << "can not set antenna";
        return;
    }

    if (hackrf_set_amp_enable(m_device, true) != HACKRF_SUCCESS) {
        qDebug() << "can not set amp";
        return;
    }

    if (hackrf_set_txvga_gain(m_device, 29) != HACKRF_SUCCESS) {
        qDebug() << "can not set amp tx";
        return;
    }


//    if (hackrf_start_tx(m_device, tx_callbackStream, this) != HACKRF_SUCCESS) {
//        qDebug() << "can not start tx stream";
//        return;
//    }

    if (hackrf_start_tx(m_device, &HackRfDevice::tx_callbackStream, this) != HACKRF_SUCCESS) {
        qDebug() << "Failed to start transmission.";
        return;
    }

    sensitivity = (2.0 * M_PI * (5e3 / DEFAULT_AUDIO_SAMPLE_RATE));

    set_sample_rate(DEFAULT_SAMPLE_RATE);
    set_frequency(DEFAULT_FREQUENCY);
}

HackRfDevice::~HackRfDevice()
{
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    hackrf_stop_tx(m_device);
    hackrf_close(m_device);
    hackrf_exit();
}

int HackRfDevice::tx_callbackStream(hackrf_transfer *transfer)
{
//    static std::vector<float> buffer;
//    buffer.clear();
//    {
//        std::lock_guard<std::mutex> lock(audioDataMutex);
//        buffer = audioData; // Copy audio data
//        audioData.clear();  // Clear the original buffer
//    }

//    applyPreEmphasis(buffer, preEmphasisTaps); // Apply pre-emphasis filter
//    fmModulate(buffer, sensitivity);          // Apply FM modulation

//    size_t idx = 0;
//    for (int i = 0; i < transfer->valid_length; i++) {
//        transfer->buffer[i] = buffer[idx++] * 127; // Convert to int8 and fill transfer buffer
//        if (idx >= buffer.size()) {
//            idx = 0; // Loop the audio data
//        }
//    }
    return 0;
}


void HackRfDevice::set_frequency(uint64_t freq)
{
    if (hackrf_set_freq(m_device, freq) != HACKRF_SUCCESS) {
        throw std::runtime_error("can not set frequency");
    }
    else
        centerFrequency = freq;
}

bool HackRfDevice::force_sample_rate(double fs_hz)
{
    auto baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(uint32_t(fs_hz));
    qDebug() << "HackRfDevice: Setting filter to " << baseband_filter_bw_hz;
    hackrf_set_baseband_filter_bandwidth(m_device, baseband_filter_bw_hz);

    qDebug() << "HackRfDevice: Setting sample rate to " << fs_hz;
    if (hackrf_set_sample_rate(m_device, fs_hz) != HACKRF_SUCCESS) {
        throw std::runtime_error("can not set sample rate");
    }
    else
    {
        return true;
    }
    return false;
}

bool HackRfDevice::setupPortAudio(PaStream **stream, std::vector<float> &audioData)
{
    PaError err = Pa_Initialize();
    if (err != paNoError) return false;

    err = Pa_OpenDefaultStream(stream, 1, 0, paFloat32, DEFAULT_AUDIO_SAMPLE_RATE, FRAMES_PER_BUFFER, paCallback, &audioData);
    if (err != paNoError) return false;

    err = Pa_StartStream(*stream);
    if (err != paNoError) return false;

    return true;
}

void HackRfDevice::applyPreEmphasis(std::vector<float> &audioData, const std::vector<float> &taps)
{
    std::vector<float> filtered(audioData.size(), 0);
    for (size_t i = 0; i < audioData.size(); ++i) {
        for (size_t j = 0; j < taps.size(); ++j) {
            if (i >= j) {
                filtered[i] += taps[j] * audioData[i - j];
            }
        }
    }
    audioData = filtered;
}

void HackRfDevice::fmModulate(std::vector<float> &audioData, float sensitivity)
{
    std::vector<float> modulated(audioData.size(), 0);
    float phase = 0;
    for (size_t i = 0; i < audioData.size(); ++i) {
        phase += sensitivity * audioData[i];
        modulated[i] = cos(phase);
    }
    audioData = modulated;
}

void HackRfDevice::set_sample_rate(uint64_t srate)
{
    if (!force_sample_rate(srate)) {
        throw std::runtime_error("can not set sample rate");
    }
    else
        sampleRate = srate;
}

