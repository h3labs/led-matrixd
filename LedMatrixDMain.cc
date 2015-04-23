#include "Beacon.h"
#include "INIReader.h"
#include "LedMatrixDMain.h"
#include "demo-canvas.h"
#include "DisplaysSequence.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <utility>

namespace ledMatrixD
{
	int pwm_bits = -1;
	bool large_display = false;
	int demo = -1;
	int rows = 32;
	int chain = 1;
	int scroll_ms = 30;
	bool do_luminance_correct = true;
	rgb_matrix::GPIO io;
}


using namespace std;

int sequence = 0;
void usage(const char* msg){
	printf( "led-matrixd: %s\n"
			"usage: led-matrixd -C [ini file]\n"
			"   C: file name (directory) where the ini configuration file\n"
			"      resides.\n"
			"\n", msg);
}

int main(int argc, char* argv[])
{
	//TODO: getopt
	int opt;
	while ((opt = getopt(argc, argv, "C:")) != -1) {
		switch (opt) {
			case 'C':
				ini::ini_file = optarg;
				break;
				/*
				   case 'D':
				   std::cout << "this happened D\n";
				   sequence = atoi(optarg);
				   std::cout << "this happened D\n";
				   if( !(0 <= sequence && sequence <= 1)){
				   std::cout << "this happened N\n";
				   usage("illegal value -D can only use 0 or 1");
				   exit(EXIT_FAILURE);
				   }
				   break;
				   */
			case '?':
			default:
				usage("");
				exit(EXIT_FAILURE);
		}
	}
	DMSG("Reading INI configuration file\n");
	ini::read_file(ini::ini_file);
	DMSG("Succesfully read INI file\n");
	//  ledMatrixD::OpenCloseSign* openClose = NULL;
	ledMatrixD::Beacon* beacon = NULL;
	//  switch(sequence){
	//    case 0:
	//      openClose = new ledMatrixD::OpenCloseSign();
	//      openClose->run();
	//      break;
	//    case 1:
	beacon = new ledMatrixD::Beacon();
	beacon->run();
	/*
	while(true){
		sleep(3);
		DMSG("waiting...\n");
	}
	*/
	ledMatrixD::runLongSequence();
	beacon->stop();
	//      break;
	//  }
	return 0;
}

