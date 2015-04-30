#ifndef __BEACON_H__
#define __BEACON_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>

#include <string>
#include <fstream>
#include <map>
#include <vector>

#include "LedMatrixDMain.h"

namespace ledMatrixD {

	class Beacon {
		typedef struct {
			//TODO this will have synchronization problems sometimes
			//should implements better k
			sem_t done;
			char msg[400];
		} MessageStore;
		private:
		int fd;
		public:
		Beacon();
		void run();
		void stop();
		bool getOpen();
		std::string getMessage();
		private:
		void waitForINotifyEvents();
		void processINotifyEvents();
		void update(int);
		void readParameters();
		void callUpdateScript();
		void callRestoreScript();
		std::vector<std::string> split(std::string& str, std::string& del);
		std::string decode(std::string& str);
		inline int getPercentEncoded(std::string::iterator& it, std::string::iterator& endit){
			std::string::iterator itn;
			if(*it != '%'){
				return -1;
			}
			itn = std::next(it, 1);
			if(!(itn < endit && (isdigit(*itn) || (isalpha(*itn) && (*itn > 'A' && *itn < 'F'))))){
				return -1;
			}
			itn = std::next(it, 2);
			if(!(itn < endit && (isdigit(*itn) || (isalpha(*itn) && (*itn > 'A' && *itn < 'F'))))){
				return -1;
			}
			std::string nstr(it, itn+1);
			unsigned int num = 0;
			if(sscanf(nstr.c_str(),"%%%X", &num) == 1){
				DMSG("Got percent encoded string \"%s\" -> \'%c\'\n", nstr.c_str(), (char)num);
				return (int)num;
			}else{
				DMSG("Got nothing\n");
				return -1;
			}
		}
		inline std::string processReservedCharacters(std::string& str)
		{
			std::string nstr;
			for(auto it = str.begin(); it != str.end(); ++it){
				auto cit = decodeMap.find(*it);
				if(cit != decodeMap.end()){
					nstr += cit->second;
				}else{
					nstr += *it;
				}
			}
			return nstr;
		}
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
		inline double secondsSinceModified(std::string filepath)
		{
			struct ::stat attrib;			// create a file attribute structure
			stat(filepath.c_str(), &attrib);	// get the attributes of filepath file
			::time_t mt = attrib.st_mtime;
			::time_t ct = time(NULL);
			return difftime(ct, mt);
		}
		std::string shopStatusFilename;
		std::string updateScriptFilename;
		std::string restoreScriptFilename;
		std::map<char, char> decodeMap;
		std::vector<int> ignoreIEventQuantity;
		std::map<std::string, std::string> parameters;
		MessageStore* messageStore;
		pid_t id;
		bool* terminate;
		bool* open;
	};
}

#endif
