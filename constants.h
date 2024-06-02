#ifndef CONSTANTS_H
#define CONSTANTS_H

// #include <SoapySDR/Device.hpp>
// #include <SoapySDR/Formats.hpp>
// #include <SoapySDR/Errors.hpp>
// #include <SoapySDR/Time.hpp>

#define _GHZ(x) ((uint64_t)(x) * 1000000000)
#define _MHZ(x) ((x) * 1000000)
#define _KHZ(x) ((x) * 1000)
#define _HZ(x) ((x) * 1)
#define DEFAULT_SAMPLE_RATE             _MHZ(4)
#define DEFAULT_AUDIO_SAMPLE_RATE       _KHZ(44.1)
#define DEFAULT_CUT_OFF                 _KHZ(300)
#define DEFAULT_FREQUENCY               _MHZ(144)
#define DEFAULT_AUDIO_GAIN              1.0
#define DEFAULT_FFT_SIZE                8192
#define DEFAULT_FFT_RATE                25 //Hz
#define MAX_FFT_SIZE                 DEFAULT_FFT_SIZE
#define RESET_FFT_FACTOR             -72

#define HACKRF_RX_VGA_MAX_DB        40.0
#define HACKRF_RX_LNA_MAX_DB        40.0
#define HACKRF_TX_VGA_MAX_DB        47.0
#define HACKRF_AMP_MAX_DB           14.0
#define FRAMES_PER_BUFFER            512


#endif // CONSTANTS_H
