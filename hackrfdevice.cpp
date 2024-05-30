#include "hackrfdevice.h"


static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData) {
    float *in = (float *)inputBuffer;
    std::vector<float> *audioData = (std::vector<float> *)userData;
    audioData->insert(audioData->end(), in, in + framesPerBuffer);

//    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
//        float sample = in[i]; // Access the i-th sample
//        qDebug() << "Audio Data: " << sample;
//    }

    return paContinue;
}


HackRfDevice::HackRfDevice(QObject *parent)
    : QObject(parent)
{
    if (!setupPortAudio(audioData)) {
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

    if (hackrf_start_tx(m_device, tx_callbackStream, (void *)this) != HACKRF_SUCCESS) {
        qDebug() << "Failed to start transmission.";
        return;
    }

    sensitivity = (2.0 * M_PI * (5e3 / DEFAULT_AUDIO_SAMPLE_RATE));

    set_sample_rate(DEFAULT_SAMPLE_RATE);
    set_frequency(DEFAULT_FREQUENCY);
}

HackRfDevice::~HackRfDevice()
{
    qDebug() << "Exiting...";
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    hackrf_stop_tx(m_device);
    hackrf_close(m_device);
    hackrf_exit();
}

int HackRfDevice::tx_callbackStream(hackrf_transfer *transfer)
{
    HackRfDevice *obj = (HackRfDevice *)transfer->tx_ctx;
    return obj->tx_callback((int8_t *)transfer->buffer, transfer->valid_length);
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

bool HackRfDevice::setupPortAudio(std::vector<float> &audioData)
{
    PaError err = Pa_Initialize();
    if (err != paNoError) return false;

    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        std::cerr << "Error: No default input device found!" << std::endl;
        Pa_Terminate();
        return false;
    }
    inputParameters.channelCount = 1; // Mono input
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, DEFAULT_AUDIO_SAMPLE_RATE, FRAMES_PER_BUFFER, paCallback, &audioData);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) return false;

    return true;
}

void HackRfDevice::set_sample_rate(uint64_t srate)
{
    if (!force_sample_rate(srate)) {
        throw std::runtime_error("can not set sample rate");
    }
    else
        sampleRate = srate;
}

const float CARRIER_FREQUENCY = 100000.0f;
const float MODULATION_INDEX = 5.0f;
const float SIGNAL_MULTIPLIER = 5.0f;
const float AUDIO_SAMPLE_RATE = 44100.0f;
const float TEST_FREQUENCY = 1000.0f;

void HackRfDevice::apply_modulation(int8_t* buffer, uint32_t length)
{
    double modulationIndex = 5.0;
    double amplitudeScalingFactor = 2.0;
    double cutoffFreq = 150.0;
    double hackrf_sample_rate = sampleRate;
    double newSampleRate = 2 * sampleRate;
    double resampleRatio = sampleRate / newSampleRate;

    LowPassFilter filter(hackrf_sample_rate, cutoffFreq);

    static std::vector<float> mic_buffer;
    mic_buffer.clear();

    {
        std::lock_guard<std::mutex> lock(audioDataMutex);
        mic_buffer = audioData;  // Copy audio data to mic_buffer
        audioData.clear();       // Clear the original audioData buffer
    }

    // If mic_buffer is empty, fill the output buffer with zeros and return
    if (mic_buffer.empty()) {
        memset(buffer, 0, length);
        return;
    }

    // If mic_buffer is smaller than half the length, repeat it until it matches or exceeds the length
    while (mic_buffer.size() < length / 2) {
        // Calculate the remaining space to fill in mic_buffer
        size_t remainingSpace = (length / 2) - mic_buffer.size();

        // Calculate how many samples to copy from the beginning of mic_buffer to fill the remaining space
        size_t samplesToCopy = std::min(remainingSpace, mic_buffer.size());

        // Append the required samples from the beginning of mic_buffer to fill the remaining space
        mic_buffer.insert(mic_buffer.end(), mic_buffer.begin(), mic_buffer.begin() + samplesToCopy);
    }

//    // Debug output: Print each sample in mic_buffer
//    for (float sample : mic_buffer) {
//        qDebug() << sample;
//    }


    for (uint32_t sampleIndex = 0; sampleIndex < length; sampleIndex += 2) {
        // Calculate time
        double time = (current_tx_sample + sampleIndex / 2) / hackrf_sample_rate;
        double interpolatedTime = time * resampleRatio;
        double audioSignal = sin(2 * M_PI * 440 * time);
//        double audioSignal = mic_buffer[sampleIndex / 2];
        double filteredAudioSignal = filter.filter(audioSignal);
        double modulatedPhase = 2 * M_PI * interpolatedTime + modulationIndex * filteredAudioSignal;
        double inPhaseComponent = cos(modulatedPhase) * amplitudeScalingFactor;
        double quadratureComponent = sin(modulatedPhase) * amplitudeScalingFactor;
        buffer[sampleIndex] = static_cast<int8_t>(std::clamp(inPhaseComponent * 127, -127.0, 127.0));
        buffer[sampleIndex + 1] = static_cast<int8_t>(std::clamp(quadratureComponent * 127, -127.0, 127.0));
    }

    current_tx_sample += length / 2;
}

int HackRfDevice::tx_callback(int8_t *buffer, uint32_t length) {
    apply_modulation(buffer, length);
    return 0;
}
