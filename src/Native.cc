#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "led-matrix.h"
#include "canvas.h"
#include "demo-canvas.h"
#include <string>

rgb_matrix::Canvas* canvas;
rgb_matrix::GPIO io;


namespace LedMatrixD {

	struct Pixel {
		Pixel() : red(0), green(0), blue(0){}
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct Image {
		Image() : width(-1), height(-1), image(NULL) {}
		~Image() { Delete(); }
		void Delete() { delete [] image; Reset(); }
		void Reset() { image = NULL; width = -1; height = -1; }
		inline bool IsValid() { return image && height > 0 && width > 0; }
		const Pixel &getPixel(int x, int y) {
			static Pixel black;
			if (x < 0 || x >= width || y < 0 || y >= height) return black;
			return image[x + width * y];
		}

		int width;
		int height;
		Pixel *image;
	};

	// Read line, skip comments.
	char *ReadLine(FILE *f, char *buffer, size_t len) {
		char *result;
		do {
			result = fgets(buffer, len, f);
		} while (result != NULL && result[0] == '#');
		return result;
	}


	bool LoadPPM(char* filename, void** new_image_ptr) {
		FILE *f = fopen(filename, "r");
		if (f == NULL) return false;
		char header_buf[256];
		const char *line = ReadLine(f, header_buf, sizeof(header_buf));
	#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
	fclose(f); return false; }
		if (sscanf(line, "P6 ") == EOF)
			EXIT_WITH_MSG("Can only handle P6 as PPM type.");
		line = ReadLine(f, header_buf, sizeof(header_buf));
		int new_width, new_height;
		if (!line || sscanf(line, "%d %d ", &new_width, &new_height) != 2)
			EXIT_WITH_MSG("Width/height expected");
		int value;
		line = ReadLine(f, header_buf, sizeof(header_buf));
		if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
			EXIT_WITH_MSG("Only 255 for maxval allowed.");
		const size_t pixel_count = new_width * new_height;
		Pixel *new_image = new Pixel [ pixel_count ];
		assert(sizeof(Pixel) == 3);   // we make that assumption.
		if (fread(new_image, sizeof(Pixel), pixel_count, f) != pixel_count) {
			line = "";
			EXIT_WITH_MSG("Not enough pixels read.");
		}
	#undef EXIT_WITH_MSG
		fclose(f);
		fprintf(stderr, "Read image '%s' with %dx%d\n", filename,
				new_width, new_height);
		Image* image = new Image();
		image->image = new_image;
		image->width = new_width;
		image->height = new_height;
		*new_image_ptr = (void*)image;
		//        std::cout << "Reading image \"" << filename << "\" with " << pixel_count << " pixels" << std::endl;
		//		const Pixel &p = current_image_.getPixel(15, 15);
		//        std::cout << "Color @ 15,15: " <<  (int)p.red << "," << (int)p.green << "," << (int)p.blue << std::endl;
		return true;
	}

	void wait(unsigned int msecs_){
		unsigned int msecs = (msecs_ * 1000) % 1000000;
		unsigned int secs = (msecs_ * 1000) / 1000000;
#ifdef DEBUG
		//std::cout << "Sleeping: " << secs << "s " << msecs << "us" << std::endl;
#endif
		sleep(secs);
		usleep((useconds_t)msecs);
	}
	/*
	* - Remove main from demo-main.cc
	* - Put load PPM in a C function
	* - Put the different required functions in Ruby in C functions
	*/

	extern "C" {	
		int pwm_bits = -1;
		bool large_display;
		int rows = 32;
		int chain = 1;
		bool do_luminance_correct = true;
		void init(){
			if (!io.Init()){
				fprintf(stderr, "IO could not initialized\n");
				exit(EXIT_FAILURE);
			}
			rgb_matrix::RGBMatrix *matrix = new rgb_matrix::RGBMatrix(&io, rows, chain);
			matrix->set_luminance_correct(do_luminance_correct);
			if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
				fprintf(stderr, "Invalid range of pwm-bits\n");
				exit(EXIT_FAILURE);
			}
			canvas = matrix;
		}
		void clear(){
		}
		void load_image(char* filename, void** image, int* width, int* height){
			LoadPPM(filename, image);
		}
		void draw_image(void** image, int x_offset, int y_offset){
			Image* current_image = (Image*)*image;
			const int screen_height = canvas->height();
			const int screen_width = canvas->width();
			for (int x = 0; x < screen_width; ++x) {
				for (int y = 0; y < screen_height; ++y) {
					const Pixel &p = current_image->getPixel(
							(x_offset + x) % current_image->width, y);
					canvas->SetPixel(x, y, p.red, p.green, p.blue);
				}
			}
		}
		void scroll_image(void** image, int msecs){
			int scrollWidth = 1;
			Image* current_image = (Image*)*image;
			if((current_image->width - canvas->width()) > 0){
				scrollWidth = current_image->width - canvas->width();
			}
			for(int j = 0; j < scrollWidth; j++){
				draw_image(image, j, 0);
				wait((unsigned int) msecs);
			}
		}
	}
}
