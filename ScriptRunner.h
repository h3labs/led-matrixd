#include <pthread.h>

namespace ledMatrixD {
  class ScriptRunner : public FileStatusNotifee {
      ScriptRunner();
      pid_t run();
      void stop();
      void notify(std::string fileName, int event);
      ~ScriptRunner();
    private
      std::string shopStatusFilename;
      std::string shopStatusUpdateScriptFilename;
      FileCreatedStatusObserver* observer;
      bool* terminate;
      bool open;
      pid_t pid;
      pthread_t runner;
  }
}
