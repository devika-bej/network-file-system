#include "tries.h"
#include "LRU.h"
#include <time.h>

#define MAX_BUFFER_SIZE 1024
// #define FILE_LRU "LRU.txt"

time_t currentTime;
struct tm *localTime;

void combine_for_LRU(char *command, int port_nm, int port_clt, char *cwd)
{
    char nm[1024];
    char clt[20048];
    sprintf(nm, "%d", port_nm);
    sprintf(clt, "%d", port_clt);
    char *temp = (char *)malloc(sizeof(char) * 1024);
    strcpy(temp, command);
    strcat(temp, ",");
    strcat(temp, nm);
    strcat(temp, ",");
    strcat(temp, clt);
    strcat(temp, ",");
    strcat(temp, cwd);

    append_data(temp, "LRU.txt");
}

void combine_with_comma(char *port_of_ack_sender, char *command, char *ack)
{
    time(&currentTime);
    localTime = localtime(&currentTime);

    char hour[5], min[6], sec[7];
    sprintf(hour, "%d", localTime->tm_hour);
    sprintf(min, "%d", localTime->tm_min);
    sprintf(sec, "%d", localTime->tm_sec);

    char *temp = (char *)malloc(sizeof(char) * 2048);
    strcpy(temp, hour);
    strcat(temp, ":");
    strcat(temp, min);
    strcat(temp, ":");
    strcat(temp, sec);
    strcat(temp, ",");

    strcat(temp, port_of_ack_sender);
    strcat(temp, ",");
    strcat(temp, command);
    strcat(temp, ",");
    strcat(temp, ack);

    printf("temp: %s\n", temp);
    append_to_file(temp, "bookkeeping.txt");
}

void append_to_file(char data[1024], char *filename)
{
    FILE *fp = fopen(filename, "a");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    fprintf(fp, "%s\n", data);

    fclose(fp);
}

void append_data(char data[1024], char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    char buffer[MAX_BUFFER_SIZE];
    int cnt = 0;
    int flag = -1;
    while (fgets(buffer, MAX_BUFFER_SIZE, fp) != NULL)
    {
        buffer[strlen(buffer) - 1] = '\0';
        if (strcmp(buffer, data) == 0)
        {
            flag = cnt;
        }
        cnt++;
    }
    fclose(fp);

    if (flag != -1)
    {
        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            perror("Error opening file");
            exit(1);
        }
        char **lines = (char **)malloc(sizeof(char *) * cnt);
        int i = 0;
        while (fgets(buffer, MAX_BUFFER_SIZE, fp) != NULL)
        {
            lines[i] = (char *)malloc(sizeof(char) * strlen(buffer));
            strcpy(lines[i], buffer);
            i++;
        }
        fclose(fp);
        fp = fopen(filename, "w");
        if (fp == NULL)
        {
            perror("Error opening file");
            exit(1);
        }
        for (int j = 0; j < cnt; j++)
        {
            if (j == flag)
            {
                continue;
            }
            fprintf(fp, "%s", lines[j]);
        }
        fclose(fp);
        append_to_file(data, filename);
        return;
    }

    if (cnt < 15)
    {
        append_to_file(data, filename);
        return;
    }

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    char **lines = (char **)malloc(sizeof(char *) * cnt);
    int i = 0;
    while (fgets(buffer, MAX_BUFFER_SIZE, fp) != NULL)
    {
        lines[i] = (char *)malloc(sizeof(char) * strlen(buffer));
        strcpy(lines[i], buffer);
        i++;
    }
    fclose(fp);
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    for (int j = 1; j < cnt; j++)
    {
        fprintf(fp, "%s", lines[j]);
    }
    fclose(fp);
    append_to_file(data, filename);
}

void extract_tokens(char *line, char *tokens[4])
{
    char *copy = strdup(line); // Make a copy of the line
    char *token = strtok(copy, ",");
    int i = 0;

    while (token != NULL && i < 4)
    {

        tokens[i] = strdup(token); // Make a copy of each token
        i++;
        token = strtok(NULL, ",");
    }

    free(copy); // Free the allocated copy
}

char **search_and_extract(char *searchToken)
{

    FILE *file = fopen("LRU.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    char line[1024];
    char **tokens;

    // MALLOC
    tokens = (char **)malloc(sizeof(char *) * 1024);
    if (tokens == NULL)
    {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }

    // MALLOC for individual tokens
    for (int i = 0; i < 4; i++)
    {
        tokens[i] = NULL;
    }

    char copy[1024]; // Allocate space for the copy

    for (int i = 0; i < 15 && fgets(line, sizeof(line), file) != NULL; i++)
    {
        strcpy(copy, line);
        char *token = strtok(copy, ",");
        if (token != NULL && strcmp(token, searchToken) == 0)
        {
            extract_tokens(line, tokens);
            printf("Match found on line %d:\n", i + 1);
            printf("Token 1: %s\n", tokens[0]);
            printf("Token 2: %s\n", tokens[1]);
            printf("Token 3: %s\n", tokens[2]);
            printf("Token 4: %s\n", tokens[3]);

            fclose(file);

            // Free allocated memory before returning
            for (int i = 0; i < 4; i++)
            {
                free(tokens[i]);
            }
            free(tokens);

            return NULL; // Modify as needed based on your requirements
        }
    }

    printf("Match not found in the first 15 lines.\n");

    fclose(file);

    for (int i = 0; i < 4; i++)
    {
        free(tokens[i]);
    }
    free(tokens);

    return NULL;
}