#include "custombuffer.h"


CustomBuffer::CustomBuffer(const std::string& device_name, QObject *parent)
    : QObject(parent),
    gr::sync_block(device_name,
                   gr::io_signature::make(1, 1, sizeof(gr_complex)),
                   gr::io_signature::make(0, 0, 0))
{

}

CustomBuffer::~CustomBuffer()
{

}

int CustomBuffer::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
{
    const float* in = (const float*)input_items[0];
    // size_t byte_size = noutput_items * sizeof(float);
    // QByteArray buffer(reinterpret_cast<const char*>(in), byte_size);
    // Perform FFT on the data
    // performFFT(in, noutput_items);

    return noutput_items;
}

void CustomBuffer::performFFT(const float* data, int length)
{
    // Allocate input and output buffers
    std::vector<float> input(length);
    std::vector<std::complex<float>> output(length / 2 + 1);

    // Copy input data
    std::copy(data, data + length, input.begin());

    // Create FFTW plan
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(length, input.data(), reinterpret_cast<fftwf_complex*>(output.data()), FFTW_ESTIMATE);

    // Execute FFT
    fftwf_execute(plan);
}

CustomBuffer::sptr CustomBuffer::make(const std::string& device_name)
{
    return std::make_shared<CustomBuffer>(device_name);
}
