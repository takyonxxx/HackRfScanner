#ifndef CUSTOMBUFFER_H
#define CUSTOMBUFFER_H
#include <QtCore>
#include <complex>
#include <gnuradio/sync_block.h>
#include <fftw3.h>

const double PI = acos(-1);


class CustomBuffer : public QObject, public gr::sync_block
{
    Q_OBJECT

public:
    typedef std::shared_ptr<CustomBuffer> sptr;
    static sptr make(const std::string& device_name = "");

    CustomBuffer(const std::string& device_name, QObject *parent = nullptr);
    ~CustomBuffer() override;

    void connectToServer(const QString &hostAddress, quint16 port);
    void performFFT(const float* data, int length);

private:   
    int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items) override;
};

#endif // CUSTOMBUFFER_H
