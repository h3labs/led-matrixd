#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/time.h>
#include "ini-reader.h"
#include "graphics.h"
#include "sign-long-sequence.h"
#include "ScriptRunner.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>



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
    if(ini::get_int("TIMING", "date_dur", &(this->dateDur)) != 0){
      this->undefinedMsg("TIMING", "date_dur");
    }
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
    char* fontDir = NULL;
    if(ini::get_string("FILE SYSTEM", "font_dir", &(fontDir)) != 0){
      this->undefinedMsg("FILE SYSTEM", "font_dir");
    }
    this->fontDir = fontDir;
    //get format
    char* format = NULL;
    if(ini::get_string("DATE", "date_format", &(format)) != 0){
      this->undefinedMsg("DATE", "date_format");
    }
    this->format = format;
    if(this->format[0] != '"' || this->format[this->format.size()-1] != '"'){
      EXIT_MSG("ini config: date_format must be a string sorrounded by \" marks ");
    }else{
      this->format = this->format.substr(1, this->format.size()-2);
    }
    //get font filename
    char* fontFilename = NULL;
    if(ini::get_string("DATE", "date_font", &(fontFilename)) != 0){
      this->undefinedMsg("DATE", "date_font");
    }
    this->fontFilename = fontFilename;
    if(!this->font.LoadFont((this->fontDir + this->fontFilename).c_str())){
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
    strftime(buf, 100, this->format.c_str(), timeinfo);
    //draw the string in buf 
#ifdef DEBUG
    std::cout << "time is \"" << buf << "\" using format " << this->format.c_str() << std::endl;
#endif
    std::string timeStr = buf;
#ifdef DEBUG
    std::cout << "outputing " << timeStr << std::endl;
#endif
    int cX = this->x;
    int cY = this->y;
    //TODO: check this functions work
    canvas->Clear();
    while(true){
      if((cY + this->font.height() ) > canvas->height()){
#ifdef DEBUG
        std::cout << "breaking because canvas is too small" << std::endl;
#endif
        break;
      }
      if( timeStr.size() == 0){
#ifdef DEBUG
        std::cout << "time does not contain anything" << std::endl;
#endif
        break;
      }
      std::string strLine;
      for(unsigned int i = 0; i < timeStr.size(); i++){
        if(timeStr[i] == '\n'){
          if( (i+1) >= timeStr.size()){
            strLine = timeStr.substr(0, i);
            timeStr = "";
          }else{
            strLine = timeStr.substr(0, i);
            timeStr = timeStr.substr(i+1);
          }
          break;
        }
        if( i >= (timeStr.size() - 1)){
          strLine = timeStr;
          timeStr = "";
          break;
        }
      }
#ifdef DEBUG
      std::cout << "writing line \"" << strLine << "\"" << std::endl;
#endif
      if( strLine.size() == 0){
#ifdef DEBUG
        std::cout << "line does not contain anything" << std::endl;
#endif
        break;
      }
      rgb_matrix::DrawText(canvas, this->font, cX, cY + this->font.baseline(), color, strLine.c_str());
      cY += this->font.height();
    }
    this->wait(dateDur);
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
    this->wait((unsigned int)this->duration);
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
    std::cout << "Reading Directory \"" << this->getFullSpritePath() << "\"" << std::endl;
    int i = 0;
    std::string suffix = ".ppm";
    while((dirFile = readdir(dir)) != NULL){
      std::cout << "adding file \"" << dirFile->d_name << "\"" << std::endl;
      unsigned int len = strlen(dirFile->d_name);
      if(len >= suffix.size()){
        std::string filename = dirFile->d_name;
        if(filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0){
          //has the .ppm suffix check this just to be safe
          fileMap[i] = filename;
        }
      }
    }
    closedir(dir);
    //initialize random number generator and distribution
    time_t t;
    unsigned int seed = (unsigned int)time(&t);
    srand((unsigned int)t);
  }
  void RandomSpriteDisplay::show(){
    for(int i = 0; i < this->times; i++){
      int randInt = rand() % fileMap.size();
      std::string filename = fileMap[randInt];
      this->loadImage(this->spritePath + filename);
      this->draw(0);
      this->wait(this->spriteDuration);
    }
  }
  RandomSpriteDisplay::~RandomSpriteDisplay(){
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
    Display* displays[] = {
      new DateDisplay(),
      new TitleDisplay(),
      new LogoDisplay(0),
      new SpinLogoDisplay(),
      new LogoDisplay(1),
      new ShopStatusDisplay(),
      new RandomSpriteDisplay(),
      new ConwaysDisplay(),
      new URLDisplay(),
      new TwitterDisplay()
    };
    std::vector<Display*> displaysVector(displays, displays + (sizeof(displays) / sizeof(Display*)));
    for(std::vector<Display*>::iterator it = displaysVector.begin(); it != displaysVector.end();){
      std::cout << "Drawing new display" << std::endl;
      (*it)->show();
      if((*it) == displaysVector.back())
        it = displaysVector.begin();
      else
        ++it;
    }
    /*
    TitleDisplay* titleDisplay = new TitleDisplay();
    LogoDisplay* logoDisplayPre = new LogoDisplay(0);
    SpinLogoDisplay* spinLogoDisplay = new SpinLogoDisplay();
    LogoDisplay* logoDisplayPost = new LogoDisplay(1);
    ShopStatusDisplay* shopStatusDisplay = new ShopStatusDisplay();
    RandomSpriteDisplay* randomSpriteDisplay = new RandomSpriteDisplay();
    titleDisplay->show();
    logoDisplayPre->show();
    spinLogoDisplay->show();
    logoDisplayPost->show();
    shopStatusDisplay->show();
    randomSpriteDisplay->show();
    */
  }
}
