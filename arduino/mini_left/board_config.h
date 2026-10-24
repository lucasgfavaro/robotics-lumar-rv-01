#pragma once
// IZQUIERDA - configuración activa del lateral.
#define ROVER_I2C_ADDRESS 0x10

#include "traction.h"
// One entry per wheel, negative/positive electrical direction.
// fullScalePps=0 disables feedback until encoder rate is measured.
const traction::Calibration WHEEL_CAL[2]={{{48,48},{0,0}},{{48,48},{0,0}}};
