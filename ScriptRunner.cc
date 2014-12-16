
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
    std::cout << "this happened3\n";
    char* shopStatusFilename = NULL;
    if(ini::get_string("FILE SYSTEM", "shop_status_flag", &shopStatusFilename) != 0){
      printf("ini config: could not find variable FILE SYSTEM > shop_status_flag");
      exit(EXIT_FAILURE);
    }
    this->shopStatusFilename = std::string(shopStatusFilename);
    //get string for location of script file
    std::cout << "this happened4\n";
    char* shopStatusUpdateScriptFilename = NULL;
    if(ini::get_string("FILE SYSTEM", "shop_status_update_script", &shopStatusUpdateScriptFilename) != 0){
      printf("ini config: could not find variable FILE SYSTEM > shop_status_update_script");
      exit(EXIT_FAILURE);
    }
    std::cout << "this happened4\n";
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
        std::cout << "child process running" << std::endl;
        this->observer = new FileCreatedStatusObserver();
        this->observer->registerForNotifications(
          ledMatrixD::path(this->shopStatusFilename), 
          ledMatrixD::basename(this->shopStatusFilename), 
          this
        );
        std::cout << "child process running2" << std::endl;
        this->observer->observe();
        munmap(this->terminate, sizeof(bool));
        pid = id;
      }else{
        std::cout << "parent process running" << std::endl;
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
    std::cout << "got notification for file: " << fileName << " with event[" << event << "]" << std::endl;
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
          std::cout << "event did not correspond to any possibility" << std::endl;
          exit(EXIT_FAILURE);
      }
      //call update script
      pid_t pid;
      pid = fork ();
      if(pid == 0){
        std::cout << "running script > " << std::endl;
        if(this->open){
          std::cout << "running script " << this->shopStatusUpdateScriptFilename << " [open]" << std::endl;
          execl(this->shopStatusUpdateScriptFilename.c_str(), "open", NULL);
          exit(EXIT_FAILURE);

        }else{
          std::cout << "running script " << this->shopStatusUpdateScriptFilename << " [closed]" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
    }else if(pid < 0){
      std::cout << "(parent) child not created... unable to call script \"" << this->shopStatusUpdateScriptFilename << "\""<< std::endl;
    }else{
      int status;
      std::cout << "(parent) waiting..." << std::endl;
      if(waitpid (pid, &status, 0) != pid){
        std::cout << "(parent) something's wrong..." << std::endl;
      }else{
        std::cout << "(parent) child arrived!" << std::endl;
      }
    }
    std::cout << "(parent) checking whether to terminate" << std::endl;
    /*
    if(*this->terminate){
      std::cout << "terminate" << std::endl;
      exit(0);
    }
    */
    std::cout << "(parent) did not termonate" << std::endl;
  }
  ScriptRunner::~ScriptRunner(){
  }
}
