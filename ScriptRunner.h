
#ifndef __SCRIPT_RUNNER_H__
#define __SCRIPT_RUNNER_H__

#include <pthread.h>

#include <string>

#include "file-status-notification.h"

namespace ledMatrixD {
  class ScriptRunner : public FileStatusNotifee {
    public:
      ScriptRunner();
      pid_t run();
      void stop();
      void notify(std::string fileName, int event);
      ~ScriptRunner();
    private:
      std::string shopStatusFilename;
      std::string shopStatusUpdateScriptFilename;
      FileCreatedStatusObserver* observer;
      bool* terminate;
      bool open;
      pid_t pid;
      pthread_t runner;
  };
}

#endif
