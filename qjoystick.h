#ifndef QJOYSTICK_H
#define QJOYSTICK_H

#include <QObject>
#include <SDL/SDL.h>
#include <QDebug>
class QJoystick : public QObject
{
  Q_OBJECT
public:
  explicit QJoystick(QObject *parent = 0);
  ~QJoystick();

  //Returns number of available joysticks
  static int availableJoysticks();

  //Returns name of joystick
  static QString joystickName(int id);

  //Returns index of joystick
  int currentJoystick();

  //Return number of joystick axes and buttons
  int joystickNumAxes();
  int joystickNumButtons();

  //Holds info about axix and buttons
  QList<qint16> axis;
  QList<bool> buttons;

  QList<qint16> axisPrevious;
  QList<bool> buttonsPrevious;

  bool setJoystick(int jsNumber);

signals:
  void buttonChanged(int id, bool state);
  void axisChanged(int id, qint16 value);
public slots:
  //Reads data from joystick (needs to be done in some time intervals)
  bool getData();


private:
  SDL_Joystick* m_joystick;

};

#endif // QJOYSTICK_H
