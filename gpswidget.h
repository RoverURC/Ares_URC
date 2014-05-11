#ifndef GPSWIDGET_H
#define GPSWIDGET_H

#include <QGLWidget>

class GPSWidget : public QGLWidget
{
  Q_OBJECT
public:
  explicit GPSWidget(QObject *parent = 0);

signals:

public slots:

private:
  QGraphicsView;
  QGraphicsScene;
};

#endif // GPSWIDGET_H
