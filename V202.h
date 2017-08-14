#include "stc15.h"
void V202_setTXId(uint8_t txid_[3]);
void V202_begin();
void V202_command(uint8_t throttle, int yaw, int pitch, int roll, uint8_t flags);

void delay( uint8_t n);
void delayMicroseconds( uint8_t us);
