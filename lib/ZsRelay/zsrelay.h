#ifndef ZSRELAY_H
#define ZSRELAY_H


extern uint8_t all_on[];
extern uint8_t all_off[];

void relay_on();
void relay_off();
void zsrelayCMD();
void cmd_modbus_crc(std::vector<uint8_t>& arr);
void test_calc();
std::vector<uint8_t> switchCommand(int devicePosition, int relayAddress, int relayState);
void cmd_test();


#endif // ZSRELAY_H
