#include "connectionstatuswidget.h"
#include <QGeoCoordinate>
ConnectionStatusWidget::ConnectionStatusWidget(QWidget *parent) :
  QWidget(parent)
{
  diodeColor = Qt::red;
}
void ConnectionStatusWidget::setStatus(bool status){
  if(status){
    diodeColor = Qt::green;
  }
  else{
    diodeColor = Qt::red;
  }
  repaint();
}
void ConnectionStatusWidget::paintEvent(QPaintEvent *){
  QPainter myPainter;
  myPainter.begin(this);
  myPainter.setRenderHints(QPainter::Antialiasing);
  myPainter.setBrush(diodeColor);
  int radious = qMin(height(), width())/2 -1;
  if(radious<1 )
    radious = 1;
  myPainter.drawEllipse(QPoint(width()/2,height()/2),radious,radious);
}
