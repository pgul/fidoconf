/******************************************************************************
 * FIDOCONFIG --- library for fidonet configs
 ******************************************************************************
 * Copyright (C) 1998-1999
 *  
 * Matthias Tichy
 *
 * Fido:     2:2433/1245 2:2433/1247 2:2432/605.14
 * Internet: mtt@tichy.de
 *
 * Grimmestr. 12         Buchholzer Weg 4
 * 33098 Paderborn       40472 Duesseldorf
 * Germany               Germany
 *
 * This file is part of FIDOCONFIG.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; see file COPYING. If not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#if !defined(MSDOS) || defined(__DJGPP__)
#include "fidoconfig.h"
#else
#include "fidoconf.h"
#endif
#include "common.h"

int writeArea(FILE *f, s_area *area, char netMail) {
   switch (area->msgbType) {
      
      case (MSGTYPE_SQUISH): fprintf(f, "Squish ");
                             break;
      
      case (MSGTYPE_SDM):    fprintf(f, "Fido ");
                             break;

      case (MSGTYPE_JAM):    fprintf(f, "Jam ");
                             break;
   }

   if (netMail == 1) fprintf(f, "mp");
   else fprintf(f, "e");
   fprintf(f, "8u ");

   fprintf(f, "\"%s\" %s %s ", area->areaName, area->fileName, (netMail!=1) ? area->areaName : "");

   fprintf(f, "%u:%u/%u.%u", area->useAka->zone, area->useAka->net, area->useAka->node, area->useAka->point);
   
   fprintf(f, "\n");

   return 0;
}

int generateMsgEdConfig(s_fidoconfig *config, char *fileName) {
   FILE *f;
   int  i;
   s_area *area;

   f = fopen(fileName, "w");
   if (f!= NULL) {

      fprintf(f, "Name \"%s\"\n\n", config->sysop);
      
      for (i=0; i<config->addrCount; i++)
         fprintf(f, "Address %u:%u/%u.%u\n", config->addr[i].zone, config->addr[i].net, config->addr[i].node, config->addr[i].point);
      
      if (config->echotosslog != NULL) fprintf(f, "tossLog %s\n", config->echotosslog);
      if (config->nodelistDir != NULL && config->fidoUserList != NULL)
      {
          fprintf(f, "Userlist %s%s\n",
                  config->nodelistDir, config->fidoUserList);
      }

      fprintf(f, "\n");

      for (i=0; i<config->netMailAreaCount; i++) {
         writeArea(f, &(config->netMailAreas[i]), 1);
      }

      writeArea(f, &(config->dupeArea), 0);
      writeArea(f, &(config->badArea), 0);

      for (i=0; i<config->localAreaCount; i++) {
         area = &(config->localAreas[i]);
         if (area->msgbType != MSGTYPE_PASSTHROUGH)
             writeArea(f, area, 0);
      }
      
      for (i=0; i<config->echoAreaCount; i++) {
         area = &(config->echoAreas[i]);
         if (area->msgbType != MSGTYPE_PASSTHROUGH)
             writeArea(f, area, 0);
      }
      
      return 0;
   } else printf("Could not write %s\n", fileName);

   return 1;
}

int main (int argc, char *argv[]) {
   s_fidoconfig *config;

   printf("fconf2msged\n");
   printf("-----------\n");
   if (argc < 2) {
      printf("\nUsage:\n");
      printf("   fconf2msged <msgedConfigFileName>\n");
      printf("\nExample:\n");
      printf("   fconf2msged ~/.msged\n\n");
      return 1;
   }

   printf("Generating Config-file %s\n", argv[1]);

   config = readConfig();
   if (config!= NULL) {
      generateMsgEdConfig(config, argv[1]);
      disposeConfig(config);
      return 0;
   }

   return 1;
}
