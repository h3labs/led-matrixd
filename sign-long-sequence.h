#ifndef __LED_MATRIX_D_SIGN_LONG_SEQUENCE_H__
#define __LED_MATRIX_D_SIGN_LONG_SEQUENCE_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>

namespace ledMatrixD
{
  class Display {
    public:
      Display();
      virtual void show() = 0;
      void undefinedMsg(std::string section , std::string attr);
      std::string getFilePath(std::string filename);
      bool LoadPPM(const char *filename) {
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
        current_image_.Delete();  // in case we reload faster than is picked up
        current_image_.image = new_image;
        current_image_.width = new_width;
        current_image_.height = new_height;
        return true;
      }

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

      Image current_image_;
      std::string mainPath;
  };
  class TitleDisplay : public Display {
    public:
      TitleDisplay();
      void show();
    private:
      int titleScroll = 0;
  };
  class LogoDisplay : public Display {
    public:
      LogoDisplay();
      void show();
  };
  class SpinLogoDisplay : public Display {
    public:
      SpinLogoDisplay();
      void show();
  };
  class ShopStatusDisplay : public Display {
    public:
      ShopStatusDisplay();
      void show();
  };
  class ConwaysDisplay : public Display {
    public:
      ConwaysDisplay();
      void show();
  };
  class URLDisplay : public Display {
    public:
      URLDisplay();
      void show();
  };
  class RandomSpriteDisplay : public Display {
    public:
      RandomSpriteDisplay();
      void show();
  };
  class TwitterDisplay : public Display {
    public:
      TwitterDisplay();
      void show();
  };
  void initLongSequence();
  void runLongSequence();
}
#endif
