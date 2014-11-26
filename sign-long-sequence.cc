#include <stdlib.h>
#include <stdio.h>
#include "ini-reader.h"
#include "sign-long-sequence.h"

#include <iostream>
#include <fstream>



namespace ledMatrixD {
  Canvas* canvas = NULL;
  Display::Display(){
    char* mainPathCStr = NULL;
    if(ini::get_string("FILE SYSTEM", "image_basedir", &mainPathCStr) != 0){
      this->undefinedMsg("FILE SYSTEM", "image_basedir");
    }
    this->mainPath = std::string(mainPathCStr);
  }
  std::string Display::getFilePath(std::string filename){
    return std::string(this->mainPath) + "/" + filename;
  }
  void Display::undefinedMsg(std::string section, std::string attr){
    std::cerr << "led-matrixd: undefined " << section << " > " << attr << std::endl;
    exit(EXIT_FAILURE);
  }
  /**
   * class TitleDisplay
   *
   * Display a title contained in PPM format in title_card.ppm.
   *
   */
  TitleDisplay::TitleDisplay() : titleScroll(0) {
    if(ini::get_int("ITERATIONS", "title_scroll", &(this->titleScroll)) != 0){
      this->undefinedMsg("ITERATIONS", "title_scroll");
    }
  }
  void TitleDisplay::show(){
    std::string titleFilename = "title_card.ppm";
    std::string fullTitleFilename = this->getFilePath(titleFilename);
    if(!this->LoadPPM(fullTitleFilename)){
      EXIT_MSG("could not find title file");
    }
    if(this->titleScroll < 0){
      EXIT_MSG("in ini configuration file ITERATIONS:title_scroll must be greater than zero");
    }
    //TODO: have this in a scroll method as it might generalized for other displays
    for(int i = 0; i < this->titleScroll; i++){
      //TODO: msecs change
      this->scroll(30);
    }
  }
  /**
   * class LogoDisplay
   *
   * displays the atom logo for a specific duration
   *
   */
  LogoDisplay::LogoDisplay(int type){
    switch(type){
      case 0:
        //prespin
        if(ini::get_int("TIMING", "atom_static_prespin_dur", &(this->duration)) != 0){
          this->undefinedMsg("TIMING", "atom_static_prespin_dur");
        }
        break;
      case 1:
        //postspin
        if(ini::get_int("TIMING", "atom_static_postspin_dur", &(this->duration)) != 0){
          this->undefinedMsg("TIMING", "atom_static_postspin_dur");
        }
        break;
    }
    if(this->duration < 0){
      EXIT_MSG("in ini configuration file TIMING:atom_static_[post|pre]spin_duration must be greater than zero");
    }
    char* atom_dir = NULL;
    if(ini::get_string("FILE SYSTEM", "atom_dir", &(atom_dir)) != 0){
      this->undefinedMsg("FILE SYSTEM", "atom_dir");
    }
    this->atomPath = atom_dir;
    //TODO: free atom dir
  }
  void LogoDisplay::show(){
    std::cout << "Logo: duration[" << this->duration << "]" << std::endl;
    this->loadImage(this->atomPath + "mf000.ppm");
    this->draw(0);
    this->wait((unsigned int)this->duration);
  }
  /**
   *
   * class SpinLogoDisplay
   *
   * show a series of images of the atom logo spinning
   * with a specific duration between frame.
   *
   */
  SpinLogoDisplay::SpinLogoDisplay(){
    if(ini::get_int("TIMING", "atom_frame_dur", &(this->frameDuration)) != 0){
      this->undefinedMsg("TIMING", "atom_static_pr");
    }
    if(ini::get_int("ITERATIONS", "atom_spin", &(this->atomSpin)) != 0){
      this->undefinedMsg("ITERATIONS", "atom_spin");
    }
    char* atom_dir = NULL;
    if(ini::get_string("FILE SYSTEM", "atom_dir", &(atom_dir)) != 0){
      this->undefinedMsg("FILE SYSTEM", "atom_dir");
    }
    this->atomPath = atom_dir;
    //TODO: free atom dir
  }
  void SpinLogoDisplay::show(){
    for(int j = 0; j < this->atomSpin; j++){
      for(int i = 0; i < 24; i++){
        char buf[40];
        sprintf(&buf[0], "mf%03d.ppm", i);
        //std::cout << "SpinLogo: showing pic[" << &buf[0] << "]" << std::endl;
        this->loadImage(this->atomPath + &buf[0]);
        this->draw(0);
        this->wait((unsigned int)this->frameDuration);
      }
    }
  }
  /**
   * class ShopStatusDisplay
   *
   * Show the current status of the shop depending on
   * wheather the beacon file exists
   *
   */
  ShopStatusDisplay::ShopStatusDisplay(){
    if(ini::get_int("TIMING", "scroll_ms", &(this->scrollMS)) != 0){
      this->undefinedMsg("TIMING", "scroll_ms");
    }
    if(ini::get_int("ITERATIONS", "status_scroll", &(this->scrolls)) != 0){
      this->undefinedMsg("ITERATIONS", "status_scroll");
    }
    char* beacon_filename = NULL;
    if(ini::get_string("FILE SYSTEM", "shop_status_flag", &(beacon_filename)) != 0){
      this->undefinedMsg("FILE SYSTEM", "shop_status_flag");
    }
    this->beaconFilename = beacon_filename;
    //TODO: free beacon filename
  }
  void ShopStatusDisplay::show(){
    for(int i = 0; i < this->scrolls; i++){
      if(this->beaconExists()){
        this->loadImage("open.ppm");
      }else{
        this->loadImage("closed.ppm");
      }
      this->scroll(this->scrollMS);
    }
  }
  bool ShopStatusDisplay::beaconExists(){
    std::ifstream f(beaconFilename.c_str());
    if(f.good()){
      f.close();
      return true;
    }
    f.close();
    return false;
  }
  /**
   * 
   * class ConwaysDisplay
   *
   * shows the game of life for a specific duration
   *
   */
  ConwaysDisplay::ConwaysDisplay(){
    if(ini::get_int("TIMING", "conway_dur", &(this->duration)) != 0){
      this->undefinedMsg("TIMING", "conway_dur");
    }
  }
  void ConwaysDisplay::show(){
    GameLife* gameLife = new GameLife(canvas, 400, true);
    gameLife->Start();
    this->wait((unsigned int)this->duration)
    delete gameLife;
  }
  /**
   *
   * class URLDisplay
   *
   * shows an image of an URL scrolled along the screen.
   */
  URLDisplay::URLDisplay(){
    if(ini::get_int("TIMING", "scroll_ms", &(this->scrollMS)) != 0){
      this->undefinedMsg("TIMING", "scroll_ms");
    }
    if(ini::get_int("ITERATIONS", "url_scroll", &(this->scrolls)) != 0){
      this->undefinedMsg("ITERATIONS", "url_scroll");
    }
  }
  void URLDisplay::show(){
      this->loadImage("url.ppm");
      for(int i = 0; i < this->scrolls; i++){
        this->scroll(this->scrollMS);
      }
  }
  /**
   *
   * class TwitterDisplay
   *
   * shows an image of an Twitter scrolled along the screen.
   */
  TwitterDisplay::TwitterDisplay(){
    if(ini::get_int("TIMING", "scroll_ms", &(this->scrollMS)) != 0){
      this->undefinedMsg("TIMING", "scroll_ms");
    }
    if(ini::get_int("ITERATIONS", "url_scroll", &(this->scrolls)) != 0){
      this->undefinedMsg("ITERATIONS", "url_scroll");
    }
  }
  void TwitterDisplay::show(){
      this->loadImage("twitter.ppm");
      for(int i = 0; i < this->scrolls; i++){
        this->scroll(this->scrollMS);
      }
  }

  /**
   * Functions
   */
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
    initLongSequence();
    TitleDisplay* titleDisplay = new TitleDisplay();
    LogoDisplay* logoDisplayPre = new LogoDisplay(0);
    SpinLogoDisplay* spinLogoDisplay = new SpinLogoDisplay();
    LogoDisplay* logoDisplayPost = new LogoDisplay(1);
    ShopStatusDisplay* shopStatusDisplay = new ShopStatusDisplay();
    titleDisplay->show();
    logoDisplayPre->show();
    spinLogoDisplay->show();
    logoDisplayPost->show();
    shopStatusDisplay->show();
  }
}
