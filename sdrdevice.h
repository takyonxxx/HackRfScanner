#ifndef SDRDEVICE_H
#define SDRDEVICE_H

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include "fm_transmitter.h"
#include "fm_receiver.h"

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
    void setMode(ReceiverMode rMode, int frequency);

    void start();
    void stop();

private:
    bool m_isStarted;
    Demod currentDemod;
    FreqMod currentFreqMod;
    ReceiverMode currentReceiverMode;
    FmTransmitter *_transmitter;
    FmReceiver *_receiver;

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
