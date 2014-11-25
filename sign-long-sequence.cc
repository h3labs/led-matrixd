#include <stdlib.h>
#include <stdio.h>
#include "ini-reader.h"
#include "led-matrixd-main.h"

namespace ledMatrixD {
  Canvas* canvas = NULL;
  Display::Display(){
    char* mainPathCStr = NULL;
    if(ini::get_string("FILE SYSTEM", "image_basedir", &mainPathCStr) != 0){
      this->undefinedMsg("FILE SYSTEM", "image_basedir");
    }
    this->mainPath = std::string(mainPathCStr);
  }
  void Display::getFilePath(char* filename){
    return std::string(this->mainPath) + "/" + fileName;
  }
  void Display::undefinedMsg(char* section, char* attr){
    fprintf(stderr, "led-matrixd: undefined %s > %s\n", section, attr);
    exit(EXIT_FAILURE);
  }
  /**
   * class TitleDisplay
   *
   * Display a title contained in PPM format in title_card.ppm.
   *
   */
  TitleDisplay::TitleDisplay(){
    if(ini::get_int("ITERATIONS", "title_scroll", &(this->scrollTitle)) != 0){
      this->undefinedMsg("ITERATIONS", "title_scroll");
    }
  }
  void TitleDisplay::show(){
    if(!this->loadPPM("")){
    }
  }
  void initLongSequence(){
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

    canvas = matrix;
  }
  void runLongSequence(){
  }
}
