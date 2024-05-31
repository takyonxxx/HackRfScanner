QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bandplan.cpp \
    custombuffer.cpp \
    dxc_spots.cpp \
    freqctrl.cpp \
    main.cpp \
    mainwindow.cpp \
    meter.cpp \
    plotter.cpp \
    sdrdevice.cpp

HEADERS += \
    bandplan.h \
    constants.h \
    custombuffer.h \
    dxc_spots.h \
    fm_receiver.h \
    fm_transmitter.h \
    freqctrl.h \
    mainwindow.h \
    meter.h \
    plotter.h \
    sdrdevice.h

FORMS += \
    mainwindow.ui


macos {
    QMAKE_INFO_PLIST = ./macos/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/macos/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

     HOMEBREW_CELLAR_PATH = /opt/homebrew/Cellar

     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/hackrf/2024.02.1/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/spdlog/1.13.0/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/fmt/10.2.1_1/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/gmp/6.3.0/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/volk/3.1.2/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/gnuradio/3.10.9.2_3/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/boost/1.85.0/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/soapysdr/0.8.1_1/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/fftw/3.3.10_1/include
     INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/portaudio/19.7.0/include

#    HOMEBREW_CELLAR_PATH = /usr/local/Cellar

#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/hackrf/2024.02.1/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/spdlog/1.13.0/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/fmt/10.2.1_1/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/gmp/6.3.0/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/volk/3.1.2/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/gnuradio/3.10.9.2_4/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/boost/1.85.0/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/soapysdr/0.8.1_1/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/fftw/3.3.10_1/include
#    INCLUDEPATH += $$HOMEBREW_CELLAR_PATH/portaudio/19.7.0/include

    LIBS += -L$$HOMEBREW_CELLAR_PATH/gnuradio/3.10.9.2_3/lib \
        -lgnuradio-analog \
        -lgnuradio-blocks \
        -lgnuradio-digital \
        -lgnuradio-filter \
        -lgnuradio-fft \
        -lgnuradio-runtime \
        -lgnuradio-audio \
        -lgnuradio-soapy \
        -lgnuradio-pmt \
        -lgnuradio-uhd

    LIBS += -L$$HOMEBREW_CELLAR_PATH/hackrf/2024.02.1/lib -lhackrf
    LIBS += -L$$HOMEBREW_CELLAR_PATH/boost/1.85.0/lib -lboost_system -lboost_filesystem-mt -lboost_program_options
    LIBS += -L$$HOMEBREW_CELLAR_PATH/soapysdr/0.8.1_1/lib -lSoapySDR
    LIBS += -L$$HOMEBREW_CELLAR_PATH/fftw/3.3.10_1/lib -lfftw3f
    LIBS += -L$$HOMEBREW_CELLAR_PATH/volk/3.1.2/lib -lvolk
    LIBS += -L$$HOMEBREW_CELLAR_PATH/portaudio/19.7.0/lib -lportaudio

}

unix:!macx{
    message("linux enabled")
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/include/osmosdr
    INCLUDEPATH += /usr/lib
    INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /usr/lib/x86_64-linux-gnu
    LIBS += -L/usr/lib/aarch64-linux-gnu/SoapySDR/modules0.8

    LIBS += -lboost_system -lboost_program_options -lboost_thread
    LIBS += -lrt -lpthread -losmosdr -lfmt -llog4cpp -lSoapySDR -lfftw3
    LIBS += -lgnuradio-analog \
    -lgnuradio-blocks \
    -lgnuradio-digital \
    -lgnuradio-filter \
    -lgnuradio-fft \
    -lgnuradio-runtime \
    -lgnuradio-audio \
    -lgnuradio-soapy \
    -lgnuradio-pmt \
    -lgnuradio-uhd
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    CMakeLists.txt
