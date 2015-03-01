
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string>
#include <iostream>

#include "ini-reader.h"
#include "ScriptRunner.h"

namespace ledMatrixD {
  std::string basename(std::string path){
    unsigned int lastSeparatorIndex = path.rfind("/");
    if(lastSeparatorIndex != std::string::npos){
      return path.substr(lastSeparatorIndex+1);
    }
    return std::string("");
  }
  std::string path(std::string path){
    unsigned int lastSeparatorIndex = path.rfind("/");
    if(lastSeparatorIndex != std::string::npos){
      return path.substr(0, lastSeparatorIndex+1);
    }
    return std::string("");
  }

  ScriptRunner::ScriptRunner(){
    //get string for location of beacon file
    char* shopStatusFilename = NULL;
    if(ini::get_string("FILE SYSTEM", "shop_status_flag", &shopStatusFilename) != 0){
      printf("ini config: could not find variable FILE SYSTEM > shop_status_flag");
      exit(EXIT_FAILURE);
    }
    this->shopStatusFilename = std::string(shopStatusFilename);
    //get string for location of script file
    char* shopStatusUpdateScriptFilename = NULL;
    if(ini::get_string("FILE SYSTEM", "shop_status_update_script", &shopStatusUpdateScriptFilename) != 0){
      printf("ini config: could not find variable FILE SYSTEM > shop_status_update_script");
      exit(EXIT_FAILURE);
    }
    this->shopStatusUpdateScriptFilename = std::string(shopStatusUpdateScriptFilename);
  }
  int ScriptRunner::run(){
    //create a thread to run this code on
    this->terminate = (bool*)mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(this->terminate == MAP_FAILED){
      std::cout << "mmap() failed unable to allocate shared memory" << std::endl;
    }
    *this->terminate = false;
    pid_t id = fork();
    if(id >= 0){
      if(id == 0){
#ifdef DEBUG
        std::cout << "(parent) Script runner process started" << std::endl;
#endif
        this->observer = new FileCreatedStatusObserver();
        this->observer->registerForNotifications(
            ledMatrixD::path(this->shopStatusFilename),
            ledMatrixD::basename(this->shopStatusFilename),
            this
            );
        this->observer->observe();
        munmap(this->terminate, sizeof(bool));
        pid = id;
      }else{
#ifdef DEBUG
        std::cout << "(root) process running" << std::endl;
#endif
        pid = id;
      }
    }else{
      std::cout << "fork() failed" << std::endl;
      exit(EXIT_FAILURE);
    }
    return id;
  }
  void ScriptRunner::stop(){
    //stop the threaded created by this class
    *this->terminate = true;
    //find a way to wake up notificator
  }
  void ScriptRunner::notify(std::string fileName, int event){
#ifdef DEBUG
    std::cout << "got notification for file: " << fileName << " with event[" << event << "]" << std::endl;
#endif
    //call the script and update website
    if(!fileName.compare(ledMatrixD::basename(this->shopStatusFilename))){
      switch(event){
        case 0:
          this->open = false;
          break;
        case 1:
          this->open = true;
          break;
        case 2:
          this->open = true;
          break;
        case 3:
          this->open = false;
          break;
        default:
#ifdef DEBUG
          std::cout << "event did not correspond to any possibility" << std::endl;
#endif
          exit(EXIT_FAILURE);
      }
      //call update script
      pid_t pid;
      pid = fork ();
      if(pid == 0){
#ifdef DEBUG
        std::cout << "(child) running script" << std::endl;
#endif
        if(this->open){
#ifdef DEBUG
          std::cout << "(child) ." << this->shopStatusUpdateScriptFilename << " [open]" << std::endl;
#endif
          execl(this->shopStatusUpdateScriptFilename.c_str(), "open", NULL);
          exit(EXIT_FAILURE);
        }else{
#ifdef DEBUG
          std::cout << "(child)." << this->shopStatusUpdateScriptFilename << " [closed]" << std::endl;
#endif
          execl(this->shopStatusUpdateScriptFilename.c_str(), "closed", NULL);
          exit(EXIT_FAILURE);
        }
      }else if(pid < 0){
#ifdef DEBUG
        std::cout << "(parent) child not created... unable to call script \"" << this->shopStatusUpdateScriptFilename << "\""<< std::endl;
#endif
      }else{
        int status;
#ifdef DEBUG
        std::cout << "(parent) waiting..." << std::endl;
#endif
        if(waitpid (pid, &status, 0) != pid){
#ifdef DEBUG
          std::cout << "(parent) something's wrong..." << std::endl;
#endif
        }else{
#ifdef DEBUG
          std::cout << "(parent) child arrived!" << std::endl;
#endif
        }
      }
#ifdef DEBUG
	std::cout << "(parent) checking whether to terminate" << std::endl;
#endif
      if(*this->terminate){
#ifdef DEBUG
        std::cout << "terminate" << std::endl;
#endif
        exit(0);
      }
#ifdef DEBUG
      std::cout << "(parent) did not terminate" << std::endl;
#endif
    }
  }
  ScriptRunner::~ScriptRunner(){
  }
}
