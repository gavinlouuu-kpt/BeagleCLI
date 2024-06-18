// Hardware.cpp
#include <Arduino.h>
#include <Hardware.h>
#include <pinConfig.h>
#include <Init.h>
#include <map>
#include <functional>
#include <beagleCLI.h>
#include <M5Stack.h>
#include <exp_setup.h>

// std::map<String, std::function<void()>> commandMap;
// using CommandHandler = std::function<void(int)>;
// std::map<String, CommandHandler> commandHandlers;

void hardwareCheckTask(void *pvParameters)
{
    // check sd and sensor before starting other functions
    // if sd and sensors are not working, then the system is not ready
    // suspend all other functions

    // if (!SD.begin())
    // {
    //     Serial.println("SD card failed, or not present");
    //     vTaskDelay(5000);
    //     ESP.restart();
    //     // pause all device functions
    // }
    // if (sensorCheck() = 0)
    // {
    //     Serial.println("Sensor failed, or not present");
    //     ESP.restart();
    //     // pause all device functions
    // }
    // vTaskDelay(1000);
}

void pinSetup()
{
    pinMode(PWM_Heater, OUTPUT);
    pinMode(PWM_Vin, OUTPUT);
}

void pwmSetup()
{
    ledcSetup(PWM_H_CH, HFREQ, pwmRES);
    ledcAttachPin(PWM_Heater, PWM_H_CH);
    delay(100);
    ledcSetup(PWM_V_CH, VFREQ, pwmRES);
    ledcAttachPin(PWM_Vin, PWM_V_CH);
}

void solenoidOFF()
{
    // ledcWrite(SolenoidPWM, 0);
}

void setPumpSpeed(int speed)
{
    // ledcWrite(PumpPWM, 0);
    // configIntMod("/pump_speed", speed);
    // pumpSpeed = readConfigValue("/config.json","/pump_speed").toInt();
}

void hardwareCMD()
{

    commandMap["solenoidOFF"] = []()
    { solenoidOFF(); };
    commandHandlers["setPumpSpeed"] = [](int speed)
    { setPumpSpeed(speed); };
}