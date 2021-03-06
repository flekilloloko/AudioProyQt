#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui
{
class Widget;
}

class QSerialPort;

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
private slots:
    void leerPuertoSerie();

    void on_setFiltro_clicked();

private:
    Ui::Widget *ui;
    QSerialPort *arduino;
    static const quint16 arduino_due_vendor_id = 9025;
    static const quint16 arduino_due_product_id = 61;
    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray datosSerie;
    QString bufferSerie, FCaltaR;
    bool sincronizado, armandoEspectro;
    QVector<double> x, y;
    int punteroX;
    int indice;
    int len;
    bool handShake;
    int FCaltaE;//, FCalta;
    QString espectro;



    //QVector <double> componentes;


};

#endif // WIDGET_H
