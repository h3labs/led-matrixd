#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "ini-reader.h"
#include "led-matrixd-main.h"
#include "demo-canvas.h"
#include "sign-long-sequence.h"

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
					num_fds = select(fd_set_size, &ifs, NULL, NULL, NULL);
					printf("got num_fds=%d\n", num_fds);
					if (num_fds == -1){
						perror("select()");
						exit(EXIT_FAILURE);
					}else if (num_fds){
						printf("directory something happened.\n");
						if (FD_ISSET(fd, &ifs)){
							this->notifyOfIEvents();
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


using namespace std;

int sequence = 0;
int usage(const char* msg){
  printf( "led-matrixd: %s\n"
          "usage: led-matrixd -C [ini file] -D [sequence run]\n"
          "   C: file name (directory) where the ini configuration file\n"
          "      resides.\n"
          "   D: sequence to run repeatedly\n"
          "       0: just shown open and close depending if file exists\n" 
          "       1: show a custome sequence of images, close and open\n"
          "\n", msg);
}

int main(int argc, char* argv[])
{
  //TODO: getopt
  int opt;
  while ((opt = getopt(argc, argv, "C:D")) != -1) {
    switch (opt) {
    case 'C':
      ini::ini_file = optarg;
      break;
    case 'D':
      sequence = atoi(optarg);
      if( 0 <= sequence && sequence <= 1){
        usage("illegal value -D can only use 0 or 1");
        exit(EXIT_FAILURE);
      }
      break;
    }
  }
  ini::read_file(ini::ini_file);
  switch(sequence){
    case 0:
      ledMatrixD::OpenCloseSign* openClose  = new ledMatrixD::OpenCloseSign();
      openClose->run();
      break;
    case 1:
      ledMatrixD::runLongSequence();
      break;
  }
	return 0;
}

