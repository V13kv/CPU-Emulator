#ifndef PROCESSOR_SETTINGS_H
#define PROCESSOR_SETTINGS_H


// -----------------------------------PROCESSOR CONSTANTS-----------------------------------
#define MRI_IS_IMMEDIATE(byteCodeByte)  (byteCodeByte & 0b001) != 0
#define MRI_IS_REGISTER(byteCodeByte)   (byteCodeByte & 0b010) != 0
#define MRI_IS_MEMORY(byteCodeByte)     (byteCodeByte & 0b100) != 0
#define GET_TOTAL_ARGS(byteCodeByte)    (byteCodeByte & 0b11100000) >> 5
#define GET_GLOBAL_MRI(byteCodeByte)    (byteCodeByte & 0b00011100) >> 2

const int MAX_RAM_SIZE                  = 500;      
const int DEFAULT_DOUBLE_VALUE          = 0;
const double BAD_DOUBLE_VALUE           = -663;
const int RAM_CELLS_TO_DUMP             = 15;
const double EPS                        = 0.001;
const double DOUBLES_ARE_EQUAL          = 0;
const double FIRST_DOUBLE_IS_GREATER    = 1;
const double FIRST_DOUBLE_IS_LOWER      = -1;
// -----------------------------------------------------------------------------------------

// --------------------------------------GPU CONSTANTS--------------------------------------

const int MAX_VRAM_SIZE                 = 250;

/*
    #define WINDOW_NAME                 "CSFML WINDOW"
    const sfContextSettings settings    = { .antialiasingLevel = 8 };
    const int WIDTH                     = 800;
    const int HEIGHT                    = 600;
    const int BITS_PER_PIXEL            = 32;
    const int MAX_FPS                   = 60;
    const int CIRCLE_RADIUS             = 200;
*/

// -----------------------------------------------------------------------------------------


#endif  // PROCESSOR_SETTINGS_H
