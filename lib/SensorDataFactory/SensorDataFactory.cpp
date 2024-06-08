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
#include <algorithm>

void datastreamTask(void *parameter)
{
    SensorDataFactory sensorDataFactory;
    while (true)
    {
        sensorDataFactory.dataStream();
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

TaskHandle_t datastreamTaskHandle = NULL;

void datastreamTaskCreate()
{
    xTaskCreatePinnedToCore(
        datastreamTask,        // Task function
        "datastreamTask",      // Name of the task
        10000,                 // Stack depth in words
        NULL,                  // Task input parameters
        1,                     // Priority
        &datastreamTaskHandle, // Task handle
        0                      // Core where the task should run
    );
}

void killDatastreamTaskDirectly()
{
    if (datastreamTaskHandle != NULL)
    {
        vTaskDelete(datastreamTaskHandle);
        // ledcWrite(PumpPWM, 0);       // turn off pump
        datastreamTaskHandle = NULL; // Invalidate the handle to prevent reuse
        Serial.println("datastreamTask killed successfully.");
    }
    else
    {
        Serial.println("datastreamTask was not running.");
    }
}

bool SensorDataFactory::bme_begin()
{
    if (!bme.begin())
    {
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

volatile bool readyToSample = false;

void SensorDataFactory::preSampling()
{
    // warmingInProgress = true;
    Serial.println(pumpSpeed);
    // ledcWrite(PumpPWM, pumpSpeed); // turn on pump
    // if (!bme_begin()) {
    //     Serial.println("Failed to initialize BME680 sensor.");
    //     return;
    // }
    // create a timer for 30 seconds to let the sensor warmup
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::seconds(30);
    Serial.println("Warming up sensor: ");
    while (std::chrono::steady_clock::now() < end)
    {
        bme.setGasHeater(200, 5);
        if (bme.performReading())
        {

            Serial.print(">200_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(300, 5);
        if (bme.performReading())
        {
            Serial.print(">300_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(400, 5);
        if (bme.performReading())
        {
            Serial.print(">400_Warming:");
            Serial.println(bme.gas_resistance);
        }
    }
    // warmingInProgress = false;
    // readyToSample = true;
}

void SensorDataFactory::dataStream()
{
    // ledcWrite(PumpPWM, pumpSpeed); // turn on pump
    bme_begin();
    unsigned long startTime, endTime;

    while (1)
    {
        startTime = millis();

        bme.setGasHeater(200, 10);
        if (bme.performReading())
        {
            Serial.print(">200 Res:");
            Serial.println(bme.gas_resistance);
        }

        endTime = millis();
        Serial.print("Time for 200: ");
        Serial.println(endTime - startTime);

        startTime = millis();

        bme.setGasHeater(300, 10);
        if (bme.performReading())
        {
            Serial.print(">300 Res:");
            Serial.println(bme.gas_resistance);
        }

        endTime = millis();
        Serial.print("Time for 300: ");
        Serial.println(endTime - startTime);

        startTime = millis();

        bme.setGasHeater(400, 10);
        if (bme.performReading())
        {
            Serial.print(">400 Res:");
            Serial.println(bme.gas_resistance);
        }

        endTime = millis();
        Serial.print("Time for 400: ");
        Serial.println(endTime - startTime);

        // delay(1000); // Add a delay to prevent the loop from running too fast
    }
}

void SensorDataFactory::performSampling(std::vector<float> &conVec, std::vector<uint32_t> &dataVec200, std::vector<uint32_t> &dataVec300, std::vector<uint32_t> &dataVec400)
{
    using namespace std::chrono;
    // preSampling();
    // samplingInProgress = true;

    // record initial condition
    if (bme.performReading())
    {
        conVec.push_back(bme.temperature);
        conVec.push_back(bme.humidity);
        conVec.push_back(bme.pressure);
    }

    // create a loop for a minute where dummyData is called every 60ms
    auto start = steady_clock::now();
    auto end = start + seconds(30);
    dataVec200.clear(); // Make sure it's empty before filling
    dataVec300.clear();
    dataVec400.clear();
    while (steady_clock::now() < end)
    {
        bme.setGasHeater(200, 5);
        if (bme.performReading())
        {
            dataVec200.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(300, 5);
        if (bme.performReading())
        {
            dataVec300.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(400, 5);
        if (bme.performReading())
        {
            dataVec400.push_back(bme.gas_resistance);
        }
    }
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = now - start;

    // record ending condition // no blocking
    if (bme.performReading())
    {
        conVec.push_back(bme.temperature);
        conVec.push_back(bme.humidity);
        conVec.push_back(bme.pressure);
        conVec.push_back(duration.count() * 1000); // Convert duration to milliseconds
    }
    // postSampling();
    // samplingInProgress = false;
}

void SensorDataFactory::Sampling(std::vector<float> &conVec, std::vector<uint32_t> &dataVec,
                                 std::vector<uint32_t> &dataVec150, std::vector<uint32_t> &dataVec200,
                                 std::vector<uint32_t> &dataVec250, std::vector<uint32_t> &dataVec300,
                                 std::vector<uint32_t> &dataVec350, std::vector<uint32_t> &dataVec400,
                                 std::vector<uint32_t> &dataVec450, std::vector<uint32_t> &dataVec500)
{
    using namespace std::chrono;

    if (bme.performReading())
    {
        conVec.push_back(bme.temperature);
        conVec.push_back(bme.humidity);
        conVec.push_back(bme.pressure);
    }

    // create a loop for a minute where dummyData is called every 60ms
    auto start = steady_clock::now();
    auto end = start + seconds(60);
    // dataVec.clear(); // Make sure it's empty before filling

    while (steady_clock::now() < end)
    {
        int heatingTime = 8;
        bme.setGasHeater(150, heatingTime);
        if (bme.performReading())
        {
            dataVec150.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(200, heatingTime);
        if (bme.performReading())
        {
            dataVec200.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(250, heatingTime);
        if (bme.performReading())
        {
            dataVec250.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(300, heatingTime);
        if (bme.performReading())
        {
            dataVec300.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(350, heatingTime);
        if (bme.performReading())
        {
            dataVec350.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(400, heatingTime);
        if (bme.performReading())
        {
            dataVec400.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(450, heatingTime);
        if (bme.performReading())
        {
            dataVec450.push_back(bme.gas_resistance);
        }
        bme.setGasHeater(500, heatingTime);
        if (bme.performReading())
        {
            dataVec500.push_back(bme.gas_resistance);
        }

        //

        // bme.setGasHeater(500, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec500.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(450, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec450.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(400, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec400.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(350, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec350.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(300, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec300.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(250, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec250.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(200, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec200.push_back(bme.gas_resistance);
        // }
        // bme.setGasHeater(150, heatingTime);
        // if (bme.performReading())
        // {
        //     dataVec150.push_back(bme.gas_resistance);
        // }
    }

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = now - start;

    // record ending condition // no blocking
    // if (bme.performReading())
    // {
    //     conVec.push_back(bme.temperature);
    //     conVec.push_back(bme.humidity);
    //     conVec.push_back(bme.pressure);
    //     conVec.push_back(duration.count() * 1000); // Convert duration to milliseconds
    // }
}

void SensorDataFactory::waitUser()
{

    // while (!samplingInProgress)
    while (true)
    {
        // Sleep for a short duration to prevent busy waiting
        bme.setGasHeater(200, 5);
        if (bme.performReading())
        {

            Serial.print(">200_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(300, 5);
        if (bme.performReading())
        {
            Serial.print(">300_Warming:");
            Serial.println(bme.gas_resistance);
        }
        bme.setGasHeater(400, 5);
        if (bme.performReading())
        {
            Serial.print(">400_Warming:");
            Serial.println(bme.gas_resistance);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

SensorData SensorDataFactory::createSensorData()
{
    std::string infoString = "T_s, RH_s, Pa_s, T_e, RH_e,Pa_e, t_ms";
    std::vector<float> conVec;
    // std::vector<uint32_t> dataVec200, dataVec300, dataVec400;
    std::vector<uint32_t> dataVec, dataVec150, dataVec200, dataVec250, dataVec300, dataVec350, dataVec400, dataVec450, dataVec500;
    bme_begin();
    // preSampling();
    // waitUser();
    // performSampling(conVec, dataVec200, dataVec300, dataVec400);
    Sampling(conVec, dataVec, dataVec150, dataVec200, dataVec250, dataVec300, dataVec350, dataVec400, dataVec450, dataVec500);
    return SensorData(infoString, conVec, dataVec150, dataVec200, dataVec250, dataVec300, dataVec350, dataVec400, dataVec450, dataVec500);
}

void sensorCMD()
{
    // commandMap["datastream"] = []()
    // { datastreamTaskCreate(); };
    // commandMap["killdatastream"] = []()
    // { killDatastreamTaskDirectly(); };
}