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
#include <complex>


#define tamFFT 1024

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    float fmuestreo = 50000;
    QElapsedTimer tiemp;
    QVector<QCPGraphData> datosFrec(tamFFT/2);
    
    //for(int i=0 ; i<tamFFT ; i++) muestrixs[i] = qSin(2*M_PI*3000*i*(1/fmuestreo));

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
        arduino->open(QSerialPort::ReadWrite);
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


    x.resize(tamFFT/2); y.resize(tamFFT/2);
    for (int i=0; i<tamFFT/2; i++)
    {
      datosFrec[i].key = i*(fmuestreo/(tamFFT)) ; //50k             //x[i] = i*47.157; //48,28877kHz muestreo    20,71uS entre muestras sucesivas
      datosFrec[i].value = 0;
    }


    ui->grafico->plotLayout()->clear();
    ui->grafico->setFixedHeight(600);
    ui->grafico->setFixedWidth(600);
    QCPAxisRect *frecuenciasTotal = new QCPAxisRect(ui->grafico);
    frecuenciasTotal->setupFullAxesBox(true);
    frecuenciasTotal->axis(QCPAxis::atRight, 0)->setTickLabels(true);

    QCPAxisRect *frecuenciasIndiv = new QCPAxisRect(ui->grafico);
    ui->grafico->plotLayout()->addElement(0, 0, frecuenciasTotal);
    ui->grafico->plotLayout()->addElement(1, 0, frecuenciasIndiv);
    frecuenciasTotal->setMaximumSize(600, 250);
    frecuenciasTotal->setMinimumSize(600, 250);

    QCPMarginGroup *margenes = new QCPMarginGroup(ui->grafico);
    frecuenciasTotal->setMarginGroup(QCP::msLeft | QCP::msRight, margenes);
    frecuenciasIndiv->setMarginGroup(QCP::msLeft | QCP::msRight, margenes);

    foreach (QCPAxisRect *rect, ui->grafico->axisRects())
    {
      foreach (QCPAxis *axis, rect->axes())
      {
        axis->setLayer("axis");
        axis->grid()->setLayer("grill");
      }
    }

    QCPGraph *espectroTotal = ui->grafico->addGraph(frecuenciasTotal->axis(QCPAxis::atBottom), frecuenciasTotal->axis(QCPAxis::atLeft));
    espectroTotal->data()->set(datosFrec);
    espectroTotal->valueAxis()->setRange(0, 100);
    espectroTotal->rescaleKeyAxis();
    espectroTotal->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black), QBrush(Qt::white), 6));
    espectroTotal->setPen(QPen(QColor(120, 120, 120), 2));

    QCPGraph *espectroIndiv = ui->grafico->addGraph(frecuenciasIndiv->axis(QCPAxis::atBottom), frecuenciasIndiv->axis(QCPAxis::atLeft));
    espectroIndiv->data()->set(datosFrec);
    espectroIndiv->valueAxis()->setRange(0, 100);
    espectroIndiv->rescaleKeyAxis();
    espectroIndiv->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black), QBrush(Qt::white), 6));
    espectroIndiv->setPen(QPen(QColor(120, 120, 120), 2));
    espectroIndiv->rescaleAxes();

    /*
    QCPGraph *subGraf = ui->grafico->addGraph();
    ui->grafico->graph(0)->setData(x,y);
    QCPGraph *porcion = ui->grafico->addGraph();
    ui->grafico->xAxis->setLabel("Frec");
    ui->grafico->yAxis->setLabel("Módulo");
    ui->grafico->xAxis->setRange(900, 9000);
    ui->grafico->yAxis->setRange(0, 100);
    ui->grafico->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->grafico->yAxis->grid()->setVisible(true);

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

    ui->grafico->replot();*/



}

Widget::~Widget()
{
    if(arduino->isOpen()) arduino->close();
    delete ui;
}

void Widget::leerPuertoSerie(){
    int pos, otro, otro2, j;
    QString valor = 0, todo;
    QTime tiempo;
    int tamano;
    bool esdig=false;
    QChar frecu[10];
    len = 1024;
    float muestrixs[tamFFT];
    float espectrix[tamFFT] = {0};
	std::complex<float> espec[tamFFT/2], unImag(0,1);
    //static int cont = 0;
    ffft::FFTReal <float> fft_object (tamFFT);
    //qDebug() << tiempo.elapsed();
    valor = "";
    datosSerie = arduino->readAll();    
    bufferSerie += QString::fromStdString(datosSerie.toStdString());   //aca pa delante nuevo
    tamano = espectro.size();
    todo = espectro + bufferSerie;/*
    if(todo.contains("FA")  && !handShake){
        otro = todo.indexOf("FA");
        otro2 = todo.indexOf("AF", otro+1);
        if(otro2>=0) pos = otro2;
        else pos = todo.size();

        if(otro>=0){
            if(otro>=espectro.size()){
                bufferSerie.remove(otro-espectro.size() , pos-espectro.size()-1);
            } else{
                if(otro2>=espectro.size()){
                    espectro.chop(espectro.size()-1-otro);
                } else{
                    espectro.remove(otro, espectro.size()-otro2);
                }
            }
        }
        //if(tamano>0)
        j=0;
        for(int i=otro+2 ; i<pos; i++){
            FCaltaR[j]=todo[i];
            j++;
        }

        if(pos == otro2){
            handShake = true;
            ui->Statu->setText(FCaltaR);
            FCaltaR.clear();
        }
    } else */
    if(todo.contains("AF") && !handShake){
        otro = todo.indexOf("FA");
        pos = todo.indexOf("AF", otro+1);
        if(otro>=espectro.size()){
            bufferSerie.remove(otro-espectro.size() , pos-espectro.size()-1);
        } else{
            if(pos>=espectro.size()){
                espectro.chop(espectro.size()-otro);
                bufferSerie.remove(0,pos-espectro.size()+1);
            } else{
                espectro.remove(otro, espectro.size()-pos);
            }
        }

        //otro2 = FCaltaR.size();
        for(int i=0 ; i<(pos-otro-2); i++)
            FCaltaR[i]=todo[otro + i + 2];
        ui->Statu->setText(FCaltaR);
        handShake = true;
        FCaltaR.clear();
    }

    pos = bufferSerie.indexOf("_");

    if(pos>=0){

            espectro += bufferSerie.left(pos);
            armandoEspectro = true;

            for(int i=0;i<espectro.size()-1;i++){
                if (espectro[i]>95)armandoEspectro = false;
            }
            QStringList frecuencias = espectro.split(",");
            tamano = frecuencias.size();
            if(tamano!=tamFFT+1) armandoEspectro = false;
            //if(espectro.size() > 512) armandoEspectro = false;
            if(armandoEspectro){


            espectro = bufferSerie.right(bufferSerie.size()-pos-1);
            bufferSerie.clear();
            for(int i=0;i<tamFFT;i++) {
                esdig=true;
                for(j = 0;j<frecuencias.at(i).size();j++)
                    if(!frecuencias.at(i)[j].isDigit() && frecuencias.at(i)[j] != '.' && frecuencias.at(i)[j] != '-') esdig = false;

                if(esdig && i<tamFFT && frecuencias.at(i) < 600)muestrixs[i]=frecuencias.at(i).toDouble();//(2^1);//     /2
                else if (i==0)
                    muestrixs[0] = 0;
                else
                    muestrixs[i] = muestrixs[i-1];
                otro = i;
           }
            frecuencias.clear();
            if(otro==tamFFT-1)
                fft_object.do_fft(espectrix,muestrixs);
            for (int i =1;i<tamFFT/2-1;i++)
                espec[i] = espectrix[i] + espectrix[tamFFT/2+i]* unImag ;

            espec[0]=espectrix[0];
            espec[tamFFT/2]=espectrix[tamFFT/2];
            for (int i=0; i<tamFFT/2; i++)
                y[i] = std::abs(espec[i]);//14; // div 5 para entradas con 1.0 max

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

void Widget::on_setFiltro_clicked()
{
    //int i=0;
    QByteArray datos;
    QString cadena = ui->frecFPA->text();
    FCaltaE = ui->frecFPA->text().toInt();
    handShake = false;
    datos.append("FA");
    datos.append(cadena);
    datos.append("FA");
    arduino->write(datos);
    FCaltaR.clear();
}
