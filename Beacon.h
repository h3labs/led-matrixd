#ifndef __BEACON_H__
#define __BEACON_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/select.h>

#include <string>
#include <fstream>
#include <map>

#include "led-matrixd-main.h"

namespace ledMatrixD {

	class Beacon {
		private:
		int fd;
		public:
		Beacon();
		void run();
		void stop();
		private:
		void waitForINotifyEvents();
		void processINotifyEvents();
		void update(int);
		inline bool checkFileExists(std::string filename)
		{
			std::ifstream f(filename.c_str());
			if(f.good()){
				f.close();
				return true;
			}
			f.close();
			return false;
		}
		std::string shopStatusFilename;
		std::string updateScriptFilename;
		bool* terminate;
		bool open;
		//pid_t pid;
		pthread_t runner;

	};
}

#endif
