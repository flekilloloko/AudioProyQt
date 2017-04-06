#include "Widget.h"
#include "ui_Widget.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <windows.h>
#include <math.h>
#include "fftreal/FFTReal.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    arduino_is_available = false;
    arduino_port_name = "";
    arduino = new QSerialPort;
    bufferSerie = "";
    punteroX = 0;
    sincronizado = false;
    armandoEspectro = false;

    espectro = "";


    qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        qDebug() << "Has vendor ID: " << serialPortInfo.hasVendorIdentifier();
        if(serialPortInfo.hasVendorIdentifier()){
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
        }
        qDebug() << "Has Product ID: " << serialPortInfo.hasProductIdentifier();
        if(serialPortInfo.hasProductIdentifier()){
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
        }
    }


    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            if(serialPortInfo.vendorIdentifier() == arduino_due_vendor_id){
                if(serialPortInfo.productIdentifier() == arduino_due_product_id){
                    arduino_port_name = serialPortInfo.portName();
                    arduino_is_available = true;
                }
            }
        }
    }

    if(arduino_is_available){
        // open and configure the serialport
        arduino->setPortName(arduino_port_name);
        arduino->open(QSerialPort::ReadOnly);
        arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(leerPuertoSerie()));
    }else{
        // give error message if not available
        QMessageBox::warning(this, "Port error", "Couldn't find the Arduino!");
    }


    x.resize(512); y.resize(512);
    for (int i=0; i<512; i++)
    {
      x[i] = i*48.828125*2*M_PI ; //50k             //x[i] = i*47.157; //48,28877kHz muestreo    20,71uS entre muestras sucesivas
      y[i] = 0;
    }



    QCPGraph *subGraf = ui->grafico->addGraph();
    ui->grafico->graph(0)->setData(x,y);
    ui->grafico->xAxis->setLabel("Frec");
    ui->grafico->yAxis->setLabel("Módulo");
    ui->grafico->xAxis->setRange(1, 20000);
    ui->grafico->yAxis->setRange(0, 100);
    ui->grafico->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->grafico->yAxis->grid()->setVisible(false);

    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    ui->grafico->xAxis->setTicker(logTicker);
    ui->grafico->xAxis->setNumberFormat("eb");
    subGraf->setLineStyle(QCPGraph::lsImpulse);
    subGraf->setPen(QPen(QColor("#FFA100"), 1.5));
    subGraf->setName("Espectro en frecuencias");
    ui->grafico->xAxis->setNumberPrecision(0);
    //ui->grafico->xAxis->grid()->
    ui->grafico->legend->setVisible(true);
    ui->grafico->setNoAntialiasingOnDrag(true); // more performance/responsiveness during dragging

    ui->grafico->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->grafico->replot();



}

Widget::~Widget()
{
    if(arduino->isOpen()) arduino->close();
    delete ui;
}

void Widget::leerPuertoSerie(){
    int pos;
    QString valor = 0, otro;
    QTime tiempo;
    int tamano;
    bool esdig=false;
    //static int cont = 0;

    //qDebug() << tiempo.elapsed();
    valor = "";
    datosSerie = arduino->readAll();    
    bufferSerie += QString::fromStdString(datosSerie.toStdString());   //aca pa delante nuevo
    pos = bufferSerie.indexOf("_");
    if(pos>=0){

            espectro += bufferSerie.left(pos);
            armandoEspectro = true;

            for(int i=0;i<espectro.size()-1;i++){
                if (espectro[i]>95)armandoEspectro = false;
            }
            QStringList frecuencias = espectro.split(",");
            tamano = frecuencias.size();
            if(tamano>1024) armandoEspectro = false;
            //if(espectro.size() > 512) armandoEspectro = false;
            if(armandoEspectro){


                espectro = bufferSerie.right(bufferSerie.size()-pos-1);
                bufferSerie.clear();
                for(int i=0;i<(frecuencias.size()-1);i++) {
                    esdig=true;
                    for(int j = 0;j<frecuencias.at(i).size();j++){
                        if(!frecuencias.at(i)[j].isDigit()) esdig = false;
                    }
                    if(esdig && i<512)y[i]=frecuencias.at(i).toDouble()/32;

                }
                frecuencias.clear();
                ui->grafico->graph(0)->setData(x,y);
                ui->grafico->replot();
            } else {

                espectro = bufferSerie.right(bufferSerie.size()-pos-1);
                bufferSerie.clear();
                }

        }else{
            espectro += bufferSerie;
            bufferSerie.clear();
        }
        //qDebug() << tiempo.elapsed();

}


