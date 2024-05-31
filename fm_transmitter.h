#ifndef FM_TRANSMITTER_H
#define FM_TRANSMITTER_H

#include <QtCore>
#include <iostream>
#include <gnuradio/top_block.h>
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/audio/source.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/soapy/sink.h>
#include "constants.h"

class FmTransmitter : public QObject,public gr::top_block
{
    Q_OBJECT
public:
    FmTransmitter(int frequency) : QObject(nullptr), gr::top_block("Fm Transmitter")
    {
        std::string dev = "hackrf=0";
        std::string stream_args = "";
        std::vector<std::string> tune_args = {""};
        std::vector<std::string> settings = {""};

        int sample_rate = DEFAULT_SAMPLE_RATE;
        int audio_samp_rate = DEFAULT_AUDIO_SAMPLE_RATE;
        double interpolation = 48 * (DEFAULT_SAMPLE_RATE / _MHZ(2));

        soapy_hackrf_sink_0 = gr::soapy::sink::make(
            "hackrf",
            "fc32",
            1,
            dev,
            stream_args,
            tune_args,
            settings
            );

        if (!soapy_hackrf_sink_0) {
            throw std::runtime_error("Failed to create SoapySDR sink.");
        }

        soapy_hackrf_sink_0->set_sample_rate(0, sample_rate);
        soapy_hackrf_sink_0->set_frequency(0, frequency);
        soapy_hackrf_sink_0->set_gain(0, "AMP", true);
        soapy_hackrf_sink_0->set_gain(0, "VGA", std::min(std::max(35.0, 0.0), HACKRF_TX_VGA_MAX_DB));

        rational_resampler_xxx_0 = gr::filter::rational_resampler_ccf::make(interpolation, 1);
        blocks_multiply_const_vxx_0 = gr::blocks::multiply_const_ff::make(4);
        audio_source_0 = gr::audio::source::make(audio_samp_rate, "", true);
        analog_frequency_modulator_fc_0 = gr::analog::frequency_modulator_fc::make(1.5);

        // Connections
        gr::top_block::connect((const gr::block_sptr&)analog_frequency_modulator_fc_0, 0, (const gr::block_sptr&)rational_resampler_xxx_0, 0);
        gr::top_block::connect((const gr::block_sptr&)audio_source_0, 0, (const gr::block_sptr&)blocks_multiply_const_vxx_0, 0);
        gr::top_block::connect((const gr::block_sptr&)blocks_multiply_const_vxx_0, 0, (const gr::block_sptr&)analog_frequency_modulator_fc_0, 0);
        gr::top_block::connect((const gr::block_sptr&)rational_resampler_xxx_0, 0, (const gr::block_sptr&)soapy_hackrf_sink_0, 0);
    }

    int getSample_rate() const;
    void setSample_rate(int newSample_rate);

    double getCurrentFrequency() const;
    void setCurrentFrequency(double newCurrentFrequency);

private:
    int sample_rate ;
    double currentFrequency;
    gr::soapy::sink::sptr soapy_hackrf_sink_0;
    gr::filter::rational_resampler_ccf::sptr rational_resampler_xxx_0;
    gr::blocks::multiply_const_ff::sptr blocks_multiply_const_vxx_0;
    gr::audio::source::sptr audio_source_0;
    gr::analog::frequency_modulator_fc::sptr analog_frequency_modulator_fc_0;
};

inline int FmTransmitter::getSample_rate() const
{
    return soapy_hackrf_sink_0->get_sample_rate(0);
}

inline void FmTransmitter::setSample_rate(int newSample_rate)
{
    soapy_hackrf_sink_0->set_sample_rate(0, sample_rate);
    sample_rate = newSample_rate;
}

inline double FmTransmitter::getCurrentFrequency() const
{
    return soapy_hackrf_sink_0->get_frequency(0);
}

inline void FmTransmitter::setCurrentFrequency(double newCurrentFrequency)
{
    soapy_hackrf_sink_0->set_frequency(0, newCurrentFrequency);
    currentFrequency = newCurrentFrequency;
}

#endif // FM_TRANSMITTER_H
