#include "joystickview.h"

JoystickView::JoystickView(QWidget *parent) :
  QWidget(parent),
  numberOfButtons(8),
  numberOfAxis(5)
{
  calculateUnits();

  buttons = new bool[numberOfButtons];
  for(int i=0 ; i< numberOfButtons ; i++){
    buttons[i] = 0;
  }

  axis = new qint16[numberOfAxis];
  for(int i=0; i<numberOfAxis ; i++){
    axis[i] = 0;
  }
}
JoystickView::~JoystickView(){

}
void JoystickView::calculateUnits(){
  unitHeight = height()/9;
  unitWidth = width()/19;

  unitHeight = qMin(unitHeight,unitWidth);
  unitWidth = unitHeight;
  verticalOffset = (height()%9)/2;
  horizontalOffset = (width()%19)/2;
}
int JoystickView::X(int x){
  return horizontalOffset+x*unitWidth;
}

int JoystickView::Y(int y){
  return verticalOffset+y*unitHeight;
}
void JoystickView::resizeEvent(QResizeEvent *){
  calculateUnits();
  repaint();
}

void JoystickView::paintEvent(QPaintEvent *){
  quint16 maxQUInt16 = std::numeric_limits<quint16>::max();
  QPainter myPainter;
  myPainter.begin(this);
  myPainter.setBrush(Qt::black);
  myPainter.setPen(Qt::blue);
  myPainter.setRenderHints(QPainter::Antialiasing);
  //Analog 1
  //0
  myPainter.drawRect(X(0),Y(4),X(4),Y(1));
  myPainter.drawRect(X(5),Y(4),X(4),Y(1));

  //1
  myPainter.drawRect(X(4),Y(5),X(1),Y(4));
  myPainter.drawRect(X(4),Y(0),X(1),Y(4));

  //Analog 2
  //2
  myPainter.drawRect(X(10),Y(4),X(4),Y(1));
  myPainter.drawRect(X(15),Y(4),X(4),Y(1));

  //3
  myPainter.drawRect(X(14),Y(5),X(1),Y(4));
  myPainter.drawRect(X(14),Y(0),X(1),Y(4));

  myPainter.setBrush(Qt::red);
  if(axis[0]>0){
    myPainter.drawRect(X(5), Y(4), X(4)*axis[0]*2/maxQUInt16, Y(1));
  }
  if(axis[0]<0){
    myPainter.drawRect(X(4), Y(4), X(4)*axis[0]*2/maxQUInt16, Y(1));
  }
  if(axis[1]>0){
    myPainter.drawRect(X(4), Y(5), X(1), Y(4)*axis[1]*2/maxQUInt16);
  }
  if(axis[1]<0){
    myPainter.drawRect(X(4), Y(4), X(1), Y(4)*axis[1]*2/maxQUInt16);
  }

  if(axis[3]>0){
    myPainter.drawRect(X(15), Y(4), X(4)*axis[3]*2/maxQUInt16, Y(1));
  }
  if(axis[3]<0){
    myPainter.drawRect(X(14), Y(4), X(4)*axis[3]*2/maxQUInt16, Y(1));
  }
  if(axis[4]>0){
    myPainter.drawRect(X(14), Y(5), X(1), Y(4)*axis[4]*2/maxQUInt16);
  }
  if(axis[4]<0){
    myPainter.drawRect(X(14), Y(4), X(1), Y(4)*axis[4]*2/maxQUInt16);
  }
  //Triggers

  if(buttons[4])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(6),Y(0),X(3),Y(1));

  if(buttons[6])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(6),Y(2),X(3),Y(1));

  if(buttons[5])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(10),Y(0),X(3),Y(1));

  if(buttons[7])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(10),Y(2),X(3),Y(1));

  //Buttons

  if(buttons[0])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(6),Y(7),X(2),Y(1));

  if(buttons[2])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(11),Y(7),X(2),Y(1));

  if(buttons[3])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(8),Y(6),X(3),Y(1));

  if(buttons[1])
    myPainter.setBrush(Qt::red);
  else
    myPainter.setBrush(Qt::black);
  myPainter.drawRect(X(8),Y(8),X(3),Y(1));
}
void JoystickView::changeButtonState(int id, bool status){
  if(id<0 || id>=numberOfButtons)
    return;
  //qDebug()<<"ID"<<id;
  buttons[id] = status;
  update();
}

void JoystickView::changeAxisStat(int id, qint16 value){
  if(id<0 || id>=numberOfAxis)
    return;
  //qDebug()<<"ID"<<id<<" "<<value;
  axis[id] = value;
  update();
}
