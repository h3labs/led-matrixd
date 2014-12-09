#include "file-status-notification.h"

namespace ledMatrixD {
  FileCreatedStatusObserver::FileCreatedStatusObserver(){
    this->fd = inotify_init();
    if (this->fd == -1) {
      perror("inotify_init1");
      exit(EXIT_FAILURE);
    }
  }
  int FileCreatedStatusObserver::registerForNotifications(std::string dirName, std::string fileName, FileStatusNotifee* notifee)
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
  void FileCreatedStatusObserver::observe()
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
  void FileCreatedStatusObserver::notifyOfIEvents()
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
}
