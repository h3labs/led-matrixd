// This is an example how to use the Canvas abstraction to map coordinates.
//
// This is a Canvas that delegates to some other Canvas (typically, the RGB
// matrix).
//
// Here, we want to address four 32x32 panels as one big 64x64 panel. Physically,
// we chain them together and do a 180 degree 'curve', somewhat like this:
// [>] [>]
//         v
// [<] [<]
#ifndef __LED_MATRIX_D_DEMO_CANVAS_H__
#define __LED_MATRIX_D_DEMO_CANVAS_H__
using std::min;
using std::max;
using namespace rgb_matrix;

class LargeSquare64x64Canvas : public Canvas {
public:
  // This class takes over ownership of the delegatee.
  LargeSquare64x64Canvas(Canvas *delegatee) : delegatee_(delegatee) {
    // Our assumptions of the underlying geometry:
    assert(delegatee->height() == 32);
    assert(delegatee->width() == 128);
  }
  virtual ~LargeSquare64x64Canvas() { delete delegatee_; }

  virtual void Clear() { delegatee_->Clear(); }
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) {
    delegatee_->Fill(red, green, blue);
  }
  virtual int width() const { return 64; }
  virtual int height() const { return 64; }
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue) {
    if (x < 0 || x >= width() || y < 0 || y >= height()) return;
    // We have up to column 64 one direction, then folding around. Lets map
    if (y > 31) {
      x = 127 - x;
      y = 63 - y;
    }
    delegatee_->SetPixel(x, y, red, green, blue);
  }

private:
  Canvas *delegatee_;
};

/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.
class ColorPulseGenerator : public ThreadedCanvasManipulator {
public:
  ColorPulseGenerator(Canvas *m) : ThreadedCanvasManipulator(m) {}
  void Run() {
    uint32_t continuum = 0;
    while (running()) {
      usleep(5 * 1000);
      continuum += 1;
      continuum %= 3 * 255;
      int r = 0, g = 0, b = 0;
      if (continuum <= 255) {
        int c = continuum;
        b = 255 - c;
        r = c;
      } else if (continuum > 255 && continuum <= 511) {
        int c = continuum - 256;
        r = 255 - c;
        g = c;
      } else {
        int c = continuum - 512;
        g = 255 - c;
        b = c;
      }
      canvas()->Fill(r, g, b);
    }
  }
};

class SimpleSquare : public ThreadedCanvasManipulator {
public:
  SimpleSquare(Canvas *m) : ThreadedCanvasManipulator(m) {}
  void Run() {
    const int width = canvas()->width();
    const int height = canvas()->height();
    // Diagonal
    for (int x = 0; x < width; ++x) {
      canvas()->SetPixel(x, x, 255, 255, 255);           // white
      canvas()->SetPixel(height -1 - x, x, 255, 0, 255); // magenta
    }
    for (int x = 0; x < width; ++x) {
      canvas()->SetPixel(x, 0, 255, 0, 0);              // top line: red
      canvas()->SetPixel(x, height - 1, 255, 255, 0);   // bottom line: yellow
    }
    for (int y = 0; y < height; ++y) {
      canvas()->SetPixel(0, y, 0, 0, 255);              // left line: blue
      canvas()->SetPixel(width - 1, y, 0, 255, 0);      // right line: green
    }
  }
};

class GrayScaleBlock : public ThreadedCanvasManipulator {
public:
  GrayScaleBlock(Canvas *m) : ThreadedCanvasManipulator(m) {}
  void Run() {
    const int sub_blocks = 16;
    const int width = canvas()->width();
    const int height = canvas()->height();
    const int x_step = max(1, width / sub_blocks);
    const int y_step = max(1, height / sub_blocks);
    uint8_t count = 0;
    while (running()) {
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          int c = sub_blocks * (y / y_step) + x / x_step;
          switch (count % 4) {
          case 0: canvas()->SetPixel(x, y, c, c, c); break;
          case 1: canvas()->SetPixel(x, y, c, 0, 0); break;
          case 2: canvas()->SetPixel(x, y, 0, c, 0); break;
          case 3: canvas()->SetPixel(x, y, 0, 0, c); break;
          }
        }
      }
      count++;
      sleep(2);
    }
  }
};

// Simple class that generates a rotating block on the screen.
class RotatingBlockGenerator : public ThreadedCanvasManipulator {
public:
  RotatingBlockGenerator(Canvas *m) : ThreadedCanvasManipulator(m) {}

  uint8_t scale_col(int val, int lo, int hi) {
    if (val < lo) return 0;
    if (val > hi) return 255;
    return 255 * (val - lo) / (hi - lo);
  }

  void Run() {
    const int cent_x = canvas()->width() / 2;
    const int cent_y = canvas()->height() / 2;

    // The square to rotate (inner square + black frame) needs to cover the
    // whole area, even if diagnoal. Thus, when rotating, the outer pixels from
    // the previous frame are cleared.
    const int rotate_square = min(canvas()->width(), canvas()->height()) * 1.41;
    const int min_rotate = cent_x - rotate_square / 2;
    const int max_rotate = cent_x + rotate_square / 2;

    // The square to display is within the visible area.
    const int display_square = min(canvas()->width(), canvas()->height()) * 0.7;
    const int min_display = cent_x - display_square / 2;
    const int max_display = cent_x + display_square / 2;

    const float deg_to_rad = 2 * 3.14159265 / 360;
    int rotation = 0;
    while (running()) {
      ++rotation;
      usleep(15 * 1000);
      rotation %= 360;
      for (int x = min_rotate; x < max_rotate; ++x) {
        for (int y = min_rotate; y < max_rotate; ++y) {
          float rot_x, rot_y;
          Rotate(x - cent_x, y - cent_x,
                 deg_to_rad * rotation, &rot_x, &rot_y);
          if (x >= min_display && x < max_display &&
              y >= min_display && y < max_display) { // within display square
            canvas()->SetPixel(rot_x + cent_x, rot_y + cent_y,
                               scale_col(x, min_display, max_display),
                               255 - scale_col(y, min_display, max_display),
                               scale_col(y, min_display, max_display));
          } else {
            // black frame.
            canvas()->SetPixel(rot_x + cent_x, rot_y + cent_y, 0, 0, 0);
          }
        }
      }
    }
  }

private:
  void Rotate(int x, int y, float angle,
              float *new_x, float *new_y) {
    *new_x = x * cosf(angle) - y * sinf(angle);
    *new_y = x * sinf(angle) + y * cosf(angle);
  }
};

class ImageScroller : public ThreadedCanvasManipulator {
public:
  // Scroll image with "scroll_jumps" pixels every "scroll_ms" milliseconds.
  // If "scroll_ms" is negative, don't do any scrolling.
  ImageScroller(Canvas *m, int scroll_jumps, int scroll_ms = 30)
    : ThreadedCanvasManipulator(m), scroll_jumps_(scroll_jumps),
      scroll_ms_(scroll_ms),
      horizontal_position_(0) {
  }

  virtual ~ImageScroller() {
    Stop();
    WaitStopped();   // only now it is safe to delete our instance variables.
  }

  // _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
  // Not really robust. Use at your own risk :)
  // This allows reload of an image while things are running, e.g. you can
  // life-update the content.
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
    horizontal_position_ = 0;
    MutexLock l(&mutex_new_image_);
    new_image_.Delete();  // in case we reload faster than is picked up
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    return true;
  }

  void Run() {
    const int screen_height = canvas()->height();
    const int screen_width = canvas()->width();
    while (running()) {
      {
        MutexLock l(&mutex_new_image_);
        if (new_image_.IsValid()) {
          current_image_.Delete();
          current_image_ = new_image_;
          new_image_.Reset();
        }
      }
      if (!current_image_.IsValid()) {
        usleep(100 * 1000);
        continue;
      }
      for (int x = 0; x < screen_width; ++x) {
        for (int y = 0; y < screen_height; ++y) {
          const Pixel &p = current_image_.getPixel(
                     (horizontal_position_ + x) % current_image_.width, y);
          canvas()->SetPixel(x, y, p.red, p.green, p.blue);
        }
      }
      horizontal_position_ += scroll_jumps_;
      if (horizontal_position_ < 0) horizontal_position_ = current_image_.width;
      if (scroll_ms_ <= 0) {
        // No scrolling. We don't need the image anymore.
        current_image_.Delete();
      } else {
        usleep(scroll_ms_ * 1000);
      }
    }
  }

private:
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

  const int scroll_jumps_;
  const int scroll_ms_;

  // Current image is only manipulated in our thread.
  Image current_image_;

  // New image can be loaded from another thread, then taken over in main thread.
  Mutex mutex_new_image_;
  Image new_image_;

  int32_t horizontal_position_;
};


// Abelian sandpile
// Contributed by: Vliedel
class Sandpile : public ThreadedCanvasManipulator {
public:
  Sandpile(Canvas *m, int delay_ms=50)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
    width_ = canvas()->width() - 1; // We need an odd width
    height_ = canvas()->height() - 1; // We need an odd height
    
    // Allocate memory
    values_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      values_[x] = new int[height_];
    }
    newValues_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      newValues_[x] = new int[height_];
    }
    
    // Init values
    srand(time(NULL));
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = 0;
      }
    }
  }

  ~Sandpile() {
    for (int x=0; x<width_; ++x) {
      delete [] values_[x];
    }
    delete [] values_;
    for (int x=0; x<width_; ++x) {
      delete [] newValues_[x];
    }
    delete [] newValues_;
  }

  void Run() {
    while (running()) {
      // Drop a sand grain in the centre
      values_[width_/2][height_/2]++;
      updateValues();
      
      for (int x=0; x<width_; ++x) {
        for (int y=0; y<height_; ++y) {
          switch (values_[x][y]) {
            case 0:
              canvas()->SetPixel(x, y, 0, 0, 0);
              break;
            case 1:
              canvas()->SetPixel(x, y, 0, 0, 200);
              break;
            case 2:
              canvas()->SetPixel(x, y, 0, 200, 0);
              break;
            case 3:
              canvas()->SetPixel(x, y, 150, 100, 0);
              break;
            default:
              canvas()->SetPixel(x, y, 200, 0, 0);
          }
        }
      }
      usleep(delay_ms_ * 1000); // ms
    }
  }

private:
  void updateValues() {
    // Copy values to newValues
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        newValues_[x][y] = values_[x][y];
      }
    }

    // Update newValues based on values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        if (values_[x][y] > 3) {
          // Collapse
          if (x>0)
            newValues_[x-1][y]++;
          if (x<width_-1)
            newValues_[x+1][y]++;
          if (y>0)
            newValues_[x][y-1]++;
          if (y<height_-1)
            newValues_[x][y+1]++;
          newValues_[x][y] -= 4;
        }
      }
    }
    // Copy newValues to values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = newValues_[x][y];
      }
    }
  }

  int width_;
  int height_;
  int** values_;
  int** newValues_;
  int delay_ms_;
};


// Conway's game of life
// Contributed by: Vliedel
class GameLife : public ThreadedCanvasManipulator {
public:
  GameLife(Canvas *m, int delay_ms=500, bool torus=true)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms), torus_(torus) {
    width_ = canvas()->width();
    height_ = canvas()->height();
    
    // Allocate memory
    values_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      values_[x] = new int[height_];
    }
    newValues_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      newValues_[x] = new int[height_];
    }
    
    // Init values randomly
    srand(time(NULL));
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y]=rand()%2;
      }
    }
    r_ = rand()%255;
    g_ = rand()%255;
    b_ = rand()%255;
    
    if (r_<150 && g_<150 && b_<150) {
      int c = rand()%3;
      switch (c) {
        case 0:
          r_ = 200;
          break;
        case 1:
          g_ = 200;
          break;
        case 2:
          b_ = 200;
          break;
      }
    }
  }

  ~GameLife() {
    for (int x=0; x<width_; ++x) {
      delete [] values_[x];
    }
    delete [] values_;
    for (int x=0; x<width_; ++x) {
      delete [] newValues_[x];
    }
    delete [] newValues_;
  }

  void Run() {
    while (running()) {
      
      updateValues();
      
      for (int x=0; x<width_; ++x) {
        for (int y=0; y<height_; ++y) {
          if (values_[x][y])
            canvas()->SetPixel(x, y, r_, g_, b_);
          else
            canvas()->SetPixel(x, y, 0, 0, 0);
        }
      }
      usleep(delay_ms_ * 1000); // ms
    }
  }

private:
  int numAliveNeighbours(int x, int y) {
    int num=0;
    if (torus_) {
      // Edges are connected (torus)
      num += values_[(x-1+width_)%width_][(y-1+height_)%height_];
      num += values_[(x-1+width_)%width_][y                    ];
      num += values_[(x-1+width_)%width_][(y+1        )%height_];
      num += values_[(x+1       )%width_][(y-1+height_)%height_];
      num += values_[(x+1       )%width_][y                    ];
      num += values_[(x+1       )%width_][(y+1        )%height_];
      num += values_[x                  ][(y-1+height_)%height_];
      num += values_[x                  ][(y+1        )%height_];
    }
    else {
      // Edges are not connected (no torus)
      if (x>0) {
        if (y>0)
          num += values_[x-1][y-1];
        if (y<height_-1)
          num += values_[x-1][y+1];
        num += values_[x-1][y];
      }
      if (x<width_-1) {
        if (y>0)
          num += values_[x+1][y-1];
        if (y<31)
          num += values_[x+1][y+1];
        num += values_[x+1][y];
      }
      if (y>0)
        num += values_[x][y-1];
      if (y<height_-1)
        num += values_[x][y+1];
    }
    return num;
  }
  
  void updateValues() {
    // Copy values to newValues
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        newValues_[x][y] = values_[x][y];
      }
    }
    // update newValues based on values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        int num = numAliveNeighbours(x,y);
        if (values_[x][y]) {
          // cell is alive
          if (num < 2 || num > 3)
            newValues_[x][y] = 0;
        }
        else {
          // cell is dead
          if (num == 3)
            newValues_[x][y] = 1;
        }
      }
    }
    // copy newValues to values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = newValues_[x][y];
      }
    }
  }

  int** values_;
  int** newValues_;
  int delay_ms_;
  int r_;
  int g_;
  int b_;
  int width_;
  int height_;
  bool torus_;
};

// Langton's ant
// Contributed by: Vliedel
class Ant : public ThreadedCanvasManipulator {
public:
  Ant(Canvas *m, int delay_ms=500)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
    numColors_ = 4;
    width_ = canvas()->width();
    height_ = canvas()->height();
    values_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      values_[x] = new int[height_];
    }
  }

  ~Ant() {
    for (int x=0; x<width_; ++x) {
      delete [] values_[x];
    }
    delete [] values_;
  }

  void Run() {
    antX_ = width_/2;
    antY_ = height_/2-3;
    antDir_ = 0;
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = 0;
        updatePixel(x, y);
      }
    }
    
    while (running()) {      
      // LLRR
      switch (values_[antX_][antY_]) {
        case 0:
        case 1:
          antDir_ = (antDir_+1+4) % 4;
          break;
        case 2:
        case 3:
          antDir_ = (antDir_-1+4) % 4;
          break;
      }
      
      values_[antX_][antY_] = (values_[antX_][antY_] + 1) % numColors_;
      int oldX = antX_;
      int oldY = antY_;
      switch (antDir_) {
        case 0:
          antX_++;
          break;
        case 1:
          antY_++;
          break;
        case 2:
          antX_--;
          break;
        case 3:
          antY_--;
          break;
      }
      updatePixel(oldX, oldY);
      if (antX_ < 0 || antX_ >= width_ || antY_ < 0 || antY_ >= height_)
        return;
      updatePixel(antX_, antY_);
      usleep(delay_ms_ * 1000);
    }
  }

private:
  void updatePixel(int x, int y) {
    switch (values_[x][y]) {
      case 0:
        canvas()->SetPixel(x, y, 200, 0, 0);
        break;
      case 1:
        canvas()->SetPixel(x, y, 0, 200, 0);
        break;
      case 2:
        canvas()->SetPixel(x, y, 0, 0, 200);
        break;
      case 3:
        canvas()->SetPixel(x, y, 150, 100, 0);
        break;
    }
    if (x == antX_ && y == antY_)
      canvas()->SetPixel(x, y, 0, 0, 0);
  }

  int numColors_;
  int** values_;
  int antX_;
  int antY_;
  int antDir_; // 0 right, 1 up, 2 left, 3 down
  int delay_ms_;
  int width_;
  int height_;
};



// Imitation of volume bars
// Purely random height doesn't look realistic
// Contributed by: Vliedel
class VolumeBars : public ThreadedCanvasManipulator {
public:
  VolumeBars(Canvas *m, int delay_ms=50, int numBars=8)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms),
      numBars_(numBars), t_(0) {
  }

  ~VolumeBars() {
    delete [] barHeights_;
    delete [] barFreqs_;
    delete [] barMeans_;
  }

  void Run() {
    const int width = canvas()->width();
    height_ = canvas()->height();
    barWidth_ = width/numBars_;
    barHeights_ = new int[numBars_];
    barMeans_ = new int[numBars_];
    barFreqs_ = new int[numBars_];
    heightGreen_  = height_*4/12;
    heightYellow_ = height_*8/12;
    heightOrange_ = height_*10/12;
    heightRed_    = height_*12/12;
    
    // Array of possible bar means
    int numMeans = 10;
    int means[10] = {1,2,3,4,5,6,7,8,16,32};
    for (int i=0; i<numMeans; ++i) {
      means[i] = height_ - means[i]*height_/8;
    }
    // Initialize bar means randomly
    srand(time(NULL));
    for (int i=0; i<numBars_; ++i) {
      barMeans_[i] = rand()%numMeans;
      barFreqs_[i] = 1<<(rand()%3);
    }
    
    // Start the loop
    while (running()) {
      if (t_ % 8 == 0) {
        // Change the means
        for (int i=0; i<numBars_; ++i) {
          barMeans_[i] += rand()%3 - 1;
          if (barMeans_[i] >= numMeans)
            barMeans_[i] = numMeans-1;
          if (barMeans_[i] < 0)
            barMeans_[i] = 0;
        }
      }
      
      // Update bar heights
      t_++;
      for (int i=0; i<numBars_; ++i) {
        barHeights_[i] = (height_ - means[barMeans_[i]])
          * sin(0.1*t_*barFreqs_[i]) + means[barMeans_[i]];
        if (barHeights_[i] < height_/8)
          barHeights_[i] = rand() % (height_/8) + 1;
      }
      
      for (int i=0; i<numBars_; ++i) {
        int y;
        for (y=0; y<barHeights_[i]; ++y) {
          if (y<heightGreen_) {
            drawBarRow(i, y, 0, 200, 0);
          }
          else if (y<heightYellow_) {
            drawBarRow(i, y, 150, 150, 0);
          }
          else if (y<heightOrange_) {
            drawBarRow(i, y, 250, 100, 0);
          }
          else {
            drawBarRow(i, y, 200, 0, 0);
          }
        }
        // Anything above the bar should be black
        for (; y<height_; ++y) {
          drawBarRow(i, y, 0, 0, 0);
        }
      }
      usleep(delay_ms_ * 1000);
    }
  }

private:
  void drawBarRow(int bar, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t x=bar*barWidth_; x<(bar+1)*barWidth_; ++x) {
      canvas()->SetPixel(x, height_-1-y, r, g, b);
    }
  }

  int delay_ms_;
  int numBars_;
  int* barHeights_;
  int barWidth_;
  int height_;
  int heightGreen_;
  int heightYellow_;
  int heightOrange_;
  int heightRed_;
  int* barFreqs_;
  int* barMeans_;
  int t_;
};
#endif
