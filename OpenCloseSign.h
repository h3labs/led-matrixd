
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <iostream>

#include "file-status-notification.h"

namespace ledMatrixD {
  class OpenCloseSign : public FileStatusNotifee { 
    rgb_matrix::Canvas* canvas;
    rgb_matrix::ThreadedCanvasManipulator* image_gen;
    FileCreatedStatusObserver* observer;
    std::string isOpenFileName;
    bool isOpen;
    public:
    OpenCloseSign();
    void run();
    void notify(std::string fileName, int event);
    private:
    void showSign(bool first, bool newIsOpen)
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
  };
}
