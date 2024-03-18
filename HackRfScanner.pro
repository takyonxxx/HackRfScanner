QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    custombuffer.cpp \
    freqctrl.cpp \
    main.cpp \
    mainwindow.cpp \
    sdrdevice.cpp

HEADERS += \
    custombuffer.h \
    freqctrl.h \
    mainwindow.h \
    sdrdevice.h

FORMS += \
    mainwindow.ui


macos {
    QMAKE_INFO_PLIST = ./macos/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/macos/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    INCLUDEPATH += /opt/homebrew/Cellar/spdlog/1.12.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/fmt/10.2.1/include
    INCLUDEPATH += /opt/homebrew/Cellar/gmp/6.3.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/gnuradio/3.10.9.2_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/boost/1.84.0_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/soapysdr/0.8.1_1/include

#    INCLUDEPATH += /usr/local/Cellar/spdlog/1.12.0/include
#    INCLUDEPATH += /usr/local/Cellar/fmt/10.2.1_1/include
#    INCLUDEPATH += /usr/local/Cellar/gmp/6.3.0/include
#    INCLUDEPATH += /usr/local/Cellar/gnuradio/3.10.9.2_1/include
#    INCLUDEPATH += /usr/local/Cellar/boost/1.84.0_1/include
#    INCLUDEPATH += /usr/local/Cellar/soapysdr/0.8.1_1/include

    INCLUDEPATH += /opt/homebrew/Cellar/gnuradio/3.10.9.2_1/lib
#    INCLUDEPATH +=  /usr/local/Cellar/gnuradio/3.10.9.2_1/lib

    LIBS += -L/opt/homebrew/Cellar/gnuradio/3.10.9.2_1/lib \
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

    LIBS += -L/opt/homebrew/Cellar/boost/1.84.0_1/lib -lboost_system -lboost_filesystem-mt -lboost_program_options
    LIBS += -L/opt/homebrew/Cellar/soapysdr/0.8.1_1/lib -lSoapySDR
#    LIBS += -L/usr/local/Cellar/boost/1.84.0_1/lib -lboost_system -lboost_filesystem-mt -lboost_program_options
#    LIBS += -L/usr/local/Cellar/soapysdr/0.8.1_1/lib -lSoapySDR
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
    LIBS += -lrt -lpthread -losmosdr -lfmt -llog4cpp -lSoapySDR
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
