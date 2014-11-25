#ifndef __LED_MATRIX_D_MAIN_H__
#define __LED_MATRIX_D_MAIN_H__
namespace ledMatrixD {
  int pwm_bits = -1;
	bool large_display = false;
	int demo = -1;
	int rows = 32;
	int chain = 1;
	int scroll_ms = 30;
	bool do_luminance_correct = true;
	rgb_matrix::GPIO io;
}
#endif

