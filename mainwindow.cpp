#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * MainWindow to główne okno programu, znajdują sie w nim ...
 * Opis:
 *  Na początku tworzone są obiekty : 2 Joypady, którym nie są przypisane żadne joypady w systemie,
 *  Przypisać joypad można tylko raz wchodząc w settings window
 *
 *  W kolejnym kroku tworzone są timery które odliczają czas po którym następuje uaktualnienie stanu
 *  guzików i osi w każdym z joypadów,
 *  Joypad wykrywa, że jakaś oś/guzik uległa/uległ zmianie i emitują sygnały axisChanged, buttonChanged
 *
 *  Następnie tworzone są timery króre po przepełnieniu uaktywniają wysłanie danych przez sieć do manipulatora
 *  i Łazika
 *
 *  Następnie tworzone są obiekty Manipulaotra i Łazika dziedzicące po ModbusClient, które interpretują komendy
 *  z joypadów, obiekty te odpowiedzalne są również za połączenie.
 *
 *  Następnie łączone są sygnały z settingsWindow które uaktywniają połączenie, oraz wyświetlają jego status
 *
 *  Następnie łączone są sygnały zmiany osi/przycisku z elementami w UI odpowiedzialnymi za wyświetlanie stanu
 *  joypada
 *
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  myTcpSocket = NULL;
  myJoystickRover = new QJoystick(this);
  myJoystickManipulator = new QJoystick(this);

  //Timer for refreshing joypadData inside this program
  joypadRoverRefresh = new QTimer(this);
  joypadManipulatorRefresh = new QTimer(this);
  connect(joypadRoverRefresh,SIGNAL(timeout()),myJoystickRover,SLOT(getData()));
  connect(joypadManipulatorRefresh,SIGNAL(timeout()),myJoystickManipulator,SLOT(getData()));

  ui->setupUi(this);

  //Manipulator and rover objects that inherits from ModbusServer
  myRover = new Rover(this);
  myManipulator = new Manipulator(this);

  //Rover
  connect(this->ui->pushButtonSelectRoverJoypad,SIGNAL(clicked()),this,SLOT(createRoverJoystick()));
  connect(this->myRover,SIGNAL(statusConnectedChanged(bool)),this,SLOT(setStatusDiodeRover(bool)));
  connect(this->ui->pushButtonConnectRover,SIGNAL(clicked()),this,SLOT(connectToHostRover()));
  connect(this->ui->pushButtonDisconnectRover,SIGNAL(clicked()),this,SLOT(disconnectFromHostRover()));
  //Manipulator
  connect(this->ui->pushButtonSelectManipulatorJoypad,SIGNAL(clicked()),this,SLOT(createManipulatorJoystick()));
  connect(this->myManipulator,SIGNAL(statusConnectedChanged(bool)),this,SLOT(setStatusDiodeManipulator(bool)));
  connect(this->ui->pushButtonConnectManipulator,SIGNAL(clicked()),this,SLOT(connectToHostManipulator()));
  connect(this->ui->pushButtonDisconnectManipulator,SIGNAL(clicked()),this,SLOT(disconnectFromHostManipulator()));

  //Show data on joypadRoverWidget
  connect(myJoystickRover,SIGNAL(axisChanged(int,qint16)),this->ui->widgetJoypadRover,SLOT(changeAxisStat(int,qint16)));
  connect(myJoystickRover,SIGNAL(buttonChanged(int,bool)),this->ui->widgetJoypadRover,SLOT(changeButtonState(int,bool)));

  //Show data on joypadManipulatorWidget
  connect(myJoystickManipulator,SIGNAL(axisChanged(int,qint16)),this->ui->widgetJoypadManipulator,SLOT(changeAxisStat(int,qint16)));
  connect(myJoystickManipulator,SIGNAL(buttonChanged(int, bool)),this->ui->widgetJoypadManipulator,SLOT(changeButtonState(int,bool)));

  //Rover sending and reading data via modbus (time intervals)
  roverRefresh = new QTimer(this);
  connect(roverRefresh,SIGNAL(timeout()),myRover,SLOT(updateRoverData()));

  //Manipulator sending data via modbus (time intervals)
  manipulatorRefresh = new QTimer(this);
  connect(manipulatorRefresh,SIGNAL(timeout()),myManipulator,SLOT(updateManipulatorData()));

  //Send data about changed Axis/Button state from joypad to Rover
  connect(myJoystickRover,SIGNAL(axisChanged(int,qint16)),myRover,SLOT(interpretJoypadAxis(int,qint16)));
  connect(myJoystickRover,SIGNAL(buttonChanged(int,bool)),myRover,SLOT(interpretJoypadButton(int,bool)));

  //Send data about changed Axis/Button state from joypad to Manipulator
  connect(myJoystickManipulator,SIGNAL(axisChanged(int,qint16)),myManipulator,SLOT(interpretJoypadAxis(int,qint16)));
  connect(myJoystickManipulator,SIGNAL(buttonChanged(int,bool)),myManipulator,SLOT(interpretJoypadButton(int,bool)));

  //Connect to gps and Motor Drivers displays
  connect(myRover,SIGNAL(roverDataUpdated()),this,SLOT(updateRoverDisplayData()));

  //Connect to Manipulator Status
  connect(myManipulator,SIGNAL(manipulatorDataUpdated()),this,SLOT(updateManipulatorDisplayData()));

  initManipulatorAxisDisplayData();
  initRoverDisplayData();
  setDisplayStyle();

  for(int i=0;i<QJoystick::availableJoysticks();i++){
    ui->comboBoxRoverJoystick->addItem(QJoystick::joystickName(i));
    ui->comboBoxManipulatorJoystick->addItem(QJoystick::joystickName(i));
  }

  //TEMP
  tempTimer = new QTimer(this);
  connect(tempTimer,SIGNAL(timeout()),this,SLOT(updateManipulatorDisplayData()));
  connect(tempTimer,SIGNAL(timeout()),this,SLOT(updateRoverDisplayData()));
  tempTimer->start(200);

  //GPS
  connect(this->ui->pushButtonResetGPS,SIGNAL(clicked()),this->ui->widgetGPS,SLOT(reset()));
  connect(this->ui->pushButtonGetGPSRover,SIGNAL(clicked()),this,SLOT(getGPS()));
  connect(this->ui->pushButtonGetGPSUser,SIGNAL(clicked()),this,SLOT(readGPSFromUser()));
}

void MainWindow::connectToHostManipulator(){
  if(myManipulator->connectToModbusServer(ui->lineEditIPManipulator->text(), ui->lineEditPortManipulator->text().toInt()))
    manipulatorRefresh->start(500);
}

void MainWindow::connectToHostRover(){
  if(myRover->connectToModbusServer(ui->lineEditIPRover->text(), ui->lineEditPortRover->text().toInt()))
    roverRefresh->start(10);
}

void MainWindow::disconnectFromHostManipulator(){
  myManipulator->disconnectFromModbusServer();
  manipulatorRefresh->stop();
}

void MainWindow::disconnectFromHostRover(){
  myRover->disconnectFromModbusServer();
  roverRefresh->stop();
}

MainWindow::~MainWindow(){
  delete ui;
}
// Is done only once
void MainWindow::createManipulatorJoystick(){
  myJoystickManipulator->setJoystick(this->ui->comboBoxManipulatorJoystick->currentIndex());
  this->ui->comboBoxManipulatorJoystick->setDisabled(true);
  this->ui->pushButtonSelectManipulatorJoypad->setDisabled(true);
  joypadManipulatorRefresh->start(5);
}
// Is done only once
void MainWindow::createRoverJoystick(){
  myJoystickRover->setJoystick(this->ui->comboBoxRoverJoystick->currentIndex());
  this->ui->comboBoxRoverJoystick->setDisabled(true);
  this->ui->pushButtonSelectRoverJoypad->setDisabled(true);
  joypadRoverRefresh->start(5);
}
void MainWindow::updateManipulatorDisplayData(){
  //Connection stats
  this->ui->lineEditManipulatorRequestNumber->setText(QString::number(myManipulator->getRequestNumber()));
  this->ui->lineEditManipulatorResponseNumber->setText(QString::number(myManipulator->getGoodResponseNumber()));
  this->ui->lineEditManipulatorBadResponseNumber->setText(QString::number(myManipulator->getRequestNumber()-myManipulator->getGoodResponseNumber()));

  quint16 value;
  myManipulator->getRegister(0,value);
  this->ui->progressBarManipulatorAxis0->setValue(value);
  myManipulator->getRegister(1,value);
  this->ui->progressBarManipulatorAxis1->setValue(value);
  myManipulator->getRegister(2,value);
  this->ui->progressBarManipulatorAxis2->setValue(value);
  myManipulator->getRegister(3,value);
  this->ui->progressBarManipulatorAxis3->setValue(value);
  myManipulator->getRegister(4,value);
  this->ui->progressBarManipulatorAxis4->setValue(value);
  myManipulator->getRegister(5,value);
  this->ui->progressBarManipulatorAxis5->setValue(qint16(value));
}
void MainWindow::initRoverDisplayData(){
  this->ui->progressBarLeftPWM->setMaximum(1000);
  this->ui->progressBarLeftPWM->setMinimum(-1000);

  this->ui->progressBarRightPWM->setMaximum(1000);
  this->ui->progressBarRightPWM->setMinimum(-1000);
}

void MainWindow::initManipulatorAxisDisplayData(){
  quint16 value;
  myManipulator->getRegister(0,value);
  this->ui->progressBarManipulatorAxis0->setMinimum(Manipulator::axisMin[0]);
  this->ui->progressBarManipulatorAxis0->setMaximum(Manipulator::axisMax[0]);
  this->ui->progressBarManipulatorAxis0->setValue(value);
  this->ui->progressBarManipulatorAxis0->setStyleSheet("QProgressBar::chunk { background-color: #00ffff}");

  myManipulator->getRegister(1,value);
  this->ui->progressBarManipulatorAxis1->setMinimum(Manipulator::axisMin[1]);
  this->ui->progressBarManipulatorAxis1->setMaximum(Manipulator::axisMax[1]);
  this->ui->progressBarManipulatorAxis1->setValue(value);
  this->ui->progressBarManipulatorAxis1->setStyleSheet("QProgressBar::chunk { background-color: #00ffff}");

  myManipulator->getRegister(2,value);
  this->ui->progressBarManipulatorAxis2->setMinimum(Manipulator::axisMin[2]);
  this->ui->progressBarManipulatorAxis2->setMaximum(Manipulator::axisMax[2]);
  this->ui->progressBarManipulatorAxis2->setValue(value);
  this->ui->progressBarManipulatorAxis2->setStyleSheet("QProgressBar::chunk { background-color: #00ffff}");

  myManipulator->getRegister(3,value);
  this->ui->progressBarManipulatorAxis3->setMinimum(Manipulator::axisMin[3]);
  this->ui->progressBarManipulatorAxis3->setMaximum(Manipulator::axisMax[3]);
  this->ui->progressBarManipulatorAxis3->setValue(value);
  this->ui->progressBarManipulatorAxis3->setStyleSheet("QProgressBar::chunk { background-color: #00ffff}");

  myManipulator->getRegister(4,value);
  this->ui->progressBarManipulatorAxis4->setMinimum(Manipulator::axisMin[4]);
  this->ui->progressBarManipulatorAxis4->setMaximum(Manipulator::axisMax[4]);
  this->ui->progressBarManipulatorAxis4->setValue(value);
  this->ui->progressBarManipulatorAxis4->setStyleSheet("QProgressBar::chunk { background-color: #00ff00}");

  myManipulator->getRegister(5,value);
  this->ui->progressBarManipulatorAxis5->setMinimum(Manipulator::axisMin[5]);
  this->ui->progressBarManipulatorAxis5->setMaximum(Manipulator::axisMax[5]);
  this->ui->progressBarManipulatorAxis5->setValue(value);
  this->ui->progressBarManipulatorAxis5->setStyleSheet("QProgressBar::chunk { background-color: #ffff00}");
}
void MainWindow::updateRoverDisplayData(){

  //Connection stats
  this->ui->lineEditRoverRequestNumber->setText(QString::number(myRover->getRequestNumber()));
  this->ui->lineEditRoverResponseNumber->setText(QString::number(myRover->getGoodResponseNumber()));
  this->ui->lineEditRoverBadResponseNumber->setText(QString::number(myRover->getRequestNumber()-myRover->getGoodResponseNumber()));

  //PWM Values:
  this->ui->progressBarLeftPWM->setValue((qint16)myRover->getLeftPWM());
  this->ui->progressBarRightPWM->setValue((qint16)myRover->getRightPWM());
  //Motor 16
  {
    quint16 value;
    myRover->getRegister(1,value); //5V
    this->ui->progressBarMotor16_5V->setValue(value);
    myRover->getRegister(2,value); //12V
    this->ui->progressBarMotor16_12V->setValue(value);
    myRover->getRegister(3,value); //24V
    this->ui->progressBarMotor16_24V->setValue(value);
    myRover->getRegister(4,value); //AccV
    this->ui->progressBarMotor16_AccV->setValue(value);
    myRover->getRegister(5,value); //Current
  }
  //Motor 17
  {
    quint16 value;
    myRover->getRegister(6,value); //5V
    this->ui->progressBarMotor17_5V->setValue(value);
    myRover->getRegister(7,value); //12V
    this->ui->progressBarMotor17_12V->setValue(value);
    myRover->getRegister(8,value); //24V
    this->ui->progressBarMotor17_24V->setValue(value);
    myRover->getRegister(9,value); //AccV
    this->ui->progressBarMotor17_AccV->setValue(value);
    myRover->getRegister(10,value); //Current
  }
  //Motor 18
  {
    quint16 value;
    myRover->getRegister(11,value); //5V
    this->ui->progressBarMotor18_5V->setValue(value);
    myRover->getRegister(12,value); //12V
    this->ui->progressBarMotor18_12V->setValue(value);
    myRover->getRegister(13,value); //24V
    this->ui->progressBarMotor18_24V->setValue(value);
    myRover->getRegister(14,value); //AccV
    this->ui->progressBarMotor18_AccV->setValue(value);
    myRover->getRegister(15,value); //Current
  }
  //Motor 19
  {
    quint16 value;
    myRover->getRegister(16,value); //5V
    this->ui->progressBarMotor19_5V->setValue(value);
    myRover->getRegister(17,value); //12V
    this->ui->progressBarMotor19_12V->setValue(value);
    myRover->getRegister(18,value); //24V
    this->ui->progressBarMotor19_24V->setValue(value);
    myRover->getRegister(19,value); //AccV
    this->ui->progressBarMotor19_AccV->setValue(value);
    myRover->getRegister(20,value); //Current
  }
  //Motor 20
  {
    quint16 value;
    myRover->getRegister(21,value); //5V
    this->ui->progressBarMotor20_5V->setValue(value);
    myRover->getRegister(22,value); //12V
    this->ui->progressBarMotor20_12V->setValue(value);
    myRover->getRegister(23,value); //24V
    this->ui->progressBarMotor20_24V->setValue(value);
    myRover->getRegister(24,value); //AccV
    this->ui->progressBarMotor20_AccV->setValue(value);
    myRover->getRegister(25,value); //Current
  }
  //Motor 21
  {
    qDebug()<<"Motor 21";
    quint16 value;
    myRover->getRegister(26,value); //5V
    this->ui->progressBarMotor21_5V->setValue(value);
    myRover->getRegister(27,value); //12V
    this->ui->progressBarMotor21_12V->setValue(value);
    myRover->getRegister(28,value); //24V
    this->ui->progressBarMotor21_24V->setValue(value);
    myRover->getRegister(29,value); //AccV
    this->ui->progressBarMotor21_AccV->setValue(value);
  }

  checkTelemetryValues();

}
void MainWindow::setDisplayStyle(){
  this->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 5px; };");
}
//Checks if motor driver volages and current arent too big and warn operator
void MainWindow::checkTelemetryValues(){
  const int value5V = 5200;
  const int value12V = 13000;
  const int value24V = 25000;
  const int valueAccV = 26000;
  //5V
  {
    if(this->ui->progressBarMotor16_5V->value()>value5V)
      this->ui->progressBarMotor16_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor16_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor17_5V->value()>value5V)
      this->ui->progressBarMotor17_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor17_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor18_5V->value()>value5V)
      this->ui->progressBarMotor18_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor18_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor19_5V->value()>value5V)
      this->ui->progressBarMotor19_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor19_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor20_5V->value()>value5V)
      this->ui->progressBarMotor20_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor20_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor21_5V->value()>value5V)
      this->ui->progressBarMotor21_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor21_5V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");
  }

  //12V
  {
    if(this->ui->progressBarMotor16_12V->value()>value12V)
      this->ui->progressBarMotor16_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor16_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor17_12V->value()>value12V)
      this->ui->progressBarMotor17_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor17_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor18_12V->value()>value12V)
      this->ui->progressBarMotor18_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor18_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor19_12V->value()>value12V)
      this->ui->progressBarMotor19_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor19_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor20_12V->value()>value12V)
      this->ui->progressBarMotor20_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor20_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor21_12V->value()>value12V)
      this->ui->progressBarMotor21_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor21_12V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

  }
  //24V
  {
    if(this->ui->progressBarMotor16_24V->value()>value24V)
      this->ui->progressBarMotor16_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor16_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor17_24V->value()>value24V)
      this->ui->progressBarMotor17_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor17_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor18_24V->value()>value24V)
      this->ui->progressBarMotor18_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor18_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor19_24V->value()>value24V)
      this->ui->progressBarMotor19_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor19_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor20_24V->value()>value24V)
      this->ui->progressBarMotor20_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor20_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

    if(this->ui->progressBarMotor21_24V->value()>value24V)
      this->ui->progressBarMotor21_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor21_24V->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #aaaaff}");

  }
  //AccV
  {
    if(this->ui->progressBarMotor16_AccV->value()>valueAccV)
      this->ui->progressBarMotor16_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor16_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor17_AccV->value()>valueAccV)
      this->ui->progressBarMotor17_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor17_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor18_AccV->value()>valueAccV)
      this->ui->progressBarMotor18_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor18_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor19_AccV->value()>valueAccV)
      this->ui->progressBarMotor19_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor19_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor20_AccV->value()>valueAccV)
      this->ui->progressBarMotor20_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor20_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

    if(this->ui->progressBarMotor21_AccV->value()>valueAccV)
      this->ui->progressBarMotor21_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #ff0000;}");
    else
      this->ui->progressBarMotor21_AccV->setStyleSheet("QProgressBar::chunk {  border-radius: 5px; background-color: #4444ff}");

  }

}
//Settings
//Rover
void MainWindow::setStatusDiodeRover(bool status){
  this->ui->statusDiodeRover->setStatus(status);
}
//Manipulator
void MainWindow::setStatusDiodeManipulator(bool status){
  this->ui->statusDiodeManipulator->setStatus(status);
}
void MainWindow::readGPSFromUser(){
  bool ok;
    double latitude = ui->lineEditLatitudeDegrees->text().toDouble(&ok);
  if(ok){
    latitude += ui->lineEditLatitudeMinutes->text().toDouble(&ok)/60;
  }
  if(ok){
    latitude +=ui->lineEditLatitudeSeconds->text().toDouble(&ok)/3600;
  }
  if(ok){
    if(ui->lineEditLatitudeDesignator->text().data()[0] == 'S')
      latitude *= (-1);
    else if((QChar)ui->lineEditLatitudeDesignator->text().data()[0] == 'N')
      ;
    else
      ok = false;
  }
  double longitude;
  if(ok){
    longitude = ui->lineEditLongitudeDegrees->text().toDouble(&ok);
  }
  if(ok){
    longitude += ui->lineEditLongitudeMinutes->text().toDouble(&ok)/60;
  }
  if(ok){
    longitude +=ui->lineEditLongitudeSeconds->text().toDouble(&ok)/3600;
  }
  if(ok){
    if(ui->lineEditLongitudeDesignator->text().data()[0] == 'W')
      longitude *= (-1);
    else if(ui->lineEditLongitudeDesignator->text().data()[0] != 'E')
      ok = false;
  }
  QGeoCoordinate userCoordinate;
  if(ok){
    qDebug()<<"ITS OK";
    qDebug()<<"LAT"<<latitude;
    qDebug()<<"LONG"<<longitude;
    userCoordinate.setLatitude(latitude);
    userCoordinate.setLongitude(longitude);
    ui->widgetGPS->setTargetCoordinate(userCoordinate);
  }
  ui->widgetGPS->update();
}
void MainWindow::readFromGPS(){
  QByteArray array = myTcpSocket->readAll();
  //TO DO
  qDebug()<<array;
  if(array[18] == 'A'){

    qDebug()<<"Good Frame";
    QGeoCoordinate coordinate;
    //Latitude display and write into coordinate
    double degrees = (array[20]-48)*10+(array[21]-48);
    ui->labelGPSLatitudeActualDegrees->setText(QString::number(degrees));
    double minutes = (array[22]-48)*10+(array[23]-48)+(array[25]-48)*0.1+(array[26]-48)*0.01+(array[27]-48)*0.001+(array[28]-48)*0.001;
    int minutesInt = minutes;
    ui->labelGPSLatitudeActualMinutes->setText(QString::number(minutesInt));
    double secondsDouble = (minutes-minutesInt)*60;
    ui->labelGPSLatitudeActualSecond->setText(QString::number(secondsDouble));
    char designator = array[30];
    ui->labelLatitudeDesignator->setText(QString(designator));
    double value;
    if(designator == 'N')
      value = degrees+minutes/60;
    if(designator == 'S')
      value = (-1)*(degrees +minutes/60);
    coordinate.setLatitude(value);

    //Longnitude display and write into coordinate
    degrees = (array[32]-48)*100+(array[33]-48)*10+(array[34]-48);
    ui->labelGPSLongitudeActualDegrees->setText(QString::number(degrees));
    minutes = (array[35]-48)*10+(array[36]-48)+(array[38]-48)*0.1+(array[39]-48)*0.01+(array[40]-48)*0.001+(array[41]-48)*0.001;
    minutesInt = minutes;
    ui->labelGPSLongitudeActualMinutes->setText(QString::number(minutesInt));
    secondsDouble = (minutes-minutesInt)*60;
    ui->labelGPSLongitudeActualSecond->setText(QString::number(secondsDouble));
    designator = array[43];
    ui->labelLongitudeDesignator->setText(QString(designator));

    if(designator == 'E')
      value = degrees+minutes/60;
    if(designator == 'W')
      value = (-1)*(degrees +minutes/60);
    coordinate.setLongitude(value);

    ui->widgetGPS->addCoordinate(coordinate);
  }


  myTcpSocket->disconnect();
  myTcpSocket->deleteLater();
  myTcpSocket = NULL;
}

void MainWindow::getGPS(){
  if(myTcpSocket != NULL)
    myTcpSocket->deleteLater();
  myTcpSocket = new QTcpSocket(this);
  connect(this->myTcpSocket,SIGNAL(readyRead()),this,SLOT(readFromGPS()));
  myTcpSocket->connectToHost("192.168.1.100", 3000);
}
