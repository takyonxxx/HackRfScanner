#include "sdrdevice.h"

SdrDevice::SdrDevice(QObject *parent):
    QObject(parent)
{
    m_isStarted = false;    
}

SdrDevice::~SdrDevice()
{
    if(_receiver)
    {
        _receiver->stop();
        delete _receiver;
    }
    if(_transmitter)
    {
        _transmitter->stop();
        delete _transmitter;
    }
}

void SdrDevice::setFrequency(double frequency)
{
    if (currentReceiverMode == ReceiverMode::TX)
    {
        _transmitter->setCurrentFrequency(frequency);
    }
    else
    {
        _receiver->setCurrentFrequency(frequency);
    }
    emit infoFrequency(getCenterFrequency());
}

double SdrDevice::getCenterFrequency() const
{
    if (currentReceiverMode == ReceiverMode::TX)
    {
        return _transmitter->getCurrentFrequency();
    }
    else
    {
        return _receiver->getCurrentFrequency();
    }
}

void SdrDevice::setMode(ReceiverMode rMode, int frequency)
{    
    currentReceiverMode = rMode;
    if (currentReceiverMode == ReceiverMode::TX)
    {
        if(_receiver)
        {
            _receiver->stop();
            delete _receiver;
        }
        _transmitter = new FmTransmitter(frequency);
        if(m_isStarted)
            _transmitter->start();
    }
    else if (currentReceiverMode == ReceiverMode::RX)
    {
        if(_transmitter)
        {
            _transmitter->stop();
            delete _transmitter;
        }
        _receiver = new FmReceiver(frequency);        
        if(m_isStarted)
            _receiver->start();
    }
}

void SdrDevice::start()
{
    if (currentReceiverMode == ReceiverMode::TX)
    {
        _transmitter->start();
    }
    else
    {
        _receiver->start();
    }
    m_isStarted = true;
}

void SdrDevice::stop()
{
    if (currentReceiverMode == ReceiverMode::TX)
    {
        _transmitter->stop();
    }
    else
    {
        _receiver->stop();
    }
    m_isStarted = false;
}
