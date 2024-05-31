#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,  m_ptt(false)
    ,  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("HackRf");

    ui->m_pBPtt->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #FF5733;");
    ui->m_cFreqType->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cDemod->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cFreqStep->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->m_pIncFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");
    ui->m_pDecFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");
    ui->labelFreqStep->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #FF5733;");

    ui->m_cFreqType->setMinimumHeight(30);
    ui->m_cDemod->setMinimumHeight(30);
    ui->m_cFreqStep->setMinimumHeight(40);
    ui->m_cFreqStep->setMaximumWidth(80);
    ui->labelFreqStep->setMinimumHeight(40);
    ui->labelFreqStep->setMaximumWidth(60);


    ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_MHZ);
    ui->freqCtrl->setDigitColor(QColor("#FFC300"));
    ui->freqCtrl->setFrequency(DEFAULT_FREQUENCY);
    connect(ui->freqCtrl, &CFreqCtrl::newFrequency, this, &MainWindow::onFreqCtrl_setFrequency);


    ui->pushToggleSdr->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->pushExit->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->m_cFreqStep->addItem("1", QVariant(1));
    ui->m_cFreqStep->addItem("5", QVariant(5));
    ui->m_cFreqStep->addItem("10", QVariant(10));
    ui->m_cFreqStep->addItem("50", QVariant(50));
    ui->m_cFreqStep->addItem("100", QVariant(100));

    int m_LowCutFreq = _KHZ(-75);
    int m_HiCutFreq = _KHZ(75);

    ui->plotter->setTooltipsEnabled(true);

    ui->plotter->setSampleRate(DEFAULT_SAMPLE_RATE);
    ui->plotter->setSpanFreq(static_cast<quint32>(DEFAULT_SAMPLE_RATE));
    ui->plotter->setCenterFreq(static_cast<quint64>(DEFAULT_FREQUENCY));

    ui->plotter->setFftRange(-140.0f, 20.0f);
    ui->plotter->setPandapterRange(-140.f, 20.f);
    ui->plotter->setHiLowCutFrequencies(m_LowCutFreq, m_HiCutFreq);
    ui->plotter->setDemodRanges(m_LowCutFreq, -_KHZ(5), _KHZ(5),m_HiCutFreq, true);

    ui->plotter->setFreqUnits(_KHZ(1));
    ui->plotter->setPercent2DScreen(50);
    ui->plotter->setFilterBoxEnabled(true);
    ui->plotter->setCenterLineEnabled(true);
    ui->plotter->setClickResolution(1);

    ui->plotter->setFftPlotColor(QColor("#CEECF5"));

    ui->sMeter->setMinimumHeight(60);
    ui->sMeter->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");

    currentDemod    = DEMOD_WFM;
    currentFreqMod  = MHZ;
    currentFrequency = 100 * 1000 * 1000;
    freq_type_index = 2;
    demod_index = 1;

    d_realFftData = new float[MAX_FFT_SIZE];
    d_pwrFftData = new float[MAX_FFT_SIZE]();
    d_iirFftData = new float[MAX_FFT_SIZE];
    for (int i = 0; i < MAX_FFT_SIZE; i++)
        d_iirFftData[i] = RESET_FFT_FACTOR;  // dBFS

    ui->m_cDemod->setCurrentIndex(demod_index);

    QString homePath = QDir::homePath();
    m_sSettingsFile = homePath + "/settings.ini";

    sdrDevice = new SdrDevice(this);
    connect(sdrDevice, &SdrDevice::infoFrequency, this, &MainWindow::infoFrequency);
    connect(sdrDevice, &SdrDevice::rxBuffer, this, &MainWindow::getRxBuffer);
    sdrDevice->setMode(ReceiverMode::RX, currentFrequency);

    if (QFile(m_sSettingsFile).exists())
        loadSettings();
    else
        saveSettings();
}

MainWindow::~MainWindow()
{   
    if (sdrDevice) {
        delete sdrDevice;
        sdrDevice = nullptr;
    }
    delete ui;
}


void MainWindow::onFreqCtrl_setFrequency(qint64 freq)
{
    sdrDevice->setFrequency(freq);
    currentFrequency = sdrDevice->getCenterFrequency();
    saveSettings();
}

void MainWindow::infoFrequency(int freq)
{
    ui->freqCtrl->setFrequency(freq);
}


void MainWindow::on_pushToggleSdr_clicked()
{
    if(ui->pushToggleSdr->text() == "Start")
    {
        sdrDevice->start();
        ui->pushToggleSdr->setText("Stop");
    }
    else
    {
        sdrDevice->stop();
        ui->pushToggleSdr->setText("Start");
    }
}


void MainWindow::on_pushExit_clicked()
{
    sdrDevice->stop();
    delete sdrDevice;
    exit(0);
}


void MainWindow::on_m_pIncFreq_clicked()
{
    int selectedIndex = ui->m_cFreqStep->currentIndex();

    QVariant selectedValue = ui->m_cFreqStep->itemData(selectedIndex);
    currentFrequency = sdrDevice->getCenterFrequency();

    int selectedIntValue = selectedValue.toInt();
    if (currentFreqMod == FreqMod::KHZ) {
        selectedIntValue = 1000 * selectedIntValue;
    } else if (currentFreqMod == FreqMod::MHZ) {
        selectedIntValue = 1000 * 1000 * selectedIntValue;
    }else if (currentFreqMod == FreqMod::GHZ) {
        selectedIntValue = 1000 * 1000 * 1000 * selectedIntValue;
    }

    currentFrequency = currentFrequency + selectedIntValue;  
    sdrDevice->setFrequency(currentFrequency);
    ui->freqCtrl->setFrequency(currentFrequency);
    saveSettings();
}


void MainWindow::on_m_pDecFreq_clicked()
{
    int selectedIndex = ui->m_cFreqStep->currentIndex();

    QVariant selectedValue = ui->m_cFreqStep->itemData(selectedIndex);
    currentFrequency = sdrDevice->getCenterFrequency();

    int selectedIntValue = selectedValue.toInt();
    if (currentFreqMod == FreqMod::KHZ) {
        selectedIntValue = 1000 * selectedIntValue;
    } else if (currentFreqMod == FreqMod::MHZ) {
        selectedIntValue = 1000 * 1000 * selectedIntValue;
    }else if (currentFreqMod == FreqMod::GHZ) {
        selectedIntValue = 1000 * 1000 * 1000 * selectedIntValue;
    }

    currentFrequency = currentFrequency - selectedIntValue;
    sdrDevice->setFrequency(currentFrequency);
    ui->freqCtrl->setFrequency(currentFrequency);
    saveSettings();
}


void MainWindow::on_m_cFreqType_currentIndexChanged(int index)
{
    if (index >= 0 && index < ui->m_cFreqType->count())
    {
        currentFreqMod = static_cast<FreqMod>(index);
        freq_type_index = index;
        switch (index) {
        case 0:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_HZ);
            break;
        case 1:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_KHZ);
            break;
        case 2:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_MHZ);
            break;
        case 3:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_GHZ);
            break;
        default:
            // Handle invalid index
            break;
        }
        ui->freqCtrl->setFrequency(currentFrequency);
        saveSettings();
    }
}


void MainWindow::on_m_cDemod_currentIndexChanged(int index)
{
    if (index >= 0 && index < ui->m_cDemod->count())
    {
        currentDemod = static_cast<Demod>(index);
        demod_index = index;
        saveSettings();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    freq_type_index = settings.value("freq_type_index", "").toString().toInt();
    demod_index = settings.value("demod_index", "").toString().toInt();
    currentFrequency = settings.value("current_frequency", "").toString().toDouble();
    ui->m_cFreqType->setCurrentIndex(freq_type_index);
    ui->m_cDemod->setCurrentIndex(demod_index);

    auto freq = 0.0;
    if (currentFreqMod == FreqMod::KHZ) {
        freq = currentFrequency / 1000.0;
    } else if (currentFreqMod == FreqMod::MHZ) {
        freq = currentFrequency / 1000.0 / 1000.0;
    }else if (currentFreqMod == FreqMod::GHZ) {
        freq = currentFrequency / 1000.0 / 1000.0 / 1000.0;
    }    

    sdrDevice->setFrequency(currentFrequency);
    ui->freqCtrl->setFrequency(currentFrequency);
}

void MainWindow::saveSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("freq_type_index", QString::number(freq_type_index));
    settings.setValue("demod_index", QString::number(demod_index));
    settings.setValue("current_frequency", QString::number(currentFrequency));
}

void MainWindow::on_m_pBPtt_clicked()
{
    if (ui->m_pBPtt->text() == "RX")
    {
        ui->m_pBPtt->setText("TX");
        m_ptt = false;
        sdrDevice->setMode(ReceiverMode::TX, currentFrequency);
    }
    else
    {
        ui->m_pBPtt->setText("RX");
        m_ptt = true;
        sdrDevice->setMode(ReceiverMode::RX, currentFrequency);
    }
}

void MainWindow::getRxBuffer(const float *in, int size)
{
    qDebug() << size;
//    // Perform FFT on the input data
//    fftw_complex *fft_out;
//    fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * size);

//    double* in_double = new double[size];
//    for (int i = 0; i < size; ++i) {
//        in_double[i] = static_cast<double>(in[i]);
//    }

//    // Create a plan for forward FFT
//    fftw_plan plan_forward = fftw_plan_dft_r2c_1d(size, in_double, fft_out, FFTW_ESTIMATE);

//    fftw_execute(plan_forward);

//    // Perform calculations or process the FFT data as needed
//    for (int i = 0; i < size; ++i)
//    {
//        // Access the real and imaginary parts of the FFT result
//        double real_part = fft_out[i][0];
//        double imag_part = fft_out[i][1];

//        // Process or perform calculations with the FFT result
//        // For example, you can calculate magnitude, phase, etc.
//        // Here, I'll just print them as an example
//        qDebug() << "FFT result at index" << i << ": Real =" << real_part << ", Imaginary =" << imag_part;
//    }

//    // Clean up FFT resources
//    fftw_destroy_plan(plan_forward);
//    fftw_free(fft_out);
//    delete[] in_double;
}

void MainWindow::fftTimeout()
{
//    unsigned int fftsize;
//    unsigned int i;
//    float pwr;
//    float pwr_scale;
//    double fullScalePower = 1.0;
//    std::complex<float> pt;
//    double x_hat = 0; // State estimate
//    double x_hat_level = 0; // State estimate
//    double sum_signal_level = 0;
//    int num_iterations = 0;

//    // 75 is default
//    d_fftAvg = static_cast<float>(1.0 - 1.0e-2 * 90);

//    fftsize = static_cast<unsigned int>(m_Demodulator->fftSize());
//    if (fftsize > MAX_FFT_SIZE)
//        fftsize = MAX_FFT_SIZE;

//    auto d_fftData = m_Demodulator->spectrum();
//    if (fftsize == 0)
//    {
//        return;
//    }

//    pwr_scale = static_cast<float>(1.0 / fftsize);

//    for (i = 0; i < fftsize; i++)
//    {
//        if (i < fftsize / 2)
//        {
//            pt = d_fftData[fftsize / 2 + i];
//        }
//        else
//        {
//            pt = d_fftData[i - fftsize / 2];
//        }

//        /* calculate power in dBFS */
//        pwr = pwr_scale * (pt.imag() * pt.imag() + pt.real() * pt.real());

//        /* calculate signal level in dBFS */
//        double fft_signal = 20 * std::log10(pwr / fftsize / fullScalePower);
//        auto level = 10 * std::log10(pwr + 1.0e-20f / fullScalePower);
//        kalmanFilterUpdate(x_hat, fft_signal, 1);
//        kalmanFilterUpdate(x_hat_level, level, 1);
//        sum_signal_level += x_hat_level;
//        d_realFftData[i] = x_hat;
//        d_iirFftData[i] += d_fftAvg * (d_realFftData[i] - d_iirFftData[i]);
//        num_iterations++;
//    }

//    signal_level = sum_signal_level / num_iterations;
//    ui->plotter->setNewFttData(d_iirFftData, d_realFftData, static_cast<int>(fftsize));
}
