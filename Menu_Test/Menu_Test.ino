#include "Menu_Test_menu.h"
#include <EepromAbstraction.h>

AvrEeprom eeprom;

// pin defs inputs
int storageTankFullPIN = 9;
int storageTankEmptyPIN = 10;
int tankFullPIN = 11;
int powerSensePIN = 12;
int powerRelayPin = 7;

int startAttempts = 0;  // # of times start attempted
int startRetrys = 10;  // Default retrys to 3
int startDelayTime = 2000;  // Wait time after start given

int maxFillTime = 100;  // Max filltime in seconds
int fillTimeCount = 0; // Timer to track fill

// Global Values to track states with defaults till read by SYSTEM
bool generatorRunning = false;  // set by detected line voltage on SENSE LINE
float batteryVoltage = 12.00;
bool storageTankFull = true;
bool storageTankEmpty = false;
bool tankFull = true;
bool fueltankLow = false;

bool faultOccured = false;

bool disableIOPins = true;


enum states { IDLE=0, START=1, READY= 2, FILLING=3, STOPPED=4, FAULT=5 };
enum msgType { OK=0, FAULTED=1};

states systemState = IDLE;

int mainUpdateCycleTime = 1000;  // 1 second = 1000

bool backlightOn = true;
bool blinkLight = true;
int  backlightTimeout = 1800;  // Timeout in seconds
int  backlightTimeoutReset = -3;  // Value to reset backlight
int  backlightTimer = 0;

const int ledPin = LED_BUILTIN;  // constant for the pin we will use
int ledOn = LOW;  // the state of the pin, we will toggle it.

// create an IO abstraction, so later we could put the led on a shift register or i2c.
IoAbstractionRef ioDevice = ioUsingArduino();

void getStartUpState(){

    faultOccured = false;
  
    menuBattery.setFloatValue(batteryVoltage);     // Battery
    menuStorageFull.setBoolean(storageTankFull);   // Main Storage tank full float
    menuStorageEmpty.setBoolean(storageTankEmpty); // Main Storage tank empty float
    menuTankFull.setBoolean(tankFull);             // Stock tank full float
    menuFuelLow.setBoolean(fueltankLow);           // Fuel tank empty float
}

void process(){


  if(!faultOccured){

    switch(systemState){
      case IDLE:{
        // check to see if fill required
        if(storageTankEmpty == true){  // start a fill cycle
          systemState = START;
          Serial.println("Storage Empty - starting cycle");   
        }
        break;
      }
      case START:{
        // Start the generator routine
        startGenerator();
        break;
      }
      case READY:{
        // Wait for voltage sense or retry start
        if(generatorRunning){
          Serial.println("Power Found");  
          applyPowerToPump();  // Turn Pump on 
          systemState = FILLING;
          fillTimeCount = 0;   // start the max fill time count
        }else{
          if(startAttempts < startRetrys){
            systemState = START;
            Serial.print("Retrying Start cycle # ");   
            Serial.println(startAttempts);   
          }
          else{
            systemState = FAULT;      // Failure to start            
            Serial.println("Starting Failed ");   
          }
        }      
        break;
      }
      case FILLING:{
        
        Serial.print("Filling Count = ");           
        Serial.println(fillTimeCount);       
            
        if(storageTankFull)  // Wait for FILL to complete
        {
          systemState = STOPPED; // Move to completed
          Serial.println("Storage Tank FILLED");       
        }else{
          fillTimeCount++;  // keep track of fill time

          // Check power still on or max run time exceeded
          if(fillTimeCount > maxFillTime)
          {
            systemState = FAULT;
            Serial.println("Storage Tank fill time exceeded");       
          }
        }
        
        break;
      }
      case STOPPED:{
        Serial.println("Stop Generator/power");
        stopGenerator();  // Stop generator
        resetSystem();
        break;
      }
      case FAULT:{
        Serial.println("System Fault occured");
        sendSatSignal(FAULTED);    // Send fault signal
        faultOccured = true;
        break;
      }
    }; 
  }
}


void startGenerator(){  // Routine to start the generator
    Serial.println("Starting Generator");  
    // Something to trigger motor start digital or solenoid button push
    delay(startDelayTime);  // wait time for generator start
    systemState = READY;
    startAttempts++;        // increment start attempts
    //generatorRunning = true;  // TEST Sequence
}

void stopGenerator(){
  Serial.println("Stopping Generator");  
  // TODO  put in code to drop power and turn off power
}

void applyPowerToPump(){
  Serial.println("Activated Relay to apply power to pump");  
}

void activateRelay(){
  
}

void sendSatSignal(msgType msg){
  switch(msg){
      case OK:{
        Serial.println("Sending OK signal");
        break;
      }
      case FAULTED:{
        Serial.println("Sending FAULT signal via SAT");
        break;
      }
  }
}

void resetSystem(){
  systemState = IDLE;   // Set state back to idle
  faultOccured = false; // Clear the faults
  fillTimeCount = 0;    // reset fill time counter

  // TODO  set all the states of inputs
}

// update all the sensor values
void getSensorValues(){
  if(!disableIOPins){
    storageTankFull = digitalRead(storageTankFullPIN);    // Storage Tank Full
    storageTankEmpty = digitalRead(storageTankEmptyPIN);  // Storage Tank Empty
    tankFull = digitalRead(tankFullPIN);                  // Stock Tank Full

    generatorRunning = digitalRead(powerSensePIN);        // Power Sense
    
    //storageTankEmpty = true;  // TESTING
    //generatorRunning = true;  // TESTING
    //storageTankFull  = true;  // TESTING
  }
}

void blinkLED(){
  if(blinkLight){
  // now we write to the device, the 'S' version of the method automatically syncs.
  ioDeviceDigitalWriteS(ioDevice, ledPin, ledOn);

  ledOn = !ledOn; // toggle the LED state.
  }  
}

// Main Update Routine.
void mainUpdate() {
  
  getSensorValues();  // update the sensors
  
  process();  // move along the states

  menuMode.setCurrentValue(systemState, false);

  
  //blinkLED();

  if(backlightTimer < 0){
    lcd.backlight();
    backlightOn = true;
    renderer.invalidateAll();
  }
  
  // Timer to shut off backlight
  backlightTimer = backlightTimer + (mainUpdateCycleTime/1000);
  if(backlightTimer > backlightTimeout){
    lcd.noBacklight();
    backlightOn = false;
    renderer.invalidateAll();   
    backlightTimer = 0; 
  }
}

void setup() {
    Serial.begin(115200);

    // Set up all the I/O for project
    pinMode(storageTankFullPIN, INPUT);     // Storage Tank Full
    pinMode(storageTankEmptyPIN, INPUT);    // Storage Tank Empty
    pinMode(tankFullPIN, INPUT);            // Stock Tank Full
    pinMode(powerRelayPin, INPUT);          // Power Relay

    setupMenu();

    //menuMgr.load(eeprom);
    getStartUpState();

    // set the pin we are to use as output using the io abstraction
    ioDevicePinMode(ioDevice, ledPin, OUTPUT);

    // Create Main Update cycle that runs periodically.
    taskManager.scheduleFixedRate(mainUpdateCycleTime, mainUpdate);
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onMotorChanged(int id) {
    backlightTimer = backlightTimeoutReset;
    //Serial.print("Id is ");
    //Serial.println(menuMotor.getCurrentValue());
}

void onDialogFinished(ButtonType btnPressed, void* /*userdata*/) {        
    if(btnPressed == BTNTYPE_OK) {
      //  menuMgr.save(IntEeprom);
      //renderer.takeOverDisplay(exitMenu);
    }
}

void CALLBACK_FUNCTION onGeneratorStart(int id) {
    backlightTimer = backlightTimeoutReset;

    const char* pgmHeaderText PROGMEM = "START";
    bool remoteAllowed = false;
    BaseDialog* dlg = renderer.getDialog();
    dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
    dlg->show(pgmHeaderText, remoteAllowed, onDialogFinished); // true = shows on remote sessions.
    dlg->copyIntoBuffer("Click OK to START");     
}

void CALLBACK_FUNCTION onGeneratorStop(int id) {
    backlightTimer = backlightTimeoutReset;
}

void CALLBACK_FUNCTION onStartTimeChanged(int id) {
    backlightTimer = backlightTimeoutReset;

    WholeAndFraction wf = menuStartTime.getWholeAndFraction();
    int wholePart = wf.whole;
    int fractPart = wf.fraction;
    bool neg = wf.negative;
    //Serial.print("Start Time = ");
    //Serial.println(wholePart);
}

void CALLBACK_FUNCTION onRetrysChanged(int id) {
    backlightTimer = backlightTimeoutReset;

    WholeAndFraction wf = menuRetrys.getWholeAndFraction();
    int wholePart = wf.whole;
    int fractPart = wf.fraction;
    bool neg = wf.negative;
    //Serial.print("Retrys = ");
    //Serial.println(wholePart);
}

void CALLBACK_FUNCTION onPumpRelayChanged(int id) {
    backlightTimer = backlightTimeoutReset;
    applyPowerToPump();  // Activate Pump Relay
}

void CALLBACK_FUNCTION onSatMsgChanged(int id) {
    backlightTimer = backlightTimeoutReset;
    sendSatSignal(1);  // Send a test msg via sat
}

void CALLBACK_FUNCTION onDateChanged(int id) {
    backlightTimer = backlightTimeoutReset;
    DateStorage d =  menuDate.getDate();

    //Serial.print(d.month);
    //Serial.print("/");
    //Serial.print(d.day);
    //Serial.print("/");
    //Serial.print(d.year);
}

void CALLBACK_FUNCTION onTimeChanged(int id) {
    backlightTimer = backlightTimeoutReset;
    TimeStorage t = menuTime.getTime();

    //Serial.print(t.hours);
    //Serial.print(":");
    //Serial.print(t.minutes);
    //Serial.print(":");
    //Serial.print(t.seconds);    
}

void CALLBACK_FUNCTION onBacklighChanged(int id) {
   if(backlightOn){
    lcd.noBacklight();
   }
   else{
     lcd.backlight();
     backlightTimer = 0;
   }

   backlightOn = !backlightOn;

   renderer.invalidateAll();
}

void CALLBACK_FUNCTION onGeneratorChanged(int id) {
    backlightTimer = backlightTimeoutReset;
}

void CALLBACK_FUNCTION onStatusChanged(int id) {
    backlightTimer = backlightTimeoutReset;
}

void CALLBACK_FUNCTION onSelftestChanged(int id) {
    backlightTimer = backlightTimeoutReset;
}

void CALLBACK_FUNCTION onSetDatetimeChanged(int id) {
    backlightTimer = backlightTimeoutReset;
}

void CALLBACK_FUNCTION storageEmptyChanged(int id) {
    resetSystem();  // TESTING
    
    bool et = false;
    et = menuStorageEmpty.getBoolean();  // TESTING
    if(et)
      storageTankEmpty = true;
    else
      storageTankEmpty = false;
    
    
    Serial.print("Empty float is ");
    Serial.println(storageTankEmpty);

}

void CALLBACK_FUNCTION ondisableIOChanged(int id) {
    disableIOPins = !disableIOPins;  // toggle IO on or off
    Serial.print("IO is ");
    Serial.println(disableIOPins);
}

void CALLBACK_FUNCTION onGeneratorRunningChanged(int id) {
    bool running = false;
    running = menuRunning.getBoolean();  // TESTING
    if(running)
      generatorRunning = true;
    else
      generatorRunning = false;
    
    
    Serial.print("Generator Running is ");
    Serial.println(generatorRunning);

}

void CALLBACK_FUNCTION onStorageFullChanged(int id) {
    bool full = false;
    full = menuStorageEmpty.getBoolean();  // TESTING
    if(full)
      storageTankFull = true;
    else
      storageTankFull = false;
    
    
    Serial.print("Full float is ");
    Serial.println(storageTankFull);
}
