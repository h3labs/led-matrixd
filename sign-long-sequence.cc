#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "ini-reader.h"
#include "graphics.h"
#include "sign-long-sequence.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>



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
   * class DateDisplay
   *
   * displays a date for a specific amout of time
   *
   */
  DateDisplay::DateDisplay(){
    if(ini::get_int("DATE", "date_x", &(this->x)) != 0){
      this->undefinedMsg("DATE", "date_x");
    }
    if(ini::get_int("DATE", "date_y", &(this->y)) != 0){
      this->undefinedMsg("DATE", "date_y");
    }
    if(ini::get_int("DATE", "date_r", &(this->r)) != 0){
      this->undefinedMsg("DATE", "date_r");
    }
    if(ini::get_int("DATE", "date_g", &(this->g)) != 0){
      this->undefinedMsg("DATE", "date_g");
    }
    if(ini::get_int("DATE", "date_b", &(this->b)) != 0){
      this->undefinedMsg("DATE", "b");
    }
    //TODO: free up character strings
    //get format
    char* format = NULL;
    if(ini::get_string("DATE", "date_format", &(format)) != 0){
      this->undefinedMsg("DATE", "date_format");
    }
    this->format = format;
    //get font filename
    char* fontFilename = NULL;
    if(ini::get_string("DATE", "date_font", &(fontFilename)) != 0){
      this->undefinedMsg("DATE", "date_font");
    }
    this->fontFilename = fontFilename;
    if(!this->font.LoadFont(this->fontFilename.c_str())){
      EXIT_MSG("ini config: could not load specified font file DATE > date_font");
    }
  }
  void DateDisplay::show(){
    char buf[100];
    time_t rawtime;
    struct tm * timeinfo;
    rgb_matrix::Color color(this->r, this->g, this->b); 
    time(&rawtime);
    timeinfo = localtime (&rawtime);
    //TODO: check and make sure format is correctly added here
    strftime(buf, 100, this->format, timeinfo);
    //draw the string in buf 
    std::stringstream timeStream(buf);
    int cX = this->x;
    int cY = this->y;
    //TODO: check this functions work
    while(!timeStream.eof()){
      if((cY + this->font.height() ) > canvas->height())
      std::string strLine = timeStream.getline();
      if( strLine.size() == 0){
        continue;
      }
      rgb_matrix::DrawText(canvas, this->font, cX, cY + this->font.baseline(), color);
      cY += this->font.height();
    }
  }
  DateDisplay::~DateDisplay(){
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
   * class RandomSpriteDisplay
   *
   * shows an image from an random pick of all the
   * images contained in the sprites directory.
   *
   */
  RandomSpriteDisplay::RandomSpriteDisplay(){
    //initialize instance variables
    if(ini::get_int("TIMING", "sprite_dur", &(this->spriteDuration)) != 0){
      this->undefinedMsg("TIMING", "sprite_dur");
    }
    if(ini::get_int("ITERATIONS", "random_times", &(this->times)) != 0){
      this->undefinedMsg("ITERATIONS", "random_times");
    }
    char* sprite_dir = NULL;
    if(ini::get_string("FILE SYSTEM", "sprite_dir", &(sprite_dir)) != 0){
      this->undefinedMsg("FILE SYSTEM", "sprite_dir");
    }
    this->spritePath = sprite_dir;
    //initialize the filename map that will be randomly selected for display
    DIR* dir = opendir(this->getFullSpritePath().c_str());
    dirent* dirFile = NULL;
    if(dir == NULL){
      EXIT_MSG("ini configuration sprite_dir does not exist or is not a directory");
    }
    int i = 0;
    std::string suffix = ".ppm";
    while((dirFile = readdir(dir)) != NULL){
      if(dirFile->d_namlen >= suffix.size()){
        std::string filename = dirFile->d_name;
        if(filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0){
          //has the .ppm suffix check this just to be safe
          fileMap[i] = filename;
        }
      }
    }
    closedir(dir);
    //initialize random number generator and distribution
    this->distribution = new intDist(0, fileMap.size());
    this->distribution->reset();
  }
  void RandomSpriteDisplay::show(){
    for(int i = 0; i < this->times; i++){
      int randInt = this->distribution(generator);
      std::string filename = fileMap[randInt];
      this->loadImage(this->spritePath + filename);
      this->draw(0);
      this->wait(this->spriteDuration);
    }
  }
  RandomSpriteDisplay::~RandomSpriteDisplay(){
    delete distribution;
  }
  std::string RandomSpriteDisplay::getFullSpritePath(){
    return this->getFilePath(this->spritePath);
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
