#include "widgetgps.h"
#include <QPainter>
#include <QDebug>
WidgetGPS::WidgetGPS(QWidget *parent) :
  QGraphicsView(parent)
{
  coordinatesList.clear();
  myScene = new QGraphicsScene(this);
  this->setScene(myScene);
  this->addCoordinates(QGeoCoordinate(0,0));
  targetCoordinate.setLatitude(0);
  targetCoordinate.setLongitude(0);
}
void WidgetGPS::reset(){
  coordinatesList.clear();
  update();
}
void WidgetGPS::addCoordinates(QGeoCoordinate coordinate){
  coordinatesList.append(coordinate);
  this->update();
}

void WidgetGPS::paintEvent(QPaintEvent *event){
  myScene->deleteLater();
  myScene = new QGraphicsScene();

  for (int i=0 ; i<coordinatesList.size(); i++){
    MyPoint point = calculateXY(coordinatesList[i]);
    myScene->addEllipse(point.x,point.y,0.3,0.3,QPen(Qt::transparent),QBrush(Qt::black));
    //qDebug()<<"PX"<<point.x;
    //qDebug()<<"PY"<<point.y;
  }
  MyPoint point = calculateXY(targetCoordinate);
  myScene->addEllipse(point.x,point.y, 0.3, 0.3,QPen(Qt::transparent),QBrush(Qt::red));
  //qDebug()<<"AX"<<point.x;
  //qDebug()<<"AY"<<point.y;

  setScene(myScene);
  QGraphicsView::fitInView(scene()->sceneRect(), Qt::KeepAspectRatio );
  QGraphicsView::paintEvent(event);
}

void WidgetGPS::resizeEvent(QResizeEvent *){

}

void WidgetGPS::setTargetCoordinate(QGeoCoordinate coordinate){
  targetCoordinate = coordinate;
}
MyPoint WidgetGPS::calculateXY(QGeoCoordinate coordinate){
  MyPoint point(0,0);
  point.x = coordinate.longitude()*10000;
  point.y = coordinate.latitude()*10000;
  return point;
}

MyPoint::MyPoint(double x, double y){
      this->x = x;
      this->y = y;
}
