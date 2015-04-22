#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <string.h>

#include "ini-reader.h"
#include "Beacon.h"

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

	Beacon::Beacon(){
		this->fd = inotify_init();
		if (this->fd == -1) {
			perror("inotify_init1");
			exit(EXIT_FAILURE);
		}
		//get string for location of beacon file
		char* shopStatusFilename = NULL;
		if(ini::get_string("FILE SYSTEM", "shop_status_flag", &shopStatusFilename) != 0){
			printf("ini config: could not find variable FILE SYSTEM > shop_status_flag");
			exit(EXIT_FAILURE);
		}
		this->shopStatusFilename = std::string(shopStatusFilename);
		//get string for location of script file
		char* updateScriptFilename = NULL;
		if(ini::get_string("FILE SYSTEM", "shop_status_update_script", &updateScriptFilename) != 0){
			printf("ini config: could not find variable FILE SYSTEM > shop_status_update_script");
			exit(EXIT_FAILURE);
		}
		this->updateScriptFilename = std::string(updateScriptFilename);
		//add inotify watch for directory
		std::string dirname = ledMatrixD::path(this->shopStatusFilename);
		std::string filename = ledMatrixD::basename(this->shopStatusFilename);
		DMSG("Registering to listen (%s, %s)\n", dirname.c_str(), filename.c_str());
		int ifd = inotify_add_watch(this->fd, dirname.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_ATTRIB);
		if (ifd == -1) {
			DMSG("Error trying to add inotify watch to file\n");
			perror("inotify_add_watch()");
			exit(EXIT_FAILURE);
		}
		//create memory mapped between both processes child and parent
		this->terminate = (bool*)mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if(this->terminate == MAP_FAILED){
			DMSG("mmap() failed unable to allocate shared memory\n");
		}
		*this->terminate = false;
		std::string fullPathFilename = dirname + "/" + filename;
		if(checkFileExists(fullPathFilename)){
			this->update(1);
		}else{
			this->update(0);
		}
	}
	void Beacon::run()
	{
		//for a process that will run a loop to wait for inotify events
		//to happen in the directory of the beacon file
		DMSG("Run loop to observe if something has changed in beacon file\n");
		pid_t id = fork();
		if(id >= 0){
			if(id == 0){
				DMSG("(child) process started\n");
				this->waitForINotifyEvents();
				munmap(this->terminate, sizeof(bool));
				//this->pid = id;
			}else{
				DMSG("(root) process passing here\n");
				//this->pid = id;
			}
		}else{
			DMSG("fork() failed\n");
			exit(EXIT_FAILURE);
		}
	}
	void Beacon::stop(){
		//stop the threaded created by this class
		*this->terminate = true;
		//find a way to wake up notificator
	}
	void Beacon::waitForINotifyEvents()
	{
		fd_set ifs;
		int fd_set_size = this->fd + 1;
		while(true){
			int num_fds = 0;
			DMSG("Wait for events on beacon directory\n");
			FD_ZERO(&ifs);
			FD_SET(this->fd, &ifs);
			DMSG("Watch descriptor is %d\n", this->fd);
			//wait here until an new event comes up
			num_fds = select(fd_set_size, &ifs, NULL, NULL, NULL);
			DMSG("Highest descriptor received is %d\n", num_fds);
			if (num_fds == -1){
				perror("select()");
				exit(EXIT_FAILURE);
			}else if (num_fds){
				DMSG("New events on beacon directory\n");
				if (FD_ISSET(fd, &ifs)){
					this->processINotifyEvents();
				}
			}
		}

	}
	void Beacon::processINotifyEvents()
	{
		char ieventbuf[200];
		struct inotify_event* event = NULL;
		int len = 0; /*length read initially*/
		int llen = 0; /*length left to be read*/
		event = (struct inotify_event*)&(ieventbuf[0]);
		DMSG("Something observed in directory\n");
		len = read(fd, &ieventbuf[0], 200 * sizeof(char));
		if (len == -1 && errno != EAGAIN){
			perror("read() start of inotify struct");
			exit(EXIT_FAILURE);
		}
		llen = len;
		DMSG("Total read length is %d bytes\n",len);
		do{
			size_t ievent_size = 0;
			DMSG("");
			if (event->mask & IN_CREATE)
				printf("CREATED: ");
			else if(event->mask & IN_DELETE)
				printf("DELETED: ");
			else if(event->mask & IN_MODIFY)
				printf("MODIFIED: ");
			else if(event->mask & IN_ATTRIB)
				printf("ATTRIBUTE: ");
			ievent_size += sizeof(int); /*wd*/
			ievent_size += sizeof(uint32_t); /*mask*/
			ievent_size += sizeof(uint32_t); /*cookie*/
			ievent_size += sizeof(uint32_t); /*len*/
			ievent_size += event->len;
			if(event->len)
				printf("%s", event->name);

			if (event->mask & IN_ISDIR)
				printf(" [directory]\n");
			else
				printf(" [file]\n");
			DMSG("Total inotify_event size is %lu bytes\n", ievent_size);
			std::string filename = event->name;
			//update for event
			if(!filename.compare(ledMatrixD::basename(this->shopStatusFilename))){
				if (event->mask & IN_CREATE)
					this->update(2);
				else if(event->mask & IN_DELETE)
					this->update(3);
				else if(event->mask & IN_MODIFY)
					this->update(4);
				else if(event->mask & IN_ATTRIB)
					this->update(5);
			}
			llen -= ievent_size;
			if(llen > 0)
				event = (struct inotify_event*)&(ieventbuf[ievent_size]);
		}while(llen > 0);
		DMSG("Finished processing inotify notifications\n");
	}
	//call the update script file
	void Beacon::update(int updateType)
	{
		DMSG("Got event update of type %d\n", updateType);	
		//call the script and update website
		switch(updateType){
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
				DMSG("Event did not correspond to any type possibility\n");
				exit(EXIT_FAILURE);
		}
		//call update script
		pid_t pid;
		pid = fork ();
		if(pid == 0){
			DMSG("(child) running script\n");
			if(this->open){
				DMSG("(child) %s [open]\n", this->updateScriptFilename.c_str());
				execl(this->updateScriptFilename.c_str(), "open", NULL);
				exit(EXIT_FAILURE);
			}else{
				DMSG("(child) %s [closed]\n", this->updateScriptFilename.c_str());
				execl(this->updateScriptFilename.c_str(), "closed", NULL);
				exit(EXIT_FAILURE);
			}
		}else if(pid < 0){
			DMSG("(parent) child not created... unable to call script \"%s\"", this->updateScriptFilename.c_str());
		}else{
			int status;
			DMSG("(parent) waiting...\n");
			if(waitpid (pid, &status, 0) != pid){
				DMSG("(parent) something's wrong...\n");
			}else{
				DMSG("(parent) child arrived!\n");
			}
		}
		DMSG("(parent) checking whether to terminate\n");
		if(*this->terminate){
			DMSG("terminate\n");
			exit(0);
		}
		DMSG("(parent) did not terminate\n");
	}
}
