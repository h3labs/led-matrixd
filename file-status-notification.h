
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
    FileCreatedStatusObserver();
    int registerForNotifications(std::string dirName, std::string fileName, FileStatusNotifee* notifee);
    void observe();
    void notifyOfIEvents();
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
}
