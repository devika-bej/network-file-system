#include "headers.h"
#include "defs.h"

#ifndef LRU_H
#define LRU_H
// void append_to_file(char data[1024], char *filename);

void combine_with_comma(char *port_of_ack_sender, char *command, char *ack);
char **search_and_extract(char *searchToken);
void append_to_file(char data[1024], char *filename);

void combine_for_LRU(char *command, int port_nm, int port_clt, char *cwd);

#endif