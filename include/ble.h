#ifndef BLE_H
#define BLE_H

void startBLEStack();
void wrapperBLE(void (*wrappedFunc)(uint8_t));

#endif
