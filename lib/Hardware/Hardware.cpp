//Hardware.cpp
#include <Arduino.h>
#include <Hardware.h>
#include <pinConfig.h>
#include <Init.h>
#include <map>
#include <functional>
#include <beagleCLI.h>

// std::map<String, std::function<void()>> commandMap;
// using CommandHandler = std::function<void(int)>;
// std::map<String, CommandHandler> commandHandlers;

void pinSetup() {
    pinMode(BAT, INPUT); //read battery voltage
    pinMode(VBAT, OUTPUT); //enable to read battery voltage
    pinMode(PA_1, OUTPUT); //pump at AIN1 (PWM) 3V to H1
    pinMode(HB_1, OUTPUT); //heater at BIN1 (PWM) 5V to H4
    pinMode(V1_8, OUTPUT); //1.8V enable for gas sensor heater
    pinMode(SOL, OUTPUT); //solenoid valve
}

void pwmSetup() {
    ledcSetup(PumpPWM, PumpFREQ, pwmRES);
    ledcAttachPin(PA_1, PumpPWM);
    ledcSetup(HeaterPWM, HeaterFREQ, pwmRES);
    ledcAttachPin(HB_1, HeaterPWM);
    ledcSetup(SolenoidPWM, SolenoidFREQ, pwmRES);
    ledcAttachPin(SOL, SolenoidPWM);
}

void pumpON(){
    ledcWrite(PumpPWM, pumpSpeed);
}

void pumpOFF(){
    ledcWrite(PumpPWM, 0);
}

void solenoidON(){
    ledcWrite(SolenoidPWM, 255);
}

void solenoidOFF(){
    ledcWrite(SolenoidPWM, 0);
}

void setPumpSpeed(int speed){
    ledcWrite(PumpPWM, 0);
    configIntMod("/pump_speed", speed);
    pumpSpeed = readConfigValue("/config.json","/pump_speed").toInt();
}

void hardwareCMD(){
    commandMap["pumpON"] = []() { pumpON(); };
    commandMap["pumpOFF"] = []() { pumpOFF(); };
    commandMap["solenoidON"] = []() { solenoidON(); };
    commandMap["solenoidOFF"] = []() { solenoidOFF(); };
    commandHandlers["setPumpSpeed"] = [](int speed) { setPumpSpeed(speed); };
}