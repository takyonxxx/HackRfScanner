#include "sdrdevice.h"

SdrDevice::SdrDevice(QObject *parent):
    QObject(parent)
{
    m_ptt                  = false;
    currentDemod    = Demod::DEMOD_WFM;
    currentFreqMod  = FreqMod::MHZ;

    sample_rate            = DEFAULT_SAMPLE_RATE;
    audio_samp_rate        = DEFAULT_AUDIO_SAMPLE_RATE;
    currentFrequency       = DEFAULT_FREQUENCY;
    transition             = DEFAULT_CUT_OFF;
    audio_gain             = DEFAULT_AUDIO_GAIN;
    cut_off                = DEFAULT_CUT_OFF;
    decimation             = 6;
    interpolation          = 1;
    resampler_decimation   = 70;

    currentReceiverMode = ReceiverMode::RX;
    customBuffer = std::make_shared<CustomBuffer>("custom_buffer");
    connect(customBuffer.get(), &CustomBuffer::send_fft, this, &SdrDevice::get_fft);

    try {
        std::string dev = "hackrf=0";
        std::string stream_args = "";
        std::vector<std::string> tune_args = {""};
        std::vector<std::string> settings = {""};

        if (currentReceiverMode == ReceiverMode::TX)
        {
            hackrf_soapy_sink = gr::soapy::sink::make(
                "hackrf",
                "fc32",
                1,
                dev,
                stream_args,
                tune_args,
                settings
                );

            if (!hackrf_soapy_sink) {
                throw std::runtime_error("Failed to create SoapySDR sink.");
            }

            hackrf_soapy_sink->set_sample_rate(1, sample_rate);
            hackrf_soapy_sink->set_bandwidth(1, 0);
            hackrf_soapy_sink->set_frequency(1, currentFrequency);
            hackrf_soapy_sink->set_gain(1, "AMP", true);
            hackrf_soapy_sink->set_gain(0, "VGA", std::min(std::max(16.0, 0.0), HACKRF_TX_VGA_MAX_DB));

            // Print device information
            qDebug() << "Center Frequency: " << hackrf_soapy_sink->get_frequency(0) << " Hz";
            qDebug() << "Sample Rate: " << hackrf_soapy_sink->get_sample_rate(0) << " Hz";
            qDebug() << "Actual TX Gain: " << hackrf_soapy_sink->get_gain(0) << " dB...";
        }
        else
        {
            hackrf_soapy_source = gr::soapy::source::make(
                "hackrf",
                "fc32",
                1,
                dev,
                stream_args,
                tune_args,
                settings
                );

            if (!hackrf_soapy_source) {
                throw std::runtime_error("Failed to create SoapySDR source.");
            }

            hackrf_soapy_source->set_sample_rate(0, sample_rate);
            hackrf_soapy_source->set_bandwidth(0, 0);
            hackrf_soapy_source->set_frequency(0, currentFrequency);
            hackrf_soapy_source->set_gain(0, "AMP", false);
            hackrf_soapy_source->set_gain(0, "LNA", std::min(std::max(40.0, 0.0), HACKRF_RX_LNA_MAX_DB));
            hackrf_soapy_source->set_gain(0, "VGA", std::min(std::max(40.0, 0.0), HACKRF_RX_VGA_MAX_DB));


            // Print device information
            qDebug() << "Center Frequency: " << hackrf_soapy_source->get_frequency(0) << " Hz";
            qDebug() << "Sample Rate: " << hackrf_soapy_source->get_sample_rate(0) << " Hz";
            qDebug() << "Actual RX Gain: " << hackrf_soapy_source->get_gain(0) << " dB...";
            qDebug() << "LNA Gain: " << hackrf_soapy_source->get_gain(0, "LNA") << " dB";
            qDebug() << "VGA Gain: " << hackrf_soapy_source->get_gain(0, "VGA") << " dB";
        }

    } catch (const std::exception &e) {
        qDebug() << "Source Error: " << e.what();
    }

    setMode(currentReceiverMode);

    tb = gr::make_top_block("HackRf");   
}

SdrDevice::~SdrDevice()
{
}

void SdrDevice::setFrequency(double frequency)
{
    hackrf_soapy_source->set_frequency(0, frequency);
    currentFrequency = getCenterFrequency();
    emit infoFrequency(currentFrequency);
}

double SdrDevice::getCenterFrequency() const
{
    return hackrf_soapy_source->get_frequency(0);
}

void SdrDevice::setMode(ReceiverMode rMode)
{
    tb->disconnect_all();
    gr::blocks::null_sink::sptr null_sink = gr::blocks::null_sink::make(sizeof(gr_complex) / 2);

    if (rMode == ReceiverMode::TX) {

        // Add microphone source block
        auto audio_source = gr::audio::source::make(audio_samp_rate, "MacBook Pro Microphone", true);

        gr::filter::rational_resampler_ccf::sptr resampler_tx = gr::filter::rational_resampler_ccf::make(50, 12);

        //        self.connect((self.analog_wfm_tx_0, 0), (self.rational_resampler_xxx_0, 0))
        //        self.connect((self.blocks_wavfile_source_0, 0), (self.analog_wfm_tx_0, 0))
        //        self.connect((self.rational_resampler_xxx_0, 0), (self.soapy_hackrf_sink_0, 0))
        //  auto blocks_wavfile_source_0 = gr::blocks::wavfile_source::make("/Users/turkaybiliyor/Desktop/HackRfScanner/440Hz_44100Hz_16bit_05sec.wav", true);

        //       // FM modulator block
        auto fm_mod = gr::analog::frequency_modulator_fc::make(1.0);

        tb->connect(audio_source, 0, fm_mod, 0);
        tb->connect(fm_mod, 0, resampler_tx, 0);
        tb->connect(resampler_tx, 0, hackrf_soapy_sink, 0);

        qDebug() << "Switched to TX mode.";

    } else if (rMode == ReceiverMode::RX)
    {
        gr::filter::rational_resampler_ccf::sptr resampler_rx = gr::filter::rational_resampler_ccf::make(interpolation, resampler_decimation);

        auto low_pass_filter = gr::filter::fir_filter_fff::make(
            decimation,
            gr::filter::firdes::low_pass(1, sample_rate, cut_off, transition, gr::fft::window::WIN_HAMMING));

        gr::analog::quadrature_demod_cf::sptr quad_demod = gr::analog::quadrature_demod_cf::make(1.0);

        auto audio_sink = gr::audio::sink::make(audio_samp_rate, "", true);

        tb->connect(hackrf_soapy_source, 0, resampler_rx, 0);
        tb->connect(hackrf_soapy_source, 0, customBuffer, 0);
        tb->connect(resampler_rx, 0, quad_demod, 0);
        tb->connect(quad_demod, 0, low_pass_filter, 0);
        tb->connect(low_pass_filter, 0, audio_sink, 0);

        qDebug() << "Switched to RX mode.";
    }
}

void SdrDevice::start()
{
    tb->start();
}

void SdrDevice::stop()
{
    tb->stop();
}

void SdrDevice::get_fft(QVector<std::complex<float>> data)
{
    emit send_fft(data);
}
