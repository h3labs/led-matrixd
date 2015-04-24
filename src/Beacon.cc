#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>      // std::ifstream
#include <iostream>
#include <string.h>
#include <vector>

#include "INIReader.h"
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
		//characters to decode as
		this->decodeMap['+'] = ' ';
		//quntity of times to ignore events received
		this->ignoreIEventQuantity = std::vector<int>(5,0);
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
		//get string for location of script file
		char* restoreScriptFilename = NULL;
		if(ini::get_string("FILE SYSTEM", "shop_status_restore_script", &restoreScriptFilename) != 0){
			printf("ini config: could not find variable FILE SYSTEM > shop_status_restore_script");
			exit(EXIT_FAILURE);
		}
		this->restoreScriptFilename = std::string(restoreScriptFilename);
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
			perror("mmap()");
			exit(EXIT_FAILURE);
		}
		*this->terminate = false;
		this->messageStore = (MessageStore*)mmap(NULL, sizeof(MessageStore), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if(this->messageStore == MAP_FAILED){
			DMSG("mmap() failed unable to allocate shared memory\n");
			perror("mmap()");
			exit(EXIT_FAILURE);
		}
		DMSG("Size of MessageStore is %lu\n", sizeof(MessageStore));
		DMSG("Size of MessageStore->msg is %lu\n", sizeof(this->messageStore->msg));
		if(sem_init(&(this->messageStore->done), 1, 1) != 0){
			DMSG("sem_init() failed\n");
			perror("sem_init()");
			exit(EXIT_FAILURE);
		}
		this->messageStore->msg[0] = '\0';
		//initialize message memory
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
		this->id = fork();
		if(this->id >= 0){
			if(this->id == 0){
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
	std::string Beacon::getMessage(){
		std::string rstr;
		if(this->id == 0){
			const char* msg;
			auto it = this->parameters.find("msg");
			if(it != this->parameters.end()){
				msg = it->second.c_str();
			}else{
				msg = std::string("").c_str();
			}
			size_t maxMsgSize = sizeof(this->messageStore->msg);
			size_t len = strlen(msg);
			sem_wait(&(this->messageStore->done));
				//copy the message interprocess here
				DMSG("Writing message on shared memory \"%s\"\n", msg);
				strncpy(&(this->messageStore->msg[0]), msg, maxMsgSize);
				if(len >= maxMsgSize){
					this->messageStore->msg[maxMsgSize-1] = '\0';
				}
			sem_post(&(this->messageStore->done));
		}else{
			//parent process reads memory
			sem_wait(&(this->messageStore->done));
			if(this->messageStore->msg[0] == '\0'){
				//there is no message so continue
				DMSG("There is not message yet\n");
				rstr = std::string("");
			}else{
				//there is a message so read it
				DMSG("Reading message from shared memory\"%s\"\n", &(this->messageStore->msg[0]));
				rstr = std::string(&(this->messageStore->msg[0]));
			}
			sem_post(&(this->messageStore->done));
		}
		return rstr;
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
				//if the demon has just started and there was no file
				//then call script to recreate the file
				this->callRestoreScript();
				this->readParameters();
				break;
			case 1:
				//if the demon has just started and there is a file
				//then check if the modify time of the file is < 5 min ago
				//then call update script
				DMSG("%f seconds since shop status file was modified\n", this->secondsSinceModified(this->shopStatusFilename));
				if(this->secondsSinceModified(this->shopStatusFilename) < (5*60))
				{
					this->readParameters();
					this->callUpdateScript();
				}
				break;
			case 2: 
				//if the file gets created
				//then call update script
				if(this->ignoreIEventQuantity[2] > 0){
					DMSG("Ignore I event because another event triggered this\n");
					this->ignoreIEventQuantity[2]--;
					break;
				}
				this->readParameters();
				this->callUpdateScript();
				break;
			case 3: 
				//if the file get deleted
				//ignore a create and modify event
				//then call script to recreate the file
				//then call update script
				if(this->ignoreIEventQuantity[3] > 0){
					DMSG("Ignore I event because another event triggered this\n");
					this->ignoreIEventQuantity[3]--;
					break;
				}
				this->ignoreIEventQuantity[2]++;
				this->ignoreIEventQuantity[4]++;
				this->callRestoreScript();
				this->readParameters();
				this->callUpdateScript();
				break;
			case 4:
				//if the file gets modified
				//then call update script
				if(this->ignoreIEventQuantity[4] > 0){
					DMSG("Ignore I event because another event triggered this\n");
					this->ignoreIEventQuantity[4]--;
					break;
				}
				this->ignoreIEventQuantity[4]++;
				this->readParameters();
				this->callUpdateScript();
				break;
			case 5:
				//if the file gets its time attribute modified
				//then call update script
				//TODO: fix this
				if(this->ignoreIEventQuantity[5] > 0){
					DMSG("Ignore I event because another event triggered this\n");
					this->ignoreIEventQuantity[5]--;
					break;
				}
				if(this->secondsSinceModified(this->shopStatusFilename) < (1)){
					this->readParameters();
					this->callUpdateScript();
				}
				break;
			default:
				DMSG("Event did not correspond to any type possibility\n");
				exit(EXIT_FAILURE);
		}
		DMSG("Checking whether to terminate\n");
		if(*this->terminate){
			DMSG("terminate\n");
			exit(0);
		}
		DMSG("Termination not signaled\n");
	}
	void Beacon::readParameters()
	{
		std::ifstream ifs;
		std::string url;
		char c;
		//Read shop status file in
		ifs.open (this->shopStatusFilename.c_str(), std::ifstream::in);
		if(ifs.fail()){
			DMSG("Could not open file \"%s\"\n", this->shopStatusFilename.c_str());
			//TODO clean parameters map
			return;
		}
		while(ifs.get(c)){
			url += c;
		}
		DMSG("Contents of file are \"%s\"\n", url.c_str());
		ifs.close();
		//Sanitize string by removing \n, \tabs, an the changing %[A-Z0-9]{2} to their respective characters
		//Actually don't remove \n anymore just do the changing
		//Decode URL
		std::string decodedURL = this->decode(url);
		DMSG("Decoded URL is \"%s\"\n", decodedURL.c_str());
		//Split and map variables
		DMSG("Spliting URL\n");
		std::string del = "&";
		std::vector<std::string> params = this->split(decodedURL, del);
		//Process each parameter
		del = "=";
		//Erase all parameters
		this->parameters.erase(this->parameters.begin(), this->parameters.end());	
		for(auto it = params.begin(); it != params.end(); ++it){
			std::vector<std::string> kv = this->split(*it, del);
			std::string key;
			std::string value;
			if(kv.size() == 2){
				key = kv[0];
				value = kv[1];
			}else if(kv.size() == 1){
				key = kv[0];
				value = "";
			}else{
				DMSG("Parameters has been wrongly specified exiting (%lu)", kv.size());
				exit(EXIT_FAILURE);
			}
			value = this->processReservedCharacters(value);
			this->parameters[key] = value;
			DMSG("Parameters %s = \"%s\" (%ld) \n", key.c_str(), value.c_str(), parameters.size());
		}
		//write the message for other process to get
		this->getMessage();
	}
	std::vector<std::string> Beacon::split(std::string& str, std::string& del)
	{
		size_t pos = 0;
		std::vector<std::string> strings;
		while(true){
			size_t first = str.find(del, pos);
			if(first==std::string::npos){
				std::string token = str.substr(pos);
				if (token.compare("")){
					//DMSG("Adding token \"%s\"\n", token.c_str());
					strings.push_back(token);
				}
				break;
			}
			//found something so add that to vector
			std::string token = str.substr(pos, first-pos);
			//DMSG("Adding token \"%s\"\n", token.c_str());
			strings.push_back(token);
			pos = first + del.length();
		}
		return strings;
	}
	std::string Beacon::decode(std::string& str)
	{
		std::string dstr = "";
		std::string::iterator it = str.begin();
		std::string::iterator eit = str.end();
		std::string::iterator it1;
		std::string::iterator it2;
		DMSG("Begin decoding URL string \"%s\"", str.c_str());
		while(it != eit){
			int cdec = this->getPercentEncoded(it, eit);
			if(cdec == -1){
				//check if any of the characters spec
				//DMSG("got character \"%c\"\n", (char)*it);
				dstr += *it;
				it++;
			}else{
				//convert and advance iterator 3 times
				//DMSG("got string \"%c\"\n", (char)cdec);
				dstr += cdec;
				it = std::next(it, 3);;
			}
		}
		return dstr;
	}
	void Beacon::callUpdateScript()
	{
		//call update script
		pid_t pid;
		pid = fork();
		if(pid == 0){
			DMSG("(child) %s [%s]\n", this->updateScriptFilename.c_str(), "");
			execl(this->updateScriptFilename.c_str(), "update.sh", this->shopStatusFilename.c_str(), NULL);
			exit(EXIT_FAILURE);
		}else if(pid < 0){
			DMSG("(parent) child not created... unable to call script \"%s\"", this->shopStatusFilename.c_str());
		}else{
			int status;
			DMSG("(parent) waiting...\n");
			if(waitpid (pid, &status, 0) != pid){
				DMSG("(parent) something's wrong...\n");
			}else{
				DMSG("(parent) child arrived!\n");
			}
		}
	}
	void Beacon::callRestoreScript()
	{
		pid_t pid;
		pid = fork ();
		if(pid == 0){
			DMSG("(child) %s\n", this->restoreScriptFilename.c_str());
			execl(this->restoreScriptFilename.c_str(), "restore.sh", this->shopStatusFilename.c_str(), NULL);
			exit(EXIT_FAILURE);
		}else if(pid < 0){
			DMSG("(parent) child not created... unable to call script \"%s\"", this->restoreScriptFilename.c_str());
		}else{
			int status;
			DMSG("(parent) waiting...\n");
			if(waitpid (pid, &status, 0) != pid){
				DMSG("(parent) something's wrong...\n");
			}else{
				DMSG("(parent) child arrived!\n");
			}
		}
	}
}
