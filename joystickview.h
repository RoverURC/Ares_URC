#ifndef JOYSTICKVIEW_H
#define JOYSTICKVIEW_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <limits>
class JoystickView : public QWidget
{
  Q_OBJECT
public:
  explicit JoystickView(QWidget *parent = 0);
  ~JoystickView();
signals:

public slots:

  void changeButtonState(int id, bool status);
  void changeAxisStat(int id, qint16 value);
  //void setAnalog(quint16);
protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
private:

  int unitHeight;
  int unitWidth;
  int horizontalOffset;
  int verticalOffset;

  void calculateUnits();
  int X(int x);
  int Y(int y);

  int numberOfButtons;
  int numberOfAxis;
  bool *buttons; //array of 8 buttons
  qint16 *axis; //array of 4 axis
};

#endif // JOYSTICKVIEW_H
