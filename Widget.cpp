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

    float fmuestreo = 50000;
    QElapsedTimer tiemp;
    
    //for(int i=0 ; i<len ; i++) muestrixs[i] = qSin(2*M_PI*3000*i*(1/fmuestreo));

    arduino_is_available = false;
    arduino_port_name = "";
    arduino = new QSerialPort;
    bufferSerie = "";
    punteroX = 0;
    sincronizado = false;
    armandoEspectro = false;

    espectro = "";
    


   // 1024-point FFT object constructed.


    //qDebug()<<tiemp.nsecsElapsed();


 
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


    x.resize(len); y.resize(len);
    for (int i=0; i<len; i++)
    {
      x[i] = i*(fmuestreo/1024) ; //50k             //x[i] = i*47.157; //48,28877kHz muestreo    20,71uS entre muestras sucesivas
      y[i] = 0;
    }



    QCPGraph *subGraf = ui->grafico->addGraph();
    ui->grafico->graph(0)->setData(x,y);
    ui->grafico->xAxis->setLabel("Frec");
    ui->grafico->yAxis->setLabel("Módulo");
    ui->grafico->xAxis->setRange(1000, 5000);
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
    float muestrixs[len];
    float espectrix[len];
    //static int cont = 0;
    ffft::FFTReal <float> fft_object (len);
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
                    if(esdig && i<512)muestrixs[i]=frecuencias.at(i).toDouble();

                }
                frecuencias.clear();
                fft_object.do_fft(espectrix,muestrixs);
                for (int i=0; i<len; i++) y[i] = espectrix[i]/5; // div 5 para entradas con 1.0 max

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
