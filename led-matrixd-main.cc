extern "C" {
	#include "ini_config.h"
}
#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <utility>	


using std::min;
using std::max;
using namespace rgb_matrix;

#include "canvas.h"

namespace ledMatrixD
{
	class FileStatusNotifee {
		public:
			virtual void notify(std::string fileName, int event) = 0;
	};

	class FileCreatedStatusObserver { 
		typedef std::map<int, FileStatusNotifee*> fileNotificationMap; 
		int fd;	
		private:
			fileNotificationMap notificationMap;	
		public:
			FileCreatedStatusObserver(){
				this->fd = inotify_init();
				if (this->fd == -1) {
					perror("inotify_init1");
					exit(EXIT_FAILURE);
				}

			}
			int registerForNotifications(std::string dirName, std::string fileName, FileStatusNotifee* notifee)
			{
				int ifd = inotify_add_watch(this->fd, dirName.c_str(), IN_CREATE | IN_DELETE);
				if (ifd == -1) {
					perror("inotify_add_watch");
					exit(EXIT_FAILURE);
				}
				notificationMap.insert(std::make_pair(ifd, notifee));	
				if(checkFileExists(fileName)){
					notifee->notify(fileName, 1);		
				}else{
					notifee->notify(fileName, 0);
				}
				return ifd;
			}
			void observe()
			{	
				fd_set ifs;
				int fd_set_size = this->fd + 1;
				while(true){
					int num_fds = 0;
					printf("waiting for events to happen in signcfg/ directory\n");
					FD_ZERO(&ifs);
					FD_SET(fd, &ifs);
					FD_SET(0, &ifs);
					num_fds = select(fd_set_size, &ifs, NULL, NULL, NULL);
					printf("got num_fds=%d\n", num_fds);
					if (num_fds == -1){
						perror("select()");
						exit(EXIT_FAILURE);
					}else if (num_fds){
						printf("directory something happened.\n");
						if (FD_ISSET(fd, &ifs)){
							this->notifyOfIEvents();
						}else if(FD_ISSET(0, &ifs)){
							int len;
							char buf[200];
							printf("got something from stdin\n");
							len = read(0, (void*)buf, 200 * sizeof(char));
							printf("read %d\n", len);
							if (len == -1 && errno != EAGAIN){
								perror("read() start of inotify struct");
								exit(EXIT_FAILURE);
							}
							buf[len] = '\0';
							printf("%s", buf);
						}
					}
				}	
			}
			void notifyOfIEvents()
			{
				char ieventbuf[200];
				struct inotify_event* event = NULL;
				int len = 0;
				event = (struct inotify_event*)&(ieventbuf[0]);
				printf("something happened in signcfg/.\n");
				len = read(fd, &ieventbuf[0], 200 * sizeof(char));
				if (len == -1 && errno != EAGAIN){
					perror("read() start of inotify struct");
					exit(EXIT_FAILURE);
				}
				if (event->mask & IN_CREATE)
					printf("CREATED: ");
				else if(event->mask & IN_DELETE)
					printf("DELETED: ");	

				if(event->len)
					printf("%s", event->name);

				if (event->mask & IN_ISDIR)
					printf(" [directory]\n");
				else
					printf(" [file]\n");

				std::string fileName = event->name;
				FileStatusNotifee* notifee = notificationMap[event->wd];				
				if (event->mask & IN_CREATE)
					notifee->notify(fileName, 2);	
				else if(event->mask & IN_DELETE)
					notifee->notify(fileName, 3);
			}

		private:
			inline bool checkFileExists(std::string fileName) 	
			{
				std::ifstream f(fileName.c_str());
				if(f.good()){
					f.close();
					return true;
				}
				f.close();
				return false;
			}
	};

	int pwm_bits = -1;
	bool large_display = false;
	int demo = -1;
	int rows = 32;
	int chain = 1;
	int scroll_ms = 30;
	bool do_luminance_correct = true;
	rgb_matrix::GPIO io;

	class OpenCloseSign : public FileStatusNotifee { 
		rgb_matrix::Canvas* canvas;
		rgb_matrix::ThreadedCanvasManipulator* image_gen;
		FileCreatedStatusObserver* observer;
		std::string isOpenFileName;
		bool isOpen;
		public:
			OpenCloseSign() : canvas(NULL), image_gen(NULL) {
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
			void run()
			{
				this->observer->registerForNotifications("signcfg/", isOpenFileName, this);
				this->observer->observe();	
			}
			void notify(std::string fileName, int event)
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

namespace ini { 
	//FILE SYSTEM
	// filesystem locations for various things// should be changed for final deployment

	// base directory for images
	char* image_basedir = NULL;

	// directories within image_basedir

	// frames for the atom animation
	char* atom_dir = NULL;
	// notable images for random display
	char* sprite_dir = NULL;

	// location of the shop status file
	char* shop_status_flag = NULL;

	// location of font files
	char* font_dir = NULL;


	//TIMIMG
	// all values in this section are milliseconds

	// scroll speed
	int scroll_ms = 0;
	// how long to show mf000 before spinning
	int atom_static_prespin_dur = 0;
	// how long to show mf000 after spinning
	int atom_static_postspin_dur = 0;

	// how long each frame of the spinning atom is shown
	int atom_frame_dur = 0;
	// how long to display the date
	int date_dur = 0;
	// how long to let Conway's Game of Life run
	int conway_dur = 0;
	// how long to display Perlin noise (future feature)
	// perlin_dur = 20000
	// how long to display random image
	int sprite_dur = 0;


	//ITERATIONS
	// how may times things should be repeated

	// how many times to spin the atom
	int atom_spin = 0;
	// how many times to scroll the shop status
	int status_scroll = 0;
	// how many times to scroll the title
	int title_scroll = 0;
	// how many times to scroll the twitter
	int twitter_scroll = 0;
	// how many times to scroll the url
	
	// how many times to scroll the url
	char* url_scroll = NULL;


	//DATE	
	// format string to be passed to date call
	char* date_format = NULL;
	// matrix x axis offset
	int date_x = 0;
	// matrix y axis offset
	int date_y = 0;
	// font to use for date display
	char* date_font = NULL;
	// date color
	int date_r = 0;
	int date_g = 0;
	int date_b = 0;

	void print_string(char* name, char* value){
		printf("\t[%s] %s\n", name, value);
	}
	void print_int(char* name, int value){
		printf("\t[%s] %d\n", name, value);
	}
	void print_section(char* name){
		printf("[%s]\n", name);
	}
	void print_ini_variables(){
		
	}
	
	void read_file(const char* file){
		struct ::collection_item* ini_config = NULL;
		struct ::collection_item* errors = NULL;
		//read ini file with all the configuration options 
		int res = ::config_from_file("led-matrixd", file, &ini_config, INI_STOP_ON_ERROR, &errors);
		if(res != 0){
			perror("read_ini_file()");
			exit(EXIT_FAILURE);
		}   	
		int num_sections = 0;
		char** section_list = ::get_section_list(ini_config, &num_sections, NULL); 		
		if(section_list == NULL){
			perror("ini sections");
			exit(EXIT_FAILURE);
		}
		int i;
		for(i = 0; i < num_sections; i++){
			printf("section_name: %s\n", section_list[i]);
			int num_attr = 0;
			char** attr_list = get_attribute_list(ini_config, section_list[i], &num_attr, NULL);
			int j;
			for(j = 0; j < num_attr; j++){
				struct ::collection_item* item;
				int ares = get_config_item(section_list[i], attr_list[j], ini_config, &item);
				if(ares != 0){
					perror("get_config_item()");
					exit(EXIT_FAILURE);
				}   	
				char* item_string = get_string_config_value(item, NULL);
				if(item_string == NULL){
					perror("get_string_config_value()");
					exit(EXIT_FAILURE);
				}
				printf("\t[%s] %s\n", attr_list[j], item_string);
			}
		}	
	}
}

using namespace std;

int main(int argc, char* argv[])
{
	//ledMatrixD::OpenCloseSign* openClose  = new ledMatrixD::OpenCloseSign();
	//openClose->run();
	ini::read_file("matrix.ini");
	return 0;
}

