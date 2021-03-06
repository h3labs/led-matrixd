#ifndef __LED_MATRIX_D_DISPLAYS_SEQUENCE_H__
#define __LED_MATRIX_D_DISPLAYS_SEQUENCE_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <map>
#include "graphics.h"
#include "LedMatrixDMain.h"
#include "Beacon.h"

#define EXIT_MSG(m) {\
	std::cerr << "error: " << __FILE__ << ":" << __LINE__ << " " << m << std::endl;\
	exit(EXIT_FAILURE);\
}

namespace ledMatrixD
{
	extern Canvas* canvas;
  extern Beacon* beacon;
	class Display {
		public:
			Display();
			virtual void show() = 0;
			void undefinedMsg(std::string section , std::string attr);
			std::string getFilePath(std::string filename);
			void wait(unsigned int msecs_){
				unsigned int msecs = (msecs_ * 1000) % 1000000;
				unsigned int secs = (msecs_ * 1000) / 1000000;
#ifdef DEBUG
				//std::cout << "Sleeping: " << secs << "s " << msecs << "us" << std::endl;
#endif
				sleep(secs);
				usleep((useconds_t)msecs);
			}
			void loadImage(std::string filename){
				std::string fullFilename = this->getFilePath(filename);
				if(!this->LoadPPM(fullFilename)){
					std::cerr << fullFilename << std::endl;
					EXIT_MSG("could not find file");
				}
			}
			void scroll(int msecs){
				int scrollWidth = 1;
				if((current_image_.width - canvas->width()) > 0){
					scrollWidth = current_image_.width - canvas->width();
				}
				for(int j = 0; j < scrollWidth; j++){
					this->draw(j);
					this->wait((unsigned int) msecs);
				}
			}
			void draw(int offset = 0){
				const int screen_height = canvas->height();
				const int screen_width = canvas->width();
				for (int x = 0; x < screen_width; ++x) {
					for (int y = 0; y < screen_height; ++y) {
						const Pixel &p = current_image_.getPixel(
								(offset + x) % current_image_.width, y);
						canvas->SetPixel(x, y, p.red, p.green, p.blue);
					}
				}
			}
			bool LoadPPM(std::string filename) {
				FILE *f = fopen(filename.c_str(), "r");
				if (f == NULL) return false;
				char header_buf[256];
				const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename.c_str(), m, line); \
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
#ifdef DEBUG
				fprintf(stderr, "Read image '%s' with %dx%d\n", filename.c_str(),
						new_width, new_height);
#endif
				current_image_.Delete();  // in case we reload faster than is picked up
				current_image_.image = new_image;
				current_image_.width = new_width;
				current_image_.height = new_height;
				//        std::cout << "Reading image \"" << filename << "\" with " << pixel_count << " pixels" << std::endl;
				//		const Pixel &p = current_image_.getPixel(15, 15);
				//        std::cout << "Color @ 15,15: " <<  (int)p.red << "," << (int)p.green << "," << (int)p.blue << std::endl;
				return true;
			}

						Image current_image_;
			std::string mainPath;
	};
	class TitleDisplay : public Display {
		public:
			TitleDisplay();
			void show();
		private:
			int titleScroll;
	};
	class TaglineDisplay : public Display {
		public:
			TaglineDisplay();
			void show();
		private:
			int taglineScroll;
	};
	class LogoDisplay : public Display {
		public:
			LogoDisplay(int type);
			void show();
		private:
			std::string atomPath;
			int duration;
	};
	class SpinLogoDisplay : public Display {
		public:
			SpinLogoDisplay();
			void show();
		private:
			std::string atomPath;
			int frameDuration;
			int atomSpin;
	};
	class DateDisplay : public Display {
		public:
			DateDisplay();
			void show();
			~DateDisplay();
		private:
			std::string format;
			std::string special_date_format;
			std::string fontFilename;
			std::string fontDir;
			rgb_matrix::Font font;
			int dateDur;
			int x;
			int y;
			int r;
			int g;
			int b;
	};
	class ShopStatusDisplay : public Display {
		public:
			ShopStatusDisplay();
			void show();
		private:
			bool beaconExists();
			int scrolls;
			int scrollMS;
			std::string beaconFilename;
	};
	class ConwaysDisplay : public Display {
		public:
			ConwaysDisplay();
			void show();
		private:
			int duration;
	};
	class URLDisplay : public Display {
		public:
			URLDisplay();
			void show();
		private:
			int scrolls;
			int scrollMS;
	};
	class RandomSpriteDisplay : public Display {
		typedef std::map<int, std::string> StringMap;
		public:
		RandomSpriteDisplay();
		void show();
		~RandomSpriteDisplay();
		private:
		std::string getFullSpritePath();
		std::string spritePath;
		int spriteDuration;
		int times;
		StringMap fileMap;
	};
	class TwitterDisplay : public Display {
		public:
			TwitterDisplay();
			void show();
		private:
			int scrolls;
			int scrollMS;
	};
	class DateBannerDisplay : public Display {
		public:
			DateBannerDisplay();
			void show();
		private:
			std::string getFullDateBannerPath();
			std::string dateBannerPath;
			std::string format;
			std::string special_date_format;
			int scrolls;
			int scrollMS;
	};

	class MessageDisplay : public Display {
		public:
			MessageDisplay();
			void show();
		private:
			std::string fontFilename;
			std::string fontDir;
			rgb_matrix::Font font;
			std::string message;
			std::vector<int> widths;
      int sumOfWidths;
			int scrolls;
			int scrollMS;
	};

	void initLongSequence();
	void runLongSequence();
}
#endif
