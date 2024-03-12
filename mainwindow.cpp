#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("HackRf");

    ui->freqCtrl->setG_constant(2.5);
    ui->freqCtrl->Setup(11, 0, 2200e6, 1, UNITS_MHZ);
    ui->freqCtrl->SetDigitColor(QColor("#FFC300"));
    ui->freqCtrl->SetFrequency(DEFAULT_FREQUENCY);

    ui->pushToggleSdr->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->pushExit->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");

    sdrDevice = new SdrDevice(this);
    connect(sdrDevice, &SdrDevice::infoFrequency, this, &MainWindow::infoFrequency);
}

MainWindow::~MainWindow()
{
    delete sdrDevice;
    delete ui;
}

void MainWindow::infoFrequency(int freq)
{
    ui->freqCtrl->SetFrequency(freq);
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
    exit(0);
}

