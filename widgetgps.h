#ifndef WIDGETGPS_H
#define WIDGETGPS_H

#include <QGraphicsView>
#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <QGeoLocation>

#include <QGraphicsScene>

#include <QTcpSocket>
class MyPoint{
public:
  MyPoint(double x,double y);
  double x;
  double y;
  static MyPoint calculateXY(QGeoCoordinate coordinate);
};

class WidgetGPS : public QGraphicsView
{
  Q_OBJECT
public:
  explicit WidgetGPS(QWidget *parent = 0);
signals:

public slots:
  void reset();

  void addCoordinate(QGeoCoordinate coordinate);
  void setTargetCoordinate(QGeoCoordinate coordinate);

protected:
  void paintEvent(QPaintEvent *event);
private:
  QTcpSocket *myTcpSocket;
  QList<QGeoCoordinate> coordinatesList;
  QGeoCoordinate targetCoordinate;
  QGraphicsScene *myScene;
};

#endif // WIDGETGPS_H
