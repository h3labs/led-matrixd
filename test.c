#include "ini_config.h"

void read_ini_file(const char* file){
        struct collection_item* ini_config = NULL; 
        struct collection_item* errors = NULL; 
        //read ini file with all the configuration options 
        int res = config_from_file("led-matrixd", file, &ini_config, INI_STOP_ON_ERROR, &errors);
        if(res != 0){
      //          perror("read_ini_file()");
       //         exit(EXIT_FAILURE);
        }       
        int num_sections = 0;
        char** section_list = get_section_list((struct collection_item*)ini_config, &num_sections, NULL);        
        if(section_list != NULL){
//                perror("ini sections");
 //               exit(EXIT_FAILURE);
        }       
        int i;  
        for(i = 0; i < num_sections; i++){
               printf("section_name: %s\n", section_list[i]);
        }       
}

int main(int argc, char* argv[]) 
{
        //ledMatrixD::OpenCloseSign* openClose  = new ledMatrixD::OpenCloseSign();
        //openClose->run();
        read_ini_file("matrix.ini");
        return 0;
}

