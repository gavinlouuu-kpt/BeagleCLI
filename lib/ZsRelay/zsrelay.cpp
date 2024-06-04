#include <Arduino.h>
#include <beagleCLI.h>
#include <zsrelay.h>
#include <CRC16.h>
#include <vector>

CRC16 crc(CRC16_MODBUS_POLYNOME,
          CRC16_MODBUS_INITIAL,
          CRC16_MODBUS_XOR_OUT,
          CRC16_MODBUS_REV_IN,
          CRC16_MODBUS_REV_OUT);

// Define the command as a byte array
std::vector<uint8_t> all_on = {0x01, 0x06, 0x00, 0x34, 0x00, 0x01, 0x09, 0xC4};
// '01 06 00 34 00 00 C8 04'
std::vector<uint8_t> all_off = {0x01, 0x06, 0x00, 0x34, 0x00, 0x00, 0xC8, 0x04};

void relay_on()
{
    Serial2.write(all_on.data(), all_on.size());
    for (uint8_t i : all_on)
    {
        Serial.print(i);
    }
}

void relay_off()
{
    Serial2.write(all_off.data(), all_off.size());
    for (uint8_t i : all_off)
    {
        Serial.print(i);
    }
}

void cmd_modbus_crc(std::vector<uint8_t> &arr)
{
    // Calculate the CRC for the first 6 bytes of the array
    for (int i = 0; i < 6; i++)
    { // Assume the first 6 bytes are the data
        crc.add(arr[i]);
    }

    uint16_t crcValue = crc.calc(); // Store the calculated CRC

    // Ensure the vector can hold the CRC values
    if (arr.size() < 8)
    {
        arr.resize(8); // Resize to make space for the CRC if not already enough space
    }

    // Append CRC in little-endian order directly into the vector at positions 6 and 7
    arr[6] = crcValue & 0xFF;        // Lower byte of CRC
    arr[7] = (crcValue >> 8) & 0xFF; // Upper byte of CRC

    // If Serial2.write is needed, convert vector back to raw pointer for writing
    // Serial2.write(arr.data(), arr.size()); // Uncomment if you need to send data here
}

void test_calc()
{
    std::vector<uint8_t> arr = {0x01, 0x06, 0x00, 0x34, 0x00, 0x01, 0x00, 0x00};
    cmd_modbus_crc(arr);
    Serial2.write(arr.data(), arr.size());
}

std::vector<uint8_t> switchCommand(int devicePosition, int relayAddress, int relayState)
{
    std::vector<uint8_t> command;
    crc.restart();                                                // Reset the CRC calculation
    command.push_back(static_cast<uint8_t>(devicePosition));      // Device position (1 byte)
    command.push_back(0x06);                                      // Function code byte
    command.push_back(static_cast<uint8_t>(relayAddress >> 8));   // Relay address high byte
    command.push_back(static_cast<uint8_t>(relayAddress & 0xFF)); // Relay address low byte
    command.push_back(static_cast<uint8_t>(relayState >> 8));     // Relay state high byte
    command.push_back(static_cast<uint8_t>(relayState & 0xFF));   // Relay state low byte

    // Calculate CRC
    // CRC16 crc;
    for (uint8_t byte : command)
    {
        crc.add(byte);
    }
    uint16_t crcValue = crc.calc();

    // Append CRC in little-endian order
    command.push_back(static_cast<uint8_t>(crcValue & 0xFF)); // CRC low byte
    command.push_back(static_cast<uint8_t>(crcValue >> 8));   // CRC high byte

    return command;
}

void cmd_off()
{
    Serial2.flush();
    delay(100);
    std::vector<uint8_t> command = switchCommand(1, 47, 0);
    for (uint8_t i : command)
    {
        Serial.print(i);
    }
    Serial2.write(command.data(), command.size());
}

void cmd_on()
{
    Serial2.flush();
    delay(100);
    std::vector<uint8_t> command = switchCommand(1, 47, 1);
    for (uint8_t i : command)
    {
        Serial.print(i);
    }
    Serial2.write(command.data(), command.size());
}

void c7_on()
{
    delay(10);
    std::vector<uint8_t> command = switchCommand(1, 7, 1);
    Serial2.write(command.data(), command.size());
}

void man_test(int x)
{
    Serial2.flush();
    delay(10);
    std::vector<uint8_t> command = switchCommand(1, x, 1);
    Serial2.write(command.data(), command.size());
    Serial2.flush();
    delay(10);
    std::vector<uint8_t> command2 = switchCommand(1, 7, 0);
    Serial2.write(command2.data(), command2.size());
    Serial2.flush();
    delay(500);
    std::vector<uint8_t> command3 = switchCommand(1, x, 0);
    Serial2.write(command3.data(), command3.size());
    Serial2.flush();
    delay(10);
    std::vector<uint8_t> command4 = switchCommand(1, 7, 1);
    Serial2.write(command4.data(), command4.size());
}

void c0_on_off()
{
    man_test(0);
}

void c1_on_off()
{
    man_test(1);
}

void c2_on_off()
{
    man_test(2);
}

void zsrelayCMD()
{
    commandMap["onRelay"] = []()
    { relay_on(); };
    commandMap["offRelay"] = []()
    { relay_off(); };
    commandMap["testCalc"] = []()
    { test_calc(); };
    commandMap["cmdON"] = []()
    { cmd_on(); };
    commandMap["cmdOFF"] = []()
    { cmd_off(); };
    commandMap["c0"] = []()
    { c0_on_off(); };
    commandMap["c1"] = []()
    { c1_on_off(); };
    commandMap["c2"] = []()
    { c2_on_off(); };
    commandMap["c7"] = []()
    { c7_on(); };
}