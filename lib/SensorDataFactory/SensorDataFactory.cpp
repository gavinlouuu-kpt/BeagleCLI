// SensorDataFactory.cpp
#include <Arduino.h>
#include "SensorDataFactory.h"
#include <chrono>
#include <vector>
#include <thread>
#include <pinConfig.h>
#include <beagleCLI.h>
#include <Init.h>
#include <vector>
#include <chrono>
#include <algorithm> // For std::copy

void datastreamTask(void * parameter){
    SensorDataFactory sensorDataFactory;
    while(true){
        sensorDataFactory.dataStream();
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

TaskHandle_t datastreamTaskHandle = NULL;

void datastreamTaskCreate() {
    xTaskCreatePinnedToCore(
        datastreamTask,     // Task function
        "datastreamTask",   // Name of the task
        10000,              // Stack depth in words
        NULL,               // Task input parameters
        1,                  // Priority
        &datastreamTaskHandle, // Task handle
        0                   // Core where the task should run
    );
}


void killDatastreamTaskDirectly() {
    if (datastreamTaskHandle != NULL) {
        vTaskDelete(datastreamTaskHandle);
        ledcWrite(PumpPWM, 0); // turn off pump
        datastreamTaskHandle = NULL; // Invalidate the handle to prevent reuse
        Serial.println("datastreamTask killed successfully.");
    } else {
        Serial.println("datastreamTask was not running.");
    }
}



bool SensorDataFactory::bme_begin() {
    if (!bme.begin()) {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        return false;
    }
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_1);
    bme.setGasHeater(450, 5); // 320*C for 150 ms
    return true;
}


int SensorDataFactory::breath_check(){
    /*return a vector that is compatible with dataVec
    pass on the vector for further data collection*/
    preSampling();
    auto start_time = std::chrono::steady_clock::now(); // Record start time for timeout check
    auto previoustime = std::chrono::steady_clock::now();
    std::vector<int> Sensor_arr; // Vector to store sensor readings dynamically.

    while (true) {
        int sensorValue = 0;
        if (bme.performReading()){
            sensorValue = bme.gas_resistance;
            Serial.print(">Breath:");
            Serial.println(sensorValue);
        }
        
        // Add new sensor value, ensure vector does not exceed 100 elements.
        if (Sensor_arr.size() < 10) {
            Sensor_arr.push_back(sensorValue);
        } else {
            // Shift vector contents, discarding the oldest reading.
            std::copy(Sensor_arr.begin() + 1, Sensor_arr.end(), Sensor_arr.begin());
            Sensor_arr.back() = sensorValue;
        }

        // Every 500 milliseconds, check if the change in sensor values indicates a breath.
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - previoustime).count() > 500) {
            if (Sensor_arr.size() >= 5 && Sensor_arr.front() - Sensor_arr.back() > 500) {
                Serial.print(">Baseline:");
                Serial.println(Sensor_arr.front());
                // print out full array of Sensor_arr via serial monitor
                Serial.println("Sensor_arr: ");
                for (int i = 0; i < Sensor_arr.size(); i++) {
                    Serial.print(Sensor_arr[i]);
                    Serial.print(", ");
                }
                return Sensor_arr.front(); // Return the baseline value indicating a breath detected.
            }
            previoustime = now; // Reset the timer for the next interval check.
        }

        // Check for timeout (60 seconds) to return an error if no breath is detected.
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count() > 60) {
            Serial.println("Error: No breath detected within 60 seconds.");
            return -1; // Return an error code.
        }
    }
}

volatile bool readyToSample = false;

void SensorDataFactory::preSampling(){
    warmingInProgress = true;
    Serial.println(pumpSpeed);
    ledcWrite(PumpPWM, pumpSpeed); // turn on pump
    // if (!bme_begin()) {
    //     Serial.println("Failed to initialize BME680 sensor.");
    //     return;
    // }
    // create a timer for 30 seconds to let the sensor warmup
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::seconds(30);
    Serial.println("Warming up sensor: ");
    while(std::chrono::steady_clock::now() < end){
        bme.setGasHeater(200,5);
        if(bme.performReading()){
            
            Serial.print(">200_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(300,5);
        if(bme.performReading()){
            Serial.print(">300_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(400,5);
        if(bme.performReading()){
            Serial.print(">400_Warming:");
            Serial.println(bme.gas_resistance);
        }
    }
    warmingInProgress = false;
    readyToSample = true;
}

void SensorDataFactory::postSampling(){
    ledcWrite(PumpPWM, 0); // turn off pump

}


void SensorDataFactory::dataStream(){
    ledcWrite(PumpPWM, pumpSpeed); // turn on pump
    bme_begin();
    while (1)
    {
        bme.setGasHeater(200,5);
        if (bme.performReading()) {
        // Serial.println(bme.temperature);//Serial.print(",");
        // Serial.print(">Humidity:");Serial.println(bme.humidity);//Serial.print(",");
            Serial.print(">200 Res:");Serial.println(bme.gas_resistance);
        }
     
        bme.setGasHeater(300,5);
        if (bme.performReading()) {
        // Serial.println(bme.temperature);//Serial.print(",");
        // Serial.print(">Humidity:");Serial.println(bme.humidity);//Serial.print(",");
            Serial.print(">300 Res:");Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(400,5);
        if (bme.performReading()) {
        // Serial.println(bme.temperature);//Serial.print(",");
        // Serial.print(">Humidity:");Serial.println(bme.humidity);//Serial.print(",");
            Serial.print(">400 Res:");Serial.println(bme.gas_resistance);
        }
    }
    
        
}

void SensorDataFactory::performSampling(std::vector<float>& conVec, std::vector<uint32_t>& dataVec200, std::vector<uint32_t>& dataVec300, std::vector<uint32_t>& dataVec400) {
    using namespace std::chrono;
    // preSampling();
    samplingInProgress = true;

// record initial condition    
    if(bme.performReading()){
        conVec.push_back(bme.temperature);
        conVec.push_back(bme.humidity);
        conVec.push_back(bme.pressure);
    }
    

//create a loop for a minute where dummyData is called every 60ms 
    auto start = steady_clock::now();
    auto end = start + seconds(30);
    dataVec200.clear(); // Make sure it's empty before filling
    dataVec300.clear(); 
    dataVec400.clear(); 
    Serial.println("Sampling: ");
    while(steady_clock::now() < end){
        bme.setGasHeater(200,5);
        if(bme.performReading()){
            dataVec200.push_back(bme.gas_resistance);
            Serial.print(">200_Sampling:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(300,5);
        if(bme.performReading()){
            dataVec300.push_back(bme.gas_resistance);
            Serial.print(">300_Sampling:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(400,5);
        if(bme.performReading()){
            dataVec400.push_back(bme.gas_resistance);
            Serial.print(">400_Sampling:");
            Serial.println(bme.gas_resistance);
        }
    }
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = now - start;

// record ending condition // no blocking
    if(bme.performReading()){
        conVec.push_back(bme.temperature);
        conVec.push_back(bme.humidity);
        conVec.push_back(bme.pressure);
        conVec.push_back(duration.count() * 1000); // Convert duration to milliseconds
    }
    postSampling();
    samplingInProgress = false;

}


void SensorDataFactory::waitUser(){

    while(!samplingInProgress){
        // Sleep for a short duration to prevent busy waiting
                bme.setGasHeater(200,5);
        if(bme.performReading()){
            
            Serial.print(">200_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(300,5);
        if(bme.performReading()){
            Serial.print(">300_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(400,5);
        if(bme.performReading()){
            Serial.print(">400_Warming:");
            Serial.println(bme.gas_resistance);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

SensorData SensorDataFactory::createSensorData() {
    std::string infoString = "T_s, RH_s, Pa_s, T_e, RH_e,Pa_e, t_ms";
    std::vector<float> conVec;
    std::vector<uint32_t> dataVec200, dataVec300, dataVec400;
    bme_begin();
    preSampling();
    waitUser();
    performSampling(conVec, dataVec200, dataVec300, dataVec400);
    return SensorData(infoString, conVec, dataVec200, dataVec300, dataVec400);
}

void sensorCMD(){
    commandMap["datastream"] = []() { datastreamTaskCreate(); };
    commandMap["killdatastream"] = []() { killDatastreamTaskDirectly(); };
    
}