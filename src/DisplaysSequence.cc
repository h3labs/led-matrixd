#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/time.h>
#include "INIReader.h"
#include "graphics.h"
#include "utf8-internal.h"
#include "DisplaysSequence.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>



namespace ledMatrixD {
	Canvas* canvas = NULL;
	Beacon* beacon = NULL;
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
#ifdef DEBUG
		std::cerr << "led-matrixd: undefined " << section << " > " << attr << std::endl;
#endif
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
#ifdef DEBUG
		std::cout << "(TitleDisplay) Started" << std::endl;
#endif
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
	 * class TaglineDisplay
	 *
	 * Display a tagline contained in PPM format in tagline.ppm.
	 *
	 */
	TaglineDisplay::TaglineDisplay() : taglineScroll(0) {
		if(ini::get_int("ITERATIONS", "tagline_scroll", &(this->taglineScroll)) != 0){
			this->undefinedMsg("ITERATIONS", "tagline_scroll");
		}
	}
	void TaglineDisplay::show(){
#ifdef DEBUG
		std::cout << "(TaglineDisplay) Started" << std::endl;
#endif
		std::string taglineFilename = "tagline.ppm";
		std::string fullTaglineFilename = this->getFilePath(taglineFilename);
		if(!this->LoadPPM(fullTaglineFilename)){
			EXIT_MSG("could not find tagline file");
		}
		if(this->taglineScroll < 0){
			EXIT_MSG("in ini configuration file ITERATIONS:tagline_scroll must be greater than zero");
		}
		//TODO: have this in a scroll method as it might generalized for other displays
		for(int i = 0; i < this->taglineScroll; i++){
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
#ifdef DEBUG
		std::cout << "(LogoDisplay) Started" << std::endl;
		std::cout << "Logo: duration[" << this->duration << "]" << std::endl;
#endif
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
#ifdef DEBUG
		std::cout << "(SpinLogoDisplay) Started" << std::endl;
#endif
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
			this->undefinedMsg("DATE", "date_b");
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

		// get special date format
		char* special_format = NULL;
		if(ini::get_string("DATE", "special_date_format", &(special_format)) != 0){
			this->undefinedMsg("DATE", "special_date_format");
		}
		this->special_date_format = special_format;
		if(this->special_date_format[0] != '"' || this->special_date_format[this->special_date_format.size()-1] != '"'){
			EXIT_MSG("ini config: special_date_format must be a string sorrounded by \" marks ");
		}else{
			this->special_date_format = this->special_date_format.substr(1, this->special_date_format.size()-2);
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
		DMSG("(Date Display) Started\n");
		char buf[100];
		time_t rawtime;
		struct tm * timeinfo;

		time(&rawtime);
		timeinfo = localtime (&rawtime);
		//TODO: check and make sure format is correctly added here
		strftime(buf, 100, this->format.c_str(), timeinfo);

		char secbuf[100];

		//TODO: check and make sure format is correctly added here
		strftime(secbuf, 100, this->special_date_format.c_str(), timeinfo);

		char* dateStr = secbuf;

#ifdef DEBUG
		std::cout << "Date section: '" << dateStr << "'" << std::endl;
		std::cout << "has section? " << ini::has_section(dateStr) << std::endl;
#endif
		if (ini::has_section(dateStr)){
			// we found a special date section, repopulate the color values
			int r;
			int g;
			int b;

			if(ini::get_int(dateStr, "date_r", &r) == 0 &&
					ini::get_int(dateStr, "date_g", &g) == 0 &&
					ini::get_int(dateStr, "date_b", &b) == 0){
				this->r = r;
				this->g = g;
				this->b = b;
#ifdef DEBUG
				std::cout << "Date configured for " << dateStr << " with colors " << this->r << ", " << this->g << ", " << this->b << std::endl;
#endif
			}
		}
		else {
#ifdef DEBUG
			std::cout << "Date configuration: default" << std::endl;
#endif
		}

		//draw the string in buf
#ifdef DEBUG
		std::cout << "time is \"" << buf << "\" using format " << this->format.c_str() << std::endl;
#endif
		std::string timeStr = buf;
#ifdef DEBUG
		std::cout << "outputting " << timeStr << std::endl;
#endif

#ifdef DEBUG
		std::cout << "time colors: " << this->r << ", " << this->g << ", " << this->b << std::endl;
#endif
		rgb_matrix::Color color(this->r, this->g, this->b);

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
	}
	void ShopStatusDisplay::show(){
#ifdef DEBUG
		std::cout << "(ShopStatusDisplay) Started" << std::endl;
#endif
		for(int i = 0; i < this->scrolls; i++){
			if(beacon->getOpen()){
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
#ifdef DEBUG
		std::cout << "(ConwaysDisplay) Showing game of life" << std::endl;
#endif
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
#ifdef DEBUG
		std::cout << "(URLDisplay) Showing URL" << std::endl;
#endif
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
#ifdef DEBUG
		std::cout << "Reading Directory \"" << this->getFullSpritePath() << "\"" << std::endl;
#endif
		int i = 0;
		std::string suffix = ".ppm";
		while((dirFile = readdir(dir)) != NULL){
			unsigned int len = strlen(dirFile->d_name);
			if(len >= suffix.size()){
				std::string filename = dirFile->d_name;
				if(filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0){
#ifdef DEBUG
					std::cout << "(RandomSpriteDisplay) Adding file \"" << filename << "\"" << std::endl;
#endif
					//has the .ppm suffix check this just to be safe
					fileMap[i] = filename;
					i++;
				}
			}
		}
		closedir(dir);
		//initialize random number generator and distribution
		time_t t;
		unsigned int seed = (unsigned int)time(&t);
#ifdef DEBUG
		std::cout << "(RandomSpriteDisplay) Seed is " << seed <<  std::endl;
#endif
		srand((unsigned int)seed);
	}
	void RandomSpriteDisplay::show(){
		for(int i = 0; i < this->times; i++){
			int randInt = rand() % fileMap.size();
#ifdef DEBUG
			std::cout << "(RandomSpriteDisplay) Random integer " << randInt <<  std::endl;
#endif
			std::string filename = fileMap[randInt];
#ifdef DEBUG
			std::cout << "(RandomSpriteDisplay) Showing \"" << filename << "\"" <<  std::endl;
#endif
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
#ifdef DEBUG
		std::cout << "(TwitterDisplay) Started" << std::endl;
#endif
		this->loadImage("twitter.ppm");
		for(int i = 0; i < this->scrolls; i++){
			this->scroll(this->scrollMS);
		}
	}

	/**
	 *
	 * class DateBannerDisplay
	 *
	 * shows an image of an Twitter scrolled along the screen.
	 */
	DateBannerDisplay::DateBannerDisplay(){
		if(ini::get_int("TIMING", "scroll_ms", &(this->scrollMS)) != 0){
			this->undefinedMsg("TIMING", "scroll_ms");
		}
		if(ini::get_int("ITERATIONS", "date_banner_scroll", &(this->scrolls)) != 0){
			this->undefinedMsg("ITERATIONS", "date_banner_scroll");
		}
		char* banner_dir = NULL;
		if(ini::get_string("FILE SYSTEM", "day_banner_dir", &(banner_dir)) != 0){
			this->undefinedMsg("FILE SYSTEM", "day_banner_dir");
		}
		this->dateBannerPath = banner_dir;

		// get special date format
		char* special_format = NULL;
		if(ini::get_string("DATE", "special_date_format", &(special_format)) != 0){
			this->undefinedMsg("DATE", "special_date_format");
		}
		this->special_date_format = special_format;
		if(this->special_date_format[0] != '"' || this->special_date_format[this->special_date_format.size()-1] != '"'){
			EXIT_MSG("ini config: special_date_format must be a string sorrounded by \" marks ");
		}else{
			this->special_date_format = this->special_date_format.substr(1, this->special_date_format.size()-2);
		}

	}
	void DateBannerDisplay::show(){
#ifdef DEBUG
		std::cout << "(DateBannerDisplay) Started" << std::endl;
#endif
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

		char buf[100];
		time_t rawtime;
		struct tm * timeinfo;

		time(&rawtime);
		timeinfo = localtime (&rawtime);
		//TODO: check and make sure format is correctly added here
		strftime(buf, 100, this->format.c_str(), timeinfo);
		std::string dateStr = std::string(buf);

		//draw the string in buf
		char* banner = NULL;
		char secbuf[100];
		time_t secrawtime;
		struct tm * sectimeinfo;

		time(&secrawtime);
		sectimeinfo = localtime (&secrawtime);
		//TODO: check and make sure format is correctly added here
		strftime(secbuf, 100, this->special_date_format.c_str(), sectimeinfo);

		char* secStr = secbuf;

		// populate date banner if necessary
		if (ini::has_section(secStr)){
			if(ini::get_string(secStr, "banner", &(banner)) != 0){
				this->undefinedMsg(secStr, "banner");
			}
			else{
#ifdef DEBUG
				std::cout << "preparing banner for " << secStr << std::endl;
#endif
				this->loadImage(this->dateBannerPath + &banner[0]);

				for(int i = 0; i < this->scrolls; i++){
					this->scroll(this->scrollMS);
				}
			}
		}
	}
	std::string DateBannerDisplay::getFullDateBannerPath(){
		return this->getFilePath(this->dateBannerPath);
	}

	MessageDisplay::MessageDisplay(){
		if(ini::get_int("TIMING", "scroll_ms", &(this->scrollMS)) != 0){
			this->undefinedMsg("TIMING", "scroll_ms");
		}
		if(ini::get_int("ITERATIONS", "message_scroll", &(this->scrolls)) != 0){
			this->undefinedMsg("ITERATIONS", "message_scroll");
		}
		char* fontDir = NULL;
		if(ini::get_string("FILE SYSTEM", "font_dir", &(fontDir)) != 0){
			this->undefinedMsg("FILE SYSTEM", "font_dir");
		}
		this->fontDir = fontDir;
		char* fontFilename = NULL;
		if(ini::get_string("MESSAGE", "message_font", &(fontFilename)) != 0){
			this->undefinedMsg("MESSAGE", "message_font");
		}
		this->fontFilename = fontFilename;
		std::string fontFullpath = (this->fontDir + this->fontFilename);
		DMSG("Loading font from %s\n", (fontFullpath.c_str()));
		if(!this->font.LoadFont(fontFullpath.c_str())){
			EXIT_MSG("ini config: could not load specified font file MESSAGE > message_font");
		}
		this->message = beacon->getMessage();
		const char *utf8_text = NULL;
		DMSG("Initial message \'%s\'\n", this->message.c_str());
		utf8_text = message.c_str();
		this->widths.clear();	
		while (*utf8_text) {
			DMSG("Character \'%c\' ", *utf8_text);
			const uint32_t cp = utf8_next_codepoint(utf8_text);
			const struct rgb_matrix::Font::Glyph *g = this->font.FindGlyph(cp);
			if (g == NULL) g = this->font.FindGlyph(0xFFFD);
			if (g == NULL) DMSG("Serious Error\n");
			DMSG("w[%d]\n", g->width);
			this->widths.push_back(g->width);
		}
		this->sumOfWidths = 0;
		for(auto it = this->widths.begin(); it != this->widths.end(); ++it)
			this->sumOfWidths += *it;

	}

	void MessageDisplay::show(){
		//rgb_matrix::DrawText(canvas, this->font, 			
		//find the total size of the string and each letter
		//get all the widths of every character in the message
		//only if we get a new message else just go ahead and
		//display this message
		//get the sum of all widths
		int midY = canvas->height() / 2 + (this->font.height() / 3);
		rgb_matrix::Color white(255, 255, 255);
		rgb_matrix::Color black(0, 0, 0);
		const char *utf8_text = NULL;
		for(int s = 0; s < this->scrolls; s++){
			std::string newMessage = beacon->getMessage();
			canvas->Clear();
			DMSG("Message sroll %d\n", s);
			if(this->message.compare(newMessage)){
				this->message = newMessage;
				DMSG("Got new message \'%s\'\n", this->message.c_str());
				utf8_text = message.c_str();
				this->widths.clear();	
				while (*utf8_text) {
					DMSG("Character \'%c\' ", *utf8_text);
					const uint32_t cp = utf8_next_codepoint(utf8_text);
					const struct rgb_matrix::Font::Glyph *g = this->font.FindGlyph(cp);
					if (g == NULL) g = this->font.FindGlyph(0xFFFD);
					if (g == NULL) DMSG("Serious Error\n");
					DMSG("w[%d]\n", g->width);
					this->widths.push_back(g->width);
				}
				this->sumOfWidths = 0;
				for(auto it = this->widths.begin(); it != this->widths.end(); ++it)
					this->sumOfWidths += *it;
			}

			DMSG("Pixel width of message %d\n", this->sumOfWidths);
			for(int x = 0; x < this->sumOfWidths; x++){
				//draw the string
				utf8_text = message.c_str();
				int i = 0;
				int xp = 0;
				while (*utf8_text) {
					if((-x + xp) > canvas->width()){
						break;
					}else if((-x + xp + this->widths[i]) < 0){
						xp += this->widths[i];
						utf8_next_codepoint(utf8_text);
						i++;
						continue;
					}
					//else draw the character
					const uint32_t cp = utf8_next_codepoint(utf8_text);
					xp += font.DrawGlyph(canvas, -x + xp, midY, white, cp);
					i++;
				}
				this->wait(this->scrollMS);
				//clear the string
				utf8_text = message.c_str();
				i = 0;
				xp = 0;
				while (*utf8_text) {
					if((-x + xp) > canvas->width()){
						break;
					}else if((-x + xp + this->widths[i]) < 0){
						xp += this->widths[i];
						utf8_next_codepoint(utf8_text);
						i++;
						continue;
					}
					//else draw the character
					const uint32_t cp = utf8_next_codepoint(utf8_text);
					xp += font.DrawGlyph(canvas, -x + xp, midY , black, cp);
					i += 1;
				}
			}
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
#ifdef DEBUG
		std::cout << "rows=" << rows << std::endl;
		std::cout << "chain=" << chain << std::endl;
		std::cout << "do_luminance_correct=" << do_luminance_correct << std::endl;
		std::cout << "pwm_bits=" << pwm_bits << std::endl;
#endif
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
			/*
			   new DateDisplay(),
			   new TitleDisplay(),
			   new TaglineDisplay(),
			   new LogoDisplay(0),
			   new SpinLogoDisplay(),
			   new LogoDisplay(1),
			   new ShopStatusDisplay(),
			   new RandomSpriteDisplay(),
			   new ConwaysDisplay(),
			   new URLDisplay(),
			   new TwitterDisplay()
			   */

			/*
			   new TitleDisplay(),
			   new LogoDisplay(0),
			   new SpinLogoDisplay(),
			   new LogoDisplay(1),
			   new TitleDisplay(),
			   new TaglineDisplay(),
			   new TitleDisplay(),
			   new DateDisplay(),
			   new TitleDisplay(),
			   new ShopStatusDisplay(),
			   new TitleDisplay(),
			   new ConwaysDisplay(),
			   new TitleDisplay(),
			   new URLDisplay(),
			   new TitleDisplay(),
			   new RandomSpriteDisplay(),
			   new TitleDisplay(),
			   new TwitterDisplay()
			   ------------------
			   */

			   new TitleDisplay(),

			   new LogoDisplay(0),
			   new SpinLogoDisplay(),
			   new LogoDisplay(1),

			   new DateDisplay(),
			   new TitleDisplay(),
			   new ShopStatusDisplay(),
			new MessageDisplay(),

			   new TaglineDisplay(),
			   new ConwaysDisplay(),
			   new URLDisplay(),

			   new DateBannerDisplay(),
			   new TwitterDisplay(),
			   new RandomSpriteDisplay(),
				new MessageDisplay()
		};
		std::vector<Display*> displaysVector(displays, displays + (sizeof(displays) / sizeof(Display*)));
		for(std::vector<Display*>::iterator it = displaysVector.begin(); it != displaysVector.end();){
			DMSG("Drawing new display\n");
			(*it)->show();
			if((*it) == displaysVector.back())
				it = displaysVector.begin();
			else
				++it;
		}

	}
}
