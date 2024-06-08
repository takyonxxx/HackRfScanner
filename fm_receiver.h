#include <QtCore>
#include <gnuradio/top_block.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/soapy/source.h>
#include <gnuradio/soapy/sink.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include "constants.h"
#include "custombuffer.h"

class FmReceiver : public QObject, public gr::top_block
{
    Q_OBJECT

public:
    FmReceiver(int frequency): gr::top_block("Fm Receiver"),
        sample_rate(DEFAULT_SAMPLE_RATE), currentFrequency(frequency)
    {
        std::string dev = "hackrf=0";
        std::string stream_args = "";
        std::vector<std::string> tune_args = {""};
        std::vector<std::string> settings = {""};

        soapy_hackrf_source_0 = gr::soapy::source::make(
            "hackrf",
            "fc32",
            1,
            dev,
            stream_args,
            tune_args,
            settings
            );

        if (!soapy_hackrf_source_0) {
            throw std::runtime_error("Failed to create SoapySDR source.");
        }

        int interpolation = 1;
        double decimation = 7.5 * (DEFAULT_SAMPLE_RATE / _MHZ(2));
        int cut_off     = DEFAULT_CUT_OFF;
        int transition  = DEFAULT_CUT_OFF;
        int audio_samp_rate = DEFAULT_AUDIO_SAMPLE_RATE;

        soapy_hackrf_source_0->set_sample_rate(0, sample_rate);
        soapy_hackrf_source_0->set_frequency(0, frequency);
        soapy_hackrf_source_0->set_gain(0, "AMP", false);
        soapy_hackrf_source_0->set_gain(0, "LNA", std::min(std::max(40.0, 0.0), HACKRF_RX_LNA_MAX_DB));
        soapy_hackrf_source_0->set_gain(0, "VGA", std::min(std::max(40.0, 0.0), HACKRF_RX_VGA_MAX_DB));

        customBuffer = std::make_shared<CustomBuffer>("custom_buffer");
        rational_resampler_xxx_0 = gr::filter::rational_resampler_ccf::make(interpolation, decimation);
        blocks_multiply_const_vxx_0 = gr::blocks::multiply_const_ff::make(1 / 1.0);
        audio_sink_0 = gr::audio::sink::make(audio_samp_rate, "", true);
        analog_quadrature_demod_cf_0 = gr::analog::quadrature_demod_cf::make(1.0 / (2 * M_PI * 7500 / audio_samp_rate));
        low_pass_filter = gr::filter::fir_filter_fff::make(
            6,
            gr::filter::firdes::low_pass(1, sample_rate, cut_off, transition, gr::fft::window::WIN_HAMMING));

        gr::top_block::connect((const gr::block_sptr&)soapy_hackrf_source_0, 0, (const gr::block_sptr&)customBuffer, 0);
        gr::top_block::connect((const gr::block_sptr&)soapy_hackrf_source_0, 0, (const gr::block_sptr&)rational_resampler_xxx_0, 0);
        gr::top_block::connect((const gr::block_sptr&)rational_resampler_xxx_0, 0, (const gr::block_sptr&)analog_quadrature_demod_cf_0, 0);
        gr::top_block::connect((const gr::block_sptr&)analog_quadrature_demod_cf_0, 0, (const gr::block_sptr&)low_pass_filter, 0);       
        gr::top_block::connect((const gr::block_sptr&)low_pass_filter, 0, (const gr::block_sptr&)blocks_multiply_const_vxx_0, 0);
        gr::top_block::connect((const gr::block_sptr&)blocks_multiply_const_vxx_0, 0, (const gr::block_sptr&)audio_sink_0, 0);

        // Print device information
        // qDebug() << "Center Frequency: " << soapy_hackrf_source_0->get_frequency(0) << " Hz";
        // qDebug() << "Sample Rate: " << soapy_hackrf_source_0->get_sample_rate(0) << " Hz";
        // qDebug() << "Actual RX Gain: " << soapy_hackrf_source_0->get_gain(0) << " dB...";
        // qDebug() << "LNA Gain: " << soapy_hackrf_source_0->get_gain(0, "LNA") << " dB";
        // qDebug() << "VGA Gain: " << soapy_hackrf_source_0->get_gain(0, "VGA") << " dB";
    }

    virtual ~FmReceiver() override
    {
    }    

    int getSample_rate() const
    {
        return soapy_hackrf_source_0->get_sample_rate(0);
    }

    void setSample_rate(int newSample_rate)
    {
        soapy_hackrf_source_0->set_sample_rate(0, newSample_rate);
        sample_rate = newSample_rate;
    }

    double getCurrentFrequency() const
    {
        return soapy_hackrf_source_0->get_frequency(0);
    }

    void setCurrentFrequency(double newCurrentFrequency)
    {
        soapy_hackrf_source_0->set_frequency(0, newCurrentFrequency);
        currentFrequency = newCurrentFrequency;
    }

private:
    int sample_rate;
    double currentFrequency;
    std::shared_ptr<CustomBuffer> customBuffer;
    gr::soapy::source::sptr soapy_hackrf_source_0;
    gr::filter::rational_resampler_ccf::sptr rational_resampler_xxx_0;
    gr::blocks::multiply_const_ff::sptr blocks_multiply_const_vxx_0;
    gr::audio::sink::sptr audio_sink_0;
    gr::analog::quadrature_demod_cf::sptr analog_quadrature_demod_cf_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter;
};
