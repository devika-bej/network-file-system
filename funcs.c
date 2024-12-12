#include "defs.h"
#include "headers.h"

void print_errors(int error_code){
    if(error_code == INVALID_COMMAND){
        printf("Command is invalid.\n");
    }else if(error_code == NOT_FOUND){
        // print directory not found
        printf("File not found.\n");
    }else if(error_code == TASK_DONE){
        printf("Task completed.\n");
    }else if(error_code == DATA_END){
        printf("Data transfer complete.\n");
    }else if(error_code == NOT_ACCESSIBLE){
        // print file not accessible
        printf("File not accessible.\n");
    }else if(error_code == CONNECT_CLT_TO_NM){
        printf("Client connected to naming server.\n");
    }else if(error_code == SUCCESSFUL){
        printf("Task completed successfully.\n");
    }else if(error_code == UNSUCCESSFUL){
        printf("Task could not be completed.\n");
    }
}