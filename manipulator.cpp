#include "manipulator.h"

// Axis 0 (Pin PWM 0,1), Axis 1 (Pin PWM 2), Axis 2 (Pin PWM 3), Axis 3 (Pin PWM 4), Axis 4 UART, Axis 5 Motor
int const Manipulator::axisNumber = 6;
int const Manipulator::axisInit[] = { 250, 250, 250, 250, 0, 0};
int const Manipulator::axisMax[] = { 500, 380, 475, 550, 300, 100};
int const Manipulator::axisMin[] = { 150, 100, 100, 100, 0, -100};
int const Manipulator::axisButtonId[] = { 0, 1, 2, 3, 4, 5};

int const motorSpeedButton = 6;
int const motorSpeedToZeroButtonId = 7;
int const motorSpeedIndex = 5;
int const uartButton = 5;
int const uartAngleIndex = 4;

Manipulator::Manipulator(QObject *parent ) :
  ModbusClient(parent)
{

  requestCounter = 0;
  goodResponseCounter = 0;

  axisValues = new qint16[axisNumber];
  axisStatus = new bool[axisNumber];
  for(int i=0; i<axisNumber;i++){
    axisValues[i] = axisInit[i];
    axisStatus[i] = false;
  }
  resetManipulator();

  actualJoypadAxisValue = 0;

  incrementAxisTimer = new QTimer(this);
  incrementAxisTimer->start(50);
  connect(incrementAxisTimer,SIGNAL(timeout()),this,SLOT(incrementManipulatorAxisValues()));

  connect(this,SIGNAL(transactionFinished(bool,qint8)),this,SLOT(proceedResponse(bool,qint8)));
}
Manipulator::~Manipulator(){
  delete[] axisValues;
  delete[] axisStatus;
}
//TO DO
void Manipulator::incrementManipulatorAxisValues(){

  for(int i=0; i<axisNumber; i++){
    if(axisStatus[i] == true){
      if(axisValues[i] + actualJoypadAxisValue<axisMin[i]){
        axisValues[i] = axisMin[i];
        setRegister(i, axisValues[i]);
        return;
      }
      if(axisValues[i] + actualJoypadAxisValue>axisMax[i]){
        axisValues[i] = axisMax[i];
        setRegister(i, axisValues[i]);
        return;
      }
      axisValues[i] += actualJoypadAxisValue;
      qDebug()<<axisValues[i];
      setRegister(i, axisValues[i]);
      return;
    }
  }
}
void Manipulator::resetManipulator(){
  for(int i=0;i<axisNumber; i++){
    axisValues[i] = axisInit[i];
    setRegister(i, axisInit[i]);
  }
  for(int i=0; i<axisNumber; i++)
    axisStatus[i] = false;
}
void Manipulator::updateManipulatorData(){
  if(isWaitingForResponse)
    return;
  requestCounter ++;

  sendManipulatorData();
  //For now we dont have any read functions
  //...
}

void Manipulator::sendManipulatorData(){

  writeMultipleRegisters(0,6);
}

void Manipulator::interpretJoypadButton(int id, bool status){
  if(status == true)
    for(int i=0; i<axisNumber; i++)
      if(id == axisButtonId[i])
        axisStatus[i] = true;
      else
        axisStatus[i] = false;
  else
    for(int i=0; i<axisNumber; i++)
      axisStatus[i] = false;

  if(status == true && id == motorSpeedToZeroButtonId){
    axisValues[motorSpeedIndex] = 0;
    setRegister(motorSpeedIndex, 0);
  }
}

void Manipulator::interpretJoypadAxis(int id, qint16 value){
  if(id == 1)
    actualJoypadAxisValue = value/20000;
}

void Manipulator::proceedResponse(bool status, qint8 errorCode){
  if(status){
    goodResponseCounter++;
    emit manipulatorDataUpdated();
  }
  else
    qDebug()<<"BAD RESPONSE Manipulator Modbus"; //We can add errorCode message
}

int Manipulator::getAxisValue(int index){
  if(index<0 || index>=axisNumber)
    return 0;
  else
    return axisValues[index];
}
