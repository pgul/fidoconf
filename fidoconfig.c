#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fidoconfig.h"
#include "typesize.h"

char *readLine(FILE *f)
{
   char *line = NULL, temp[81];
   size_t size = 81;

   line = (char *) malloc(size);
   if (fgets(line, 81, f) == NULL) {
      free(line);                      // end of file...
      return NULL;
   }

   while ((strlen(line) % 80) == 0) {
      if (fgets(temp, 81, f) == NULL) break; // eof encountered
      if (temp[strlen(temp)-1] == '\n') {
         temp[strlen(temp)-1] = 0; // kill \n
      }
      realloc(line, strlen(line)+strlen(temp)+1);
      strcat(line, temp);
   }

   if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = 0;
   return line;
}

char *trimLine(char *line)
{
   char *start = line, *temp;

   while ((*start == ' ') || (*start == '\t') || (*start == '\xfe')) start++;
   temp = (char *) malloc(strlen(start)+1);
   strcpy(temp, start);
   free(line);
   return temp;
}

void parseConfig(FILE *f, s_fidoconfig *config)
{
   char *line;
   
   while ((line = readLine(f))) {
      line = trimLine(line);
      if ((line[0] != '#') && (line[0] != 0))
         parseLine(line, config);
   }
}

void initConfig(s_fidoconfig *config) {
   // set all to 0
   memset(config, sizeof(s_fidoconfig), 0);
}

s_fidoconfig *readConfig()
{
   FILE *f;
   s_fidoconfig *config;

   f = fopen("/etc/fido/config", "r");
   if (f != NULL) {
      config = (s_fidoconfig *) malloc(sizeof(s_fidoconfig));
      initConfig(config);
      parseConfig(f, config);
      return config;
   } else {
      printf("Could not find config-file!\n");
      return NULL;
   }
}

void disposeConfig(s_fidoconfig *config)
{
   free(config->name);
   free(config->sysop);
   free(config->location);
   free(config);
   config = NULL;
}