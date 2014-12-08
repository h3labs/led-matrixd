#include "OpenCloseSign.h"

namespace ledMatrixD {
  OpenCloseSign::OpenCloseSign() : canvas(NULL), image_gen(NULL) {
    this->isOpenFileName = "shopisopen.beacon";
    // The matrix, our 'frame buffer' and display updater.
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

    this->canvas = matrix;

    /** TODO: THIS HAS A BUG WHEN COMPILING
      if (large_display) {
    // Mapping the coordinates of a 32x128 display mapped to a square of 64x64
    this->canvas = new rgb_matrix::LargeSquare64x64Canvas(this->canvas);
    }*/

    // The ThreadedCanvasManipulator objects are filling
    // the matrix continuously.
    this->image_gen = NULL;


    std::cout << "setting up notification for /shopisopen.beacon" << std::endl;
    this->observer = new FileCreatedStatusObserver();
  }
  void OpenCloseSign::run()
  {
    this->observer->registerForNotifications("signcfg/", isOpenFileName, this);
    this->observer->observe();	
  }
  void OpenCloseSign::notify(std::string fileName, int event)
  {
    std::cout << "got notification for file: " << fileName << " with event[" << event << "]" << std::endl;
    if(!fileName.compare(this->isOpenFileName)){
      switch(event){
        case 0:
          this->isOpen = true;	
          this->showSign(true, false); 
          this->isOpen = false;	
          break;
        case 1:
          this->isOpen = false;	
          this->showSign(true, true); 
          this->isOpen = true;	
          break;
        case 2:
          this->showSign(false, true); 
          this->isOpen = true;	
          break;
        case 3:
          this->showSign(false, false); 
          this->isOpen = false;	
          break;
        default:
          std::cout << "event did not correspond to any possibility" << std::endl;
          exit(EXIT_FAILURE);
      }
    }
  } 

  void OpenCloseSign::showSign(bool first, bool newIsOpen)
  {
    if(!first){
      if(newIsOpen != this->isOpen){
        //remove old led matrix sign
        delete image_gen;
        std::cout << "removing old scroller image" << std::endl;
      }
    }
    //create new matrix and display for either open or close
    if(newIsOpen != this->isOpen){
      if(newIsOpen){
        ImageScroller *scroller = new ImageScroller(this->canvas,
            demo == 1 ? 1 : -1,
            scroll_ms);
        if (!scroller->LoadPPM("signcfg/open.ppm")){
          fprintf(stderr, "failed to load ppm");
          exit(EXIT_FAILURE);
        }
        this->image_gen = scroller;
        std::cout << "Show \'open\'" << std::endl;
      }else{
        ImageScroller *scroller = new ImageScroller(this->canvas,
            demo == 1 ? 1 : -1,
            scroll_ms);
        if (!scroller->LoadPPM("signcfg/closed.ppm")){
          fprintf(stderr, "failed to load ppm");
          exit(EXIT_FAILURE);
        }
        this->image_gen = scroller;
        std::cout << "Show \'closed\'" << std::endl;
      }
      this->image_gen->Start();
    }
  } 
}
