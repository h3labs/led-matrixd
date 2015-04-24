#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
extern "C" {
	#include "ini_config.h"
}

#ifndef __INI_READER_H__
#define __INI_READER_H__

namespace ini {
  const char* ini_file = std::string("matrix.ini").c_str();
  struct ::collection_item* ini_config = NULL;
  struct ::collection_item* errors = NULL;
  struct ::collection_item* get_item(const char* section, const char* attribute){
    struct ::collection_item* item = NULL;
    int ares = get_config_item(section, attribute, ini_config, &item);
    if(ares != 0){
      return NULL;
    }
    return item;
  }

  //TODO: clean all uses of this or FREE
  int get_string(const char* section, const char* attribute, char** value){
    int error;
    struct ::collection_item* item = get_item(section, attribute);
    *value = get_string_config_value(item, &error);
    return error;
  }

  int get_int(const char* section, const char* attribute, int* value){
    int error;
    struct ::collection_item* item = get_item(section, attribute);
    *value = get_int32_config_value(item, 0, 0, &error);
    return error;
  }

  bool has_section(char* section){
	int num_sections = 0;
	char** section_list = ::get_section_list(ini_config, &num_sections, NULL);

	if(section_list == NULL){
      return false;
	}

    int i;

    for (i = 0; i < num_sections; i++){
      if (strcmp(section, section_list[i]) == 0){
        return true;
      }
    }

    return false;
  }

  void print_ini_variables(){
    int num_sections = 0;
    char** section_list = ::get_section_list(ini_config, &num_sections, NULL);
    if(section_list == NULL){
      perror("ini sections");
      exit(EXIT_FAILURE);
    }
    int i;
    for(i = 0; i < num_sections; i++){
#ifdef DEBUG
      printf("section_name: %s\n", section_list[i]);
#endif
      int num_attr = 0;
      char** attr_list = get_attribute_list(ini_config, section_list[i], &num_attr, NULL);
      int j;
      for(j = 0; j < num_attr; j++){
        struct ::collection_item* item;
        int ares = get_config_item(section_list[i], attr_list[j], ini_config, &item);
        if(ares != 0){
          perror("get_config_item()");
          exit(EXIT_FAILURE);
        }
        char* item_string = get_string_config_value(item, NULL);
        if(item_string == NULL){
          perror("get_string_config_value()");
          exit(EXIT_FAILURE);
        }
        printf("\t[%s] %s\n", attr_list[j], item_string);
      }
    }
  }

  void read_file(const char* file){
    //read ini file with all the configuration options
    int res = ::config_from_file("led-matrixd", file, &ini_config, INI_STOP_ON_ERROR, &errors);
    if(res != 0){
      perror("read_ini_file()");
      exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    printf("Matrix Led Deamon Configured with:\n");
    print_ini_variables();
#endif
  }
}
#endif
