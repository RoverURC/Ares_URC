#include "widgetgps.h"
#include <QPainter>
#include <QDebug>
WidgetGPS::WidgetGPS(QWidget *parent) :
  QGraphicsView(parent)
{
  myScene = new QGraphicsScene(this);
  setScene(myScene);

  targetCoordinate.setLatitude(0);
  targetCoordinate.setLongitude(0);
}

void WidgetGPS::reset(){
  coordinatesList.clear();
  update();
}
void WidgetGPS::addCoordinate(QGeoCoordinate coordinate){
  coordinatesList.append(coordinate);
  update();
}
void WidgetGPS::paintEvent(QPaintEvent *event){
  myScene->deleteLater();
  myScene = new QGraphicsScene(this);

  for (int i=0 ; i<coordinatesList.size(); i++){
    MyPoint point = MyPoint::calculateXY(coordinatesList[i]);
    myScene->addEllipse(point.x,point.y,0.3,0.3,QPen(Qt::transparent),QBrush(Qt::black));
  }
  MyPoint point = MyPoint::calculateXY(targetCoordinate);
  myScene->addEllipse(point.x,point.y, 0.3,0.3,QPen(Qt::transparent),QBrush(Qt::red));

  setScene(myScene);
  QGraphicsView::fitInView(scene()->sceneRect(), Qt::KeepAspectRatio );
  QGraphicsView::paintEvent(event);
}
void WidgetGPS::setTargetCoordinate(QGeoCoordinate coordinate){
  targetCoordinate = coordinate;
  update();
}
MyPoint MyPoint::calculateXY(QGeoCoordinate coordinate){
  MyPoint point(0,0);
  point.x = coordinate.longitude()*10000;
  point.y = coordinate.latitude()*10000;
  return point;
}
MyPoint::MyPoint(double x, double y){
      this->x = x;
      this->y = y;
}
