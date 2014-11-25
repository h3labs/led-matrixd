#ifndef __LED_MATRIX_D_INI_READER_H__
#define __LED_MATRIX_D_INI_READER_H__
#include <stdio.h>
#include <stdlib.h>
extern "C" {
	#include "ini_config.h"
}

namespace ini { 
  extern char* ini_file;
  extern struct ::collection_item* ini_config;
  extern struct ::collection_item* errors;	
  extern struct ::collection_item* get_item(const char* section, const char* attribute);
  extern int get_string(const char* section, const char* attribute, char** value);
  extern int get_int(const char* section, const char* attribute, int* value);
  extern void print_ini_variables();
	void read_file(const char* file);
}
#endif
