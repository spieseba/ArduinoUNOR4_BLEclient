#ifndef BLE_H
#define BLE_H

void startBLEStack();
void wrapperBLE(unsigned long (*wrappedFunc)(uint8_t));

#endif
