#include "rover.h"

//Joypad axis
const int Rover::leftAxis = 1;
const int Rover::rightAxis = 4;
const int Rover::numberOfSpeedButtons = 8;
const int Rover::speedButtonsIDTable [] = {1,2,3,4,5,6,7,8};
const int Rover::speedButtonsSpeedsTable[] = {50,200,600,1000,-50,-200,-600,-1000};
Rover::Rover(QObject *parent ) :
  ModbusClient(parent)
{
  goodResponseCounter = 0;
  requestCounter = 0;
  connect(this,SIGNAL(transactionFinished(bool,qint8)),this,SLOT(proceedResponse(bool,qint8)));
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
  qDebug()<<"LEFT"<<value;
}
void Rover::setRightPWM(qint16 value){
  setRegister(33, value);
  qDebug()<<"RIGHT"<<value;
}

void Rover::sendRoverData(){

  writeMultipleRegisters(32,2);
}
void Rover::readRoverData(){

  //Read Voltage and Current from motor drivers
  readHoldingRegisters(1,30);

}
void Rover::readGPSData(){
  readHoldingRegisters(40,21);
}

void Rover::updateRoverData(){
  if(isWaitingForResponse)
    return;
  requestCounter ++;
  if(requestCounter % 3 == 0){
    sendRoverData();
  }
  else if (requestCounter % 3 == 1){
    readRoverData();
  }
  else
    readGPSData();
}

void Rover::interpretJoypadButton(int id, bool status){
  if(status == true){
    for(int i=0; i<numberOfSpeedButtons; i ++){
      if(id == speedButtonsIDTable[i]){
        setLeftPWM(speedButtonsSpeedsTable[i]);
        setRightPWM(speedButtonsSpeedsTable[i]);
      }
    }
  }
  else{
    setLeftPWM(0);
    setRightPWM(0);
  }
}


//Set speed and direction of each engine
void Rover::interpretJoypadAxis(int id, qint16 value){
  if(id==leftAxis){
    qint16 valuePWM = ((double)value/std::numeric_limits<quint16>::max()) * 1000;
    setLeftPWM(valuePWM);
  }
  if(id==rightAxis){
    qint16 valuePWM = ((double)value/std::numeric_limits<quint16>::max()) * 1000;
    setRightPWM(valuePWM);
  }
}
void Rover::proceedResponse(bool status, qint8 errorCode){
  if(status){
    goodResponseCounter++;
  }
  else
    qDebug()<<"BAD RESPONSE Rover Modbus"; //We can add errorCode message
  emit roverDataUpdated();
}
