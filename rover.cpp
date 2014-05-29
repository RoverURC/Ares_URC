#include "rover.h"

//Joypad axis
Rover::Rover(QObject *parent ) :
  ModbusClient(parent)
{
  lastAxisValue = 0;
  axisStatus = new bool[5];
  for(int i=0 ; i<5 ; i++){
      axisStatus[i] = false;
  }
  goodResponseCounter = 0;
  requestCounter = 0;

  connect(this,SIGNAL(transactionFinished(bool,qint8)),this,SLOT(proceedResponse(bool,qint8)));
}

Rover::~Rover(){
  delete[] axisStatus;
}

int Rover::getLeftPWM() const{
  quint16 value;
  getRegister(32,value);
  return (qint16)value;
}
int Rover::getRightPWM() const{
  quint16 value;
  getRegister(33,value);
  return (qint16)value;
}

void Rover::setLeftPWM(qint16 value){
  setRegister(32, value);
}
void Rover::setRightPWM(qint16 value){
  setRegister(33, value);
}

void Rover::sendRoverData(){
  writeMultipleRegisters(32,6);
}

void Rover::setManipulatorAxis(int id, int value){
  if(id<0 || id>4)
    return;
  if(value < -1000 || value > 1000)
    return;
  if(id == 4 )
    if(value < 0)
      return;
  setRegister(35, id);
  setRegister(36, value);
  setRegister(37, 1);
}

void Rover::clearManipulatorAxis(){
  setRegister(35, 0);
  setRegister(36, 0);
  setRegister(37, 0);
}

void Rover::readRoverData(){
  //Read Voltage and Current from motor drivers
  readHoldingRegisters(1,30);
}

void Rover::updateRoverData(){
  if(isWaitingForResponse)
    return;
  requestCounter ++;
  if(requestCounter % 5 != 0){
    sendRoverData();
    clearManipulatorAxis();
  }
  else{
      qDebug()<<"READING";
    readRoverData();
    }
  emit roverDataUpdated();
}

void Rover::interpretJoypadButton(int id, bool status){
  if(id<0 || id>6)
    return;
  for(int i=0;i<5;i++)
    axisStatus[i] = false;
  axisStatus[id] = status;
  if(status == true){
    float manipulatorValue = (float)lastAxisValue/32767*(1000);
    setManipulatorAxis(id, manipulatorValue);
  }
}
//Set speed and direction of each engine
void Rover::interpretJoypadAxis(int id, qint16 value){
  if(id == 1){
      lastAxisValue = value;
      //Manipulator
      for(int i=0;i<5;i++)
        if(axisStatus[i]){
            float manipulatorValue = (float)value/32767*(1000);
            setManipulatorAxis(i,manipulatorValue);
            setLeftPWM(0);
            setRightPWM(0);
            return;
          }
      //or Rover
      qint16 valuePWM = (float)value/32767*(-1000);
      setLeftPWM(valuePWM);
      setRightPWM(valuePWM);
    }
  else if(id == 2){
      qint16 valuePWM = (float)value/32767*(1000);
      setLeftPWM(valuePWM);
      setRightPWM((-1)*valuePWM);
    }
}
void Rover::proceedResponse(bool status, qint8 errorCode){
  if(status){
    goodResponseCounter++;
  }
  emit roverDataUpdated();
}
