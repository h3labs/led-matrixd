#include "demo-canvas.h"

//#define DEBUG

#ifndef __LED_MATRIX_D_MAIN_H__
#define __LED_MATRIX_D_MAIN_H__
namespace ledMatrixD {
  extern int pwm_bits;
	extern bool large_display;
	extern int demo;
	extern int rows;
	extern int chain;
	extern int scroll_ms;
	extern bool do_luminance_correct;
	extern rgb_matrix::GPIO io;
}
#endif

