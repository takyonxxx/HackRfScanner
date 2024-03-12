#ifndef SDRDEVICE_H
#define SDRDEVICE_H

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include <gnuradio/constants.h>
#include <gnuradio/prefs.h>
#include <gnuradio/top_block.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/soapy/source.h>
#include <gnuradio/blocks/complex_to_float.h>

#include <gnuradio/audio/source.h>
#include <gnuradio/analog/frequency_modulator_fc.h>

#include "custombuffer.h"

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

#define _GHZ(x) ((uint64_t)(x) * 1000000000)
#define _MHZ(x) ((x) * 1000000)
#define _KHZ(x) ((x) * 1000)
#define _HZ(x) ((x) * 1)
#define DEFAULT_SAMPLE_RATE             _MHZ(20)
#define DEFAULT_AUDIO_SAMPLE_RATE       _KHZ(48)
#define DEFAULT_CUT_OFF                 _KHZ(300)
#define DEFAULT_FREQUENCY               _MHZ(100)
#define DEFAULT_AUDIO_GAIN              1.0

#define HACKRF_RX_VGA_MAX_DB 62.0
#define HACKRF_TX_VGA_MAX_DB 47.0
#define HACKRF_RX_LNA_MAX_DB 40.0
#define HACKRF_AMP_MAX_DB 14.0

typedef enum {
    DEMOD_AM,
    DEMOD_WFM
} Demod;

typedef enum {
    HZ,
    KHZ,
    MHZ,
    GHZ
} FreqMod;

typedef enum {
    RX,
    TX
} ReceiverMode;

class SdrDevice : public QObject
{
    Q_OBJECT
public:
    explicit SdrDevice(QObject *parent = nullptr);
    ~SdrDevice();

    void setFrequency(double frequency);
    double getCenterFrequency() const;
    void setMode(ReceiverMode rMode);

    void start();
    void stop();

private:
    gr::soapy::source::sptr hackrf_soapy_source;
    gr::top_block_sptr tb;
    std::shared_ptr<CustomBuffer> customBuffer;

    int sample_rate ;
    int audio_samp_rate;
    int transition;
    int decimation;
    int interpolation;
    int resampler_decimation;
    int cut_off;
    double audio_gain;
    double currentFrequency;    
    bool m_ptt;
    Demod currentDemod;
    FreqMod currentFreqMod;

    QString enumDemodToString(Demod demod)
    {
        switch (demod)
        {
        case DEMOD_AM:
            return "AM";
        case DEMOD_WFM:
            return "WFM";
        default:
            return "Unknown";
        }
    }

    QString enumFreqModToString(FreqMod fmod)
    {
        switch (fmod)
        {
        case HZ:
            return "Hz";
        case KHZ:
            return "KHz";
        case MHZ:
            return "MHz";
        case GHZ:
            return "GHz";
        default:
            return "Unknown";
        }
    }
signals:
    void infoFrequency(int f);
};

#endif // SDRDEVICE_H
