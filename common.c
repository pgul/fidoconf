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
 *****************************************************************************
 * $Id$
 */

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#if ((!(defined(_MSC_VER) && (_MSC_VER >= 1200))) && (!defined(__TURBOC__)))
#  include <unistd.h>
#endif

#if ((defined(_MSC_VER) && (_MSC_VER >= 1200)) || defined(__TURBOC__) || defined(__DJGPP__)) || defined(__MINGW32__)
#  include <io.h>
#endif

#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#if !(defined(USE_SYSTEM_COPY) && (defined(__NT__) || defined(OS2)))
#ifdef __MINGW32__
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#endif

#if defined ( __WATCOMC__ )
#include <dos.h>
#endif

#ifdef UNIX
#include <pwd.h>
#endif

#ifdef __BEOS__
#include <KernelKit.h>
#endif

#ifdef _MAKE_DLL_MVC_
//#	include <Windows.h>
#	include <Winbase.h>
#	undef s_addr
#endif

#include "fidoconf.h"
#include "common.h"
#include "xstr.h"
#include "log.h"
#include <smapi/compiler.h>
#include <smapi/patmat.h>
#include <smapi/progprot.h>


int copyString(char *str, char **pmem)
{
   if (str==NULL) {
      printf("Line %d: There is a parameter missing after %s!\n", actualLineNr, actualKeyword);
      return 1;
   }

   nfree(*pmem);
   *pmem = (char *) sstrdup (str);

   return 0;
}

int copyStringUntilSep(char *str, char *seps, char **dest)
{
  char *sepPos;

  if ((!str) || (!(*str)))
  {
    printf("Line %d: There is a parameter missing after %s!\n", actualLineNr, actualKeyword);
    return 0;
  }

  nfree(*dest);
  sepPos = strpbrk(str, seps);
  if (sepPos)
  {
    *dest = malloc(sepPos - str + 1);
    strncpy(*dest, str, sepPos - str);
    (*dest)[sepPos - str] = 0;

    return (sepPos - str);
  }

  *dest = (char *) sstrdup(str);
  return strlen(str);
}

void *memdup(void *p, size_t size)
{
	void *newp;
	newp = smalloc(size);
	memcpy(newp, p, size);
	return newp;
}

static char *attrStr[] = { "pvt", "crash", "read", "sent", "att", 
                       "fwd", "orphan", "k/s", "loc", "hld", 
                       "xx2",  "frq", "rrq", "cpt", "arq", "urq" };
                                   
long  str2attr(const char *str)
{
   int i;
   for (i = 0; i < sizeof(attrStr) / sizeof(char *); i++) 
           if (strncasecmp(str, attrStr[i], strlen(attrStr[i]))==0) 
                   return 1 << i;
   return -1L;
}

int  addrComp(const s_addr a1, const s_addr a2)
{
   int rc = 0;

   rc =  a1.zone  != a2.zone;
   rc += a1.net   != a2.net;
   rc += a1.node  != a2.node;
   rc += a1.point != a2.point;

   return rc;
}

char *strrstr(const char *HAYSTACK, const char *NEEDLE)
{
   char *start = NULL, *temp = NULL;

   temp = strstr(HAYSTACK, NEEDLE);
   while (temp  != NULL) {
      start = temp;
      temp = strstr(temp+1,NEEDLE);
   }
   return start;
}

void string2addr(char *string, s_addr *addr) {
	char *endptr, *str = string;
	unsigned long t;

	memset(addr, '\0', sizeof(s_addr));

	if (str == NULL) return;
	if (strchr(str,':')==NULL || strchr(str,'/')==NULL) return;

	// zone
	if (NULL == strstr(str,":")) return;
	t = strtoul(str,&endptr,10);
	addr->zone = (UINT16) t;
	if(!addr->zone) return; // there is no zero zones in practice

	// net
	str = endptr+1;
	if (NULL == strstr(str,"/")) return;
	t = strtoul(str,&endptr,10);
	addr->net = (UINT16) t;

	// node
	str = endptr+1;
	t = strtoul(str,&endptr,10);
	addr->node = (UINT16) t;

	// point
	if (*endptr && !isspace( endptr[0] )) str = endptr+1; 
	else return; // end of string
	t = strtoul(str,&endptr,10);
	addr->point = (UINT16) t;
	
	return;
}

#ifdef __TURBOC__
#pragma warn -sig
#endif

UINT16 getUINT16(FILE *in)
{
   UCHAR dummy;

   dummy = (UCHAR) getc(in);
   return (dummy + (UCHAR) getc(in) * 256);
}

int fputUINT16(FILE *out, UINT16 word)
{
  UCHAR dummy;

  dummy = word % 256;        // write high Byte
  fputc(dummy, out);
  dummy = word / 256;        // write low Byte
  return fputc(dummy, out);
}

#ifdef __TURBOC__
#pragma warn +sig
#endif


INT   fgetsUntil0(UCHAR *str, size_t n, FILE *f, char *filter)
{
   size_t i;

   for (i=0;i<n-1 ;i++ ) {
	   
	  do {
		  str[i] = (UCHAR)getc(f);
	  } while (filter && *filter && str[i] && strchr(filter, str[i]) != NULL);

      // if end of file
      if (feof(f)) {
         str[i] = 0;
         return i+1;
      } /* endif */

      if (0 == str[i]) {
         return i+1;
      } /* endif */

   } /* endfor */

   str[n-1] = 0;
   return n;
}

char *stripLeadingChars(char *str, const char *chr)
{
   char *i = str;

   if (str != NULL) {
   
      while (NULL != strchr(chr, *i)) {       // *i is in chr
         i++;
      } /* endwhile */                        // i points to the first occurences
                                              // of a character not in chr
      strcpy(str, i);
   }
   return str;
}

char *strUpper(char *str)
{
   char *temp = str;
   
   while(*str != 0) {
      *str = (char)toupper(*str);
      str++;
   }
   return temp;
}

char *strLower(char *str)
{
   char *temp = str;
   
   while(*str != 0) {
      *str = (char)tolower(*str);
      str++;
   }
   return temp;
}

char *shell_expand(char *str)
{
    char *slash = NULL, *ret = NULL, c;
#ifdef UNIX
    struct passwd *pw = NULL;
#endif
    char *pfix = NULL;

    if (str == NULL)
    {
        return str;
    }
    if (*str == '\0' || str[0] != '~')
    {
        return str;
    }
    for (slash = str; *slash != '/' && *slash != '\0'
#ifndef UNIX
                     && *slash != '\\'
#endif
         ; slash++);
    c = *slash;
    *slash = 0;

    if (str[1] == '\0')
    {
        pfix = getenv("HOME");
#ifdef UNIX        
        if (pfix == NULL)
        {
            pw = getpwuid(getuid());
            if (pw != NULL)
            {
                pfix = pw->pw_dir;
            }
        }
#endif
    }
#ifdef UNIX
    else
    {
        pw = getpwnam(str + 1);
        if (pw != NULL)
        {
            pfix = pw->pw_dir;
        }
    }
#endif
    *slash = c;

    if (pfix == NULL)  /* could not find an expansion */
    {
        return str;
    }

    ret = smalloc(strlen(slash) + strlen(pfix) + 1);
    strcpy(ret, pfix);
    strcat(ret, slash);
    nfree(str);
    return ret;
}

/* ================================================================ 

Function: makeUniqueDosFileName 

OVERVIEW:

The following function is used to create "unique" 8.3 filenames.  This is
a major concerning when creating fidonet PKT files.  If you use this
function to create PKT filenames, and your node only runs programs that use
the same revision of the fidoconfig library, it will be guranteed that your
program will emit unique PKT filenames throughout the whole epoch (!), and
in almost all cases you can also be sure that your packets will not have
name clashes with packets from other nodes that run fidoconfig programs
during a period of about two days.  (Normally, the tosser of your uplink
should not have a problem with clashes of pkt filenames from different links
anyway, but just in case he uses a brain-dead tosser ...).

CALLING:

The function takes a directory name as argument (which is prepended to the
generated file name, but has no further meaning), an extension (which again
is appended to the gernated file name, but has no furhter meaning), and a
fidoconfig structure.  The fidoconfig sturcture is used to create a basic
offset number that distinguishes your node/point from another node/point.

DETAILS:

The function guarantees to create
  - 36 filenames per second (if the average number of filenames created
    per second over the whole lifetime of the program is greater, the
    program will be delayed for the appropriate time via an atexit
    handler). That is, you can poll as many filenames out of the function
    as you wish, and you will get them quickly when you poll them, the
    worst case is that if your program ran a very short time it will be
    delayed before finally exiting.
  - If the fidoconfig file always has the same primary AKA, and no two
    programs that use that use the function run at the same time on your
    system, it will emit unique filenames during the whole epoch (!).
    That is, a Fidonet editor should NOT use this function (because a
    editor usually runs parallel to the tossertask), while the tosser,
    router, ticker, ... may safely use it as those programs usually run
    serialized.
  - If the primary AKA in the fidoconfig changes, the file names will
    change substantially, so that different nodes who both run hpt
    or other fidoconfig programs will generate substantially different file
    names.
  - ATTENTION: This function presently does not check if the file
    names that it calculates do already exist. If you are afraid of
    this (because you also use non-fidoconfig programs), you must
    check it yourself in the application.

IMPLEMENTATION NOTES:

The alogrithm for creating pkt file names works as follows:

 - Step 1:  Based on OUR AKA, a static offset number is computed.  This is
   done so that two systems which both run hpt create somewhat different
   PKT file name. The offset number is computed as follows:

      We imagine the node numbe decomposed in its digits:
         node  = node1000 * 1000 + node100 * 100 + node10 *10 + node1
      analoguous for the net number:
         net   = net1000 * 1000 + net100 * 100 + net10 * 10 + net1
      and the point number:
         point = point1000 * 1000 + point100 * 100 + point10 * 10 + point1

      Then the decimal digits of "offset" are composed as follows:

      8         7         6        5        4        3        2        1
  (I) node10    node1     net10    net1     node100  node1000 net100   net1000
 (II) node10    node1     point10  point1   node100  node1000 net100   net1000

      where line (I) is used if point!=0, and line (II) is used if point==0.

      Then the offset number is multiplied by 21 * 36. (This will overflow
      a 32 bit unsigned integer, so in the code we only multiply by 21 and
      then do other tricks).

 - Step 2: If this is the first start, the value of time() is obtained and
      increased by one. That value is the "base packet number" for the file
      name about to be created.

 - Step 3: The base packet number is added to the offset number and printed
      as a seven digit number in a 36-based number system using the digits
      0..9 and a..z. (We need all alphanumeric characters to assure all
      uniquenesses that we guranteed ... hexadezimal characters are not
      enough for this).

 - Step 4: The last (eigth') character in the generated file name is
      a counter to allow for creating more than one filename per second.
      The initial value of the counter is:

            (net10 * 10 + net1 + point100) modulo 36

 - Step 5: The counter is printed as the eight character using characters
      0..9, and a..z (for counter values from 10 to 35).

 - Step 6: On subsequent calls of this routine, the counter value is
      increased by one. If it becomes greater than 35 it is wrapped to zero.
      If all counter values have been used up (i.E. after the increasement
      and possibly wrapping the counter value is again the initial value),
      the base packet number is increased by one.

  - Step 7: At program exit, the program sleeps until the value of time()
      is greater or at least equal to the highest base packet number that
      has been used.
      (This is done via atexit. If the registering of the atexit program
      fails, the algorithm above is modified so that every time that
      the base packet number is increased, the program immediately waits
      until time() is equal the base packet number. This is slower, but
      it is a secure fallback in case atexit fails).

The result is:

  - The routine is able to create 36 filenames per second. If more filenames
    are requested within a single second, calling the routine might delay
    program execution, but the routine will still produce as many file names
    as you request.

  - As long as the AKA that is based for calculating the offset does not
    change, and of course as long as the system clock is running continously
    without being turned backwards, the routine will create unique filenames
    during the whole epoch!

  - For different nodes that have different AKAs, there will usually be a
    considerable distance between the filenames created by the one node and
    those created by another node. Especially, different points of the same
    node will usually have different file names, and different nodes of the
    same hubs will usually have very different file names over a period
    of at least one day, and usually much more. There is no exact guarantee
    that always two different nodes create different file names within this
    period, but the chances are high. (Note that any decent tosser should
    not have any problem with receving arcmail bundles from two different
    nodes that contain pkt files with the same name; however, Fastecho
    and probably others do have this problem unless the sysop installs
    special scripts to circument it, and this is why we do the whole shit
    of sender specific offset numbers ...)

  - Remark: This code requires sizeof(unsinged long) >= 4. This is true
    for all known 16, 32 and 64 bit architectures.

   ================================================================ */

static time_t last_reftime_used;
static int may_run_ahead;

static void atexit_wait_handler_function(void)
{
    time_t t;

    time(&t);
    while (t < last_reftime_used)
    {
#ifdef __BEOS__
        snooze(10);
#elif defined(UNIX)
        usleep(10);
#else
        sleep(1);
#endif
        time (&t);
    }
}
    
char *makeUniqueDosFileName(const char *dir, const char *ext,
			    s_fidoconfig *config)
{
   char                *fileName;
   
   static unsigned      counter  = 0x100, refcounter = 0x100;
   static time_t        refTime  = 0x0;
   static short         reftime36[7];
   static volatile int  flag = 0;

   unsigned             icounter;
   time_t               tmpt;
   static char          digits[37]="0123456789abcdefghijklmnopqrstuvwxyz";
   int                  i, digit;
   short                offset36[7];
   unsigned long        node10, node1, digit6, digit5, node100, node1000,
                        net100, net1000, tempoffset, net10, net1, point100;
   size_t               pathLen  = strlen(dir);

   /* make it reentrant */
   while (flag)
   {
#ifdef __BEOS__
       snooze(10);
#elif defined(UNIX) || defined(EMX)   
       usleep(10);       /* wait to get a fresh number */
#else
       sleep(1);
#endif
   }

   flag = 1;

   if ((fileName = malloc(pathLen + 1 + 8 + 1 + strlen(ext) + 1)) == NULL)
   {                            /* delim file . ext null */
       flag = 0;
       return NULL;
   }
                           
   memcpy(fileName, dir, pathLen + 1);

   if (pathLen && fileName[pathLen - 1] != '\\' &&
                  fileName[pathLen - 1] != '/' &&
                  fileName[pathLen - 1] != PATH_DELIM)
   {
       fileName[pathLen + 1] = '\0';
       fileName[pathLen] = PATH_DELIM;
       pathLen++;
   }

   if (refTime == 0x0)
   {
       time(&refTime);
       may_run_ahead = !atexit(atexit_wait_handler_function);
       last_reftime_used = refTime;
   }

   /* we make a node specific offset, so that two nodes that both run hpt
      each generate more or less distinct pkt file names */

   node10 = (config->addr[0].node % 100) / 10;
   node1  = (config->addr[0].node % 10);
   if (config->addr[0].point != 0)
   {
       digit6 = (config->addr[0].point % 100) / 10;
       digit5 = config->addr[0].point % 10;
   }
   else
   {
       digit6 = (config->addr[0].net % 100) / 10;
       digit5 = (config->addr[0].net % 10);
   }
   node100  = (config->addr[0].node % 1000) / 100;
   node1000 = (config->addr[0].node % 10000) / 1000;
   net100   = (config->addr[0].net % 1000) / 100;
   net1000  = (config->addr[0].net % 10000) / 1000;
   net10    = (config->addr[0].net % 100) / 10;
   net1     = config->addr[0].net % 10;
   point100 = (config->addr[0].point % 1000) / 100;
 

   tempoffset = (node10   * 10000000UL +
                 node1    * 1000000UL  +
                 digit6   * 100000UL   +
                 digit5   * 10000UL    +
                 node100  * 1000UL     +
                 node1000 * 100UL      +
                 net100   * 10UL       +
                 net1000  * 1UL          ) * 21UL;

   icounter = (unsigned)((net10 * 10U + net1 + point100) % 36U);

   offset36[0] = 0;  /* this is the multiplication by 36! */
   for (i = 1; i <= 6; i++)
   {
       offset36[i] = (short)(tempoffset % 36);
       tempoffset = tempoffset / 36;
   }

   do
   {
       if (counter == icounter || icounter != refcounter)
       {
	   counter = refcounter = icounter;
           last_reftime_used = ++refTime;

           if (!may_run_ahead)
           {
               time (&tmpt);
	   
               while (tmpt < refTime)
               {
#ifdef __BEOS__
                   snooze(50);               
#elif defined(UNIX) || defined(EMX)   
                   usleep(50);       /* wait to get a fresh number */
#else
                   sleep(1);
#endif
                   time(&tmpt);
               }
           }

           tmpt = refTime;
           for (i = 0; i <= 6; i++)
           {
               reftime36[i] = (short)(tmpt % 36);
               tmpt         = tmpt / 36;
           }
       }

       for (i = 0, digit = 0; i < 7; i++)
       {
           digit = digit + reftime36[i] + offset36[i];
           fileName[pathLen + (6 - i)] = digits[digit % 36];
           digit = digit / 36;
       }

       sprintf(fileName + pathLen + 7, "%c.%s", digits[counter], ext);
       counter = ((counter + 1) % 36);

   } while (0); /* too slow because of readdir: fexist(fileName) == TRUE */;

   flag = 0;

   return fileName;
}

#ifdef UNIX
#define MOVE_FILE_BUFFER_SIZE 128000
#else
#define MOVE_FILE_BUFFER_SIZE 16384
#endif

int move_file(const char *from, const char *to)
{
#if !(defined(USE_SYSTEM_COPY) && (defined(__NT__) || defined(OS2)))
    int rc;

    rc = rename(from, to);
    if (!rc) {               /* rename succeeded. fine! */
#elif defined(__NT__) && defined(USE_SYSTEM_COPY)
    int rc;
    rc = MoveFile(from, to);
    if (rc == TRUE) {
#elif defined(OS2) && defined(USE_SYSTEM_COPY)
    USHORT rc;
    rc = DosMove((PSZ)from, (PSZ)to);
    if (!rc) {
#endif
      return 0;
    }

    /* Rename did not succeed, probably because the move is accross
       file system boundaries. We have to copy the file. */

    if (copy_file(from, to)) return -1;
    remove(from);
    return 0;
}

	
int copy_file(const char *from, const char *to)
{
#if !(defined(USE_SYSTEM_COPY) && (defined(__NT__) || defined(OS2)))
    char *buffer;
    size_t read;
    FILE *fin, *fout;
    struct stat st;
    struct utimbuf ut;

    /* Rename did not succeed, probably because the move is accross
       file system boundaries. We have to copy the file. */

    buffer = malloc(MOVE_FILE_BUFFER_SIZE);
    if (buffer == NULL)	return -1;

    fin = fopen(from, "rb");
    if (fin == NULL) { nfree(buffer); return -1; }

    fout = fopen(to, "wb");
    if (fout == NULL) { nfree(buffer); fclose(fin); return -1; }

    while ((read = fread(buffer, 1, MOVE_FILE_BUFFER_SIZE, fin)) > 0)
    {
	if (fwrite(buffer, 1, read, fout) != read)
	{
	    fclose(fout); fclose(fin); remove(to); nfree(buffer);
	    return -1;
	}
    }

    nfree(buffer);
    if (ferror(fout) || ferror(fin))
    {
	fclose(fout);
	fclose(fin);
        remove(to);
	return -1;
    }
    memset(&st, 0, sizeof(st));
    fstat(fileno(fin), &st);
    fclose(fin);
    if (fclose(fout))
    {
	fclose(fout);
	fclose(fin);
        remove(to);
	return -1;
    }
    ut.actime = st.st_atime;
    ut.modtime = st.st_mtime;
    utime(to, &ut);
#elif defined (__NT__) && defined(USE_SYSTEM_COPY)
    int rc = CopyFile(from, to, FALSE);
    if (rc == FALSE) {
      remove(to);
      return -1;
    }       
#elif defined (OS2) && defined(USE_SYSTEM_COPY)
    USHORT rc = DosCopy((PSZ)from, (PSZ)to, 1);
    if (rc) {
      remove(to);
      return -1;
    }       
#endif
    return 0;
}

char *aka2str(s_addr aka) {
  static char straka[SIZE_aka2str];

    if (aka.point) sprintf(straka,"%u:%u/%u.%u",aka.zone,aka.net,aka.node,aka.point);
    else sprintf(straka,"%u:%u/%u",aka.zone,aka.net,aka.node);
	
    return straka;
}

int patimat(char *raw,char *pat)
{
    char *upraw=NULL, *uppat=NULL;
    int i;

    if (raw) upraw=strUpper(sstrdup(raw));
    if (pat) uppat=strUpper(sstrdup(pat));
    i=patmat(upraw,uppat);
    nfree(upraw);
    nfree(uppat);

    return(i);
}

void freeGroups(char **grps, int numGroups)
{
	int i;

	if ( grps == NULL) return;

	for ( i = 0; i < numGroups; i++) {
		nfree (grps[i]);
	}

	nfree (grps);
}

void freeLink (s_link *link)
{

  if (link == NULL) return;

  nfree (link->hisAka.domain);
  if (link->handle != link->name) nfree(link->handle);
  nfree (link->name);
  if (link->pktPwd != link->defaultPwd)nfree(link->pktPwd);
  if (link->ticPwd != link->defaultPwd)nfree(link->ticPwd);
  if (link->areaFixPwd != link->defaultPwd) nfree(link->areaFixPwd);
  if (link->fileFixPwd != link->defaultPwd) nfree(link->fileFixPwd);
  if (link->bbsPwd != link->defaultPwd) nfree(link->bbsPwd);
  if (link->sessionPwd != link->sessionPwd) nfree(link->sessionPwd);
  nfree(link->email);
  nfree(link->emailFrom);
  nfree(link->emailSubj);
  nfree(link->defaultPwd);
  nfree(link->pktFile);
  nfree(link->packFile);
  nfree(link->floFile);
  nfree(link->bsyFile);
  nfree(link->LinkGrp);
  freeGroups(link->AccessGrp, link->numAccessGrp);
  freeGroups(link->optGrp, link->numOptGrp);
  freeGroups(link->frMask, link->numFrMask);
  freeGroups(link->dfMask, link->numDfMask);
  nfree(link->forwardRequestFile);
  nfree(link->forwardFileRequestFile);
  nfree(link->denyFwdFile);
  nfree(link->autoAreaCreateDefaults);
  nfree(link->autoAreaCreateFile);
  nfree(link->autoFileCreateDefaults);
  nfree(link->autoFileCreateFile);
  nfree(link->RemoteRobotName);
  nfree(link->RemoteFileRobotName);
  nfree(link->msgBaseDir);
  return;
}

int e_readCheck(const s_fidoconfig *config, s_area *echo, s_link *link) {

    // rc == '\x0000' access o'k
    // rc == '\x0001' no access group
    // rc == '\x0002' no access level
    // rc == '\x0003' no access export
    // rc == '\x0004' not linked
    
    unsigned int i, rc = 0;

    for (i=0; i<echo->downlinkCount; i++) {
		if (link == echo->downlinks[i]->link) break;
    }
    if (i == echo->downlinkCount) return 4;

    // pause
    if (((link->Pause & EPAUSE) == EPAUSE) && echo->noPause==0) return 3;

    if (echo->group) {
		if (link->numAccessGrp) {
			if (config->numPublicGroup) {
				if (!grpInArray(echo->group,link->AccessGrp,link->numAccessGrp) &&
					!grpInArray(echo->group,config->PublicGroup,config->numPublicGroup))
					rc = 1;
			} else if (!grpInArray(echo->group,link->AccessGrp,link->numAccessGrp)) rc = 1;
		} else if (config->numPublicGroup) {
			if (!grpInArray(echo->group,config->PublicGroup,config->numPublicGroup)) rc = 1;
		} else if (link->numOptGrp==0) return 1;
		
		if (link->numOptGrp) {
			if (grpInArray(echo->group,link->optGrp,link->numOptGrp)) {
				if (link->export==0) return 3; else rc = 0;
			}
		}
	}
	
    if (echo->levelread > link->level) return 2;
    
    return rc;
}

int e_writeCheck(const s_fidoconfig *config, s_area *echo, s_link *link) {

    // rc == '\x0000' access o'k
    // rc == '\x0001' no access group
    // rc == '\x0002' no access level
    // rc == '\x0003' no access import
    // rc == '\x0004' not linked

    unsigned int i, rc = 0;

    for (i=0; i<echo->downlinkCount; i++) {
		if (link == echo->downlinks[i]->link) break;
    }
    if (i == echo->downlinkCount) return 4;
    
    if (echo->group) {
		if (link->numAccessGrp) {
			if (config->numPublicGroup) {
				if (!grpInArray(echo->group,link->AccessGrp,link->numAccessGrp) &&
					!grpInArray(echo->group,config->PublicGroup,config->numPublicGroup))
					rc = 1;
			} else if (!grpInArray(echo->group,link->AccessGrp,link->numAccessGrp)) rc = 1;
		} else if (config->numPublicGroup) {
			if (!grpInArray(echo->group,config->PublicGroup,config->numPublicGroup)) rc = 1;
		} else if (link->numOptGrp==0) return 1;

		if (link->numOptGrp) {
			if (grpInArray(echo->group,link->optGrp,link->numOptGrp)) {
				if (link->import==0) return 3; else rc = 0;
			}
		}
    }
	
    if (echo->levelwrite > link->level) return 2;
    
    return rc;
}

/* safe malloc, realloc, calloc */

void *smalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
		fprintf(stderr, "out of memory");
		abort();
    }
    return ptr;
}

void *srealloc(void *ptr, size_t size)
{
    void *newptr = realloc(ptr, size);
    if (newptr == NULL) {
		fprintf(stderr, "out of memory");
		abort();
    }
    return newptr;
}

void *scalloc(size_t nmemb, size_t size)
{
    void *ptr = smalloc(size*nmemb);
	memset(ptr,'\0',size*nmemb);
    return ptr;
}

char *sstrdup(const char *src)
{
    char *ptr;
    
    if (src == NULL) return NULL;
    ptr = strdup (src);
    if (ptr == NULL) {
		fprintf(stderr, "out of memory");
		abort();
    }
    return ptr;
}

char    *GetFilenameFromPathname(char* pathname)
{
    char *fname = strrchr(pathname,PATH_DELIM);
    if(fname)
        fname++;
    else
        fname = pathname;
    return fname;
}

char *makeMsgbFileName(ps_fidoconfig config, char *s) {
    // allowed symbols: 0..9, a..z, A..Z, ".,!@#$^()~-_{}[]"
    static char defstr[]="\"*/:;<=>?\\|%`'&+"; // not allowed
    char *name=NULL, *str;

    if (config->notValidFNChars) str = config->notValidFNChars;
    else str = defstr;

    while (*s) {
	if (strchr(str,*s)) xscatprintf(&name,"%%%x", *s);
	else xscatprintf(&name, "%c", *s);
	s++;
    }

    return name;
}

int NCreateOutboundFileName(ps_fidoconfig config, s_link *link, e_flavour prio, e_pollType typ)
{
   int fd; // bsy file for current link
   int nRet = 0;
   char *name=NULL, *sepDir=NULL, limiter=PATH_DELIM, *tmpPtr;
   e_bundleFileNameStyle bundleNameStyle = eUndef;

   if (link->linkBundleNameStyle!=eUndef) bundleNameStyle=link->linkBundleNameStyle;
   else if (config->bundleNameStyle!=eUndef) bundleNameStyle=config->bundleNameStyle;
   
   if (bundleNameStyle != eAmiga) {
	   if (link->hisAka.point) xscatprintf(&name, "%08x.", link->hisAka.point);
	   else xscatprintf(&name, "%04x%04x.", link->hisAka.net, link->hisAka.node);
   } else {
	   xscatprintf(&name, "%u.%u.%u.%u.", link->hisAka.zone,
				   link->hisAka.net, link->hisAka.node, link->hisAka.point);
   }

   if (typ != REQUEST) {
	   switch (prio) {
	   case crash :     xstrcat(&name, "c");
		   break;
	   case hold :      xstrcat(&name, "h");
		   break;
	   case direct :    xstrcat(&name, "d");
		   break;
	   case immediate : xstrcat(&name, "i");
		   break;
	   case normal :    xstrcat(&name, (typ==PKT) ? "o" : "f");
		   break;
	   }
   } else xstrcat(&name, "req");

   switch (typ) {
   case PKT :     xstrcat(&name, "ut");
	   break;
   case FLOFILE : xstrcat(&name, "lo");
	   break;
   case REQUEST :
	   break;
   }

   // create floFile
   xstrcat(&link->floFile, config->outbound);

   // add suffix for other zones
   if (link->hisAka.zone != config->addr[0].zone && bundleNameStyle != eAmiga) {
	   link->floFile[strlen(link->floFile)-1]='\0';
	   xscatprintf(&link->floFile, ".%03x%c", link->hisAka.zone, limiter);
   }

   if (link->hisAka.point && bundleNameStyle != eAmiga)
	   xscatprintf(&link->floFile, "%04x%04x.pnt%c",
				   link->hisAka.net, link->hisAka.node, limiter);
   
   _createDirectoryTree(link->floFile); // create directoryTree if necessary
   xstrcat(&link->bsyFile, link->floFile);
   xstrcat(&link->floFile, name);

   // separate bundles

   if (config->separateBundles && (bundleNameStyle!=eAmiga || (bundleNameStyle==eAmiga && link->packerDef==NULL))) {

       xstrcat(&sepDir, link->bsyFile);
       if (bundleNameStyle==eAmiga) 
	   xscatprintf(&sepDir, "%u.%u.%u.%u.sep%c", 
		       link->hisAka.zone, link->hisAka.net,
		       link->hisAka.node ,link->hisAka.point, limiter);
       else if (link->hisAka.point) xscatprintf(&sepDir, "%08x.sep%c", 
						link->hisAka.point, limiter);
       else xscatprintf(&sepDir, "%04x%04x.sep%c", link->hisAka.net,
			link->hisAka.node, limiter);

       _createDirectoryTree(sepDir);
       nfree(sepDir);
   }

   // create bsyFile
   if ((tmpPtr=strrchr(name, '.')) != NULL) *tmpPtr = '\0';
   xstrscat(&link->bsyFile, name, ".bsy", NULL);
   nfree(name);

   // maybe we have session with this link?
   if ( (fd=open(link->bsyFile, O_CREAT | O_RDWR | O_EXCL, S_IREAD | S_IWRITE)) < 0 ) {
#if !defined(__WATCOMC__)	   
	   int save_errno = errno;
	   
	   if (save_errno != EEXIST) {
		   w_log('7', "cannot create *.bsy file \"%s\" for %s (errno %d)\n", link->bsyFile, link->name, (int)save_errno);
         nRet = -1;
		   
	   } else {
#endif
		   w_log('7', "link %s is busy.", aka2str(link->hisAka));
		   nfree(link->floFile);
		   nfree(link->bsyFile);
		   nRet = 1;
#if !defined(__WATCOMC__)	   
	   }
#endif
   } else  {
      close(fd);
      nRet = 0;
   }
   return nRet;
}

int needUseFileBoxForLink (ps_fidoconfig config, s_link *link)
{
    char limiter=PATH_DELIM;
    char *bsyFile = NULL;
    e_bundleFileNameStyle bundleNameStyle = eUndef;

    /* link->useFileBox means:
     * 0 - unknown, need to check
     * 1 - don't use
     * 2 - use box
     */

    if (link->useFileBox == 1) return 0; // Don't use
    if (link->useFileBox == 2) return 1; // Use

    // link->useFileBox == 0 -> still don't know

    if ( (link->fileBox==NULL && config->fileBoxesDir==NULL) ||
         (theApp.module == 2  && !link->tickerPackToBox)
       )
    {
        link->useFileBox = 1;
        return 0;
    }

    if (link->fileBoxAlways) {
	link->useFileBox = 2;
	return 1;
    }

    // check if can we use outbound
    xstrcat(&bsyFile, config->outbound);

    // add suffix for other zones
    if (link->hisAka.zone != config->addr[0].zone && bundleNameStyle != eAmiga) {
	bsyFile[strlen(bsyFile)-1]='\0';
	xscatprintf(&bsyFile, ".%03x%c", link->hisAka.zone, limiter);
    }

    if (link->hisAka.point && bundleNameStyle != eAmiga)
	xscatprintf(&bsyFile, "%04x%04x.pnt%c",
		    link->hisAka.net, link->hisAka.node, limiter);
   
    _createDirectoryTree(bsyFile); // create directoryTree if necessary

    if (link->linkBundleNameStyle!=eUndef) bundleNameStyle=link->linkBundleNameStyle;
    else if (config->bundleNameStyle!=eUndef) bundleNameStyle=config->bundleNameStyle;
   
    if (bundleNameStyle != eAmiga) {
	if (link->hisAka.point) xscatprintf(&bsyFile, "%08x", link->hisAka.point);
	else xscatprintf(&bsyFile, "%04x%04x", link->hisAka.net, link->hisAka.node);
    } else {
	xscatprintf(&bsyFile, "%u.%u.%u.%u", link->hisAka.zone,
		    link->hisAka.net, link->hisAka.node, link->hisAka.point);
    }

    xstrscat(&bsyFile, ".bsy", NULL);
    
    if (fexist(bsyFile)) {
	link->useFileBox = 2;
    } else {
	// link is not busy, use outrbound
	link->useFileBox = 1;
    }

    nfree (bsyFile);

    return link->useFileBox - 1;
}

char *makeFileBoxName (ps_fidoconfig config, s_link *link)
{
    char *name=NULL;

    xscatprintf (&name, "%s%d.%d.%d.%d%s%c",
		 config->fileBoxesDir,
		 link->hisAka.zone,
		 link->hisAka.net,
		 link->hisAka.node,
		 link->hisAka.point,
		 (link->echoMailFlavour==hold) ? ".h" : "",
		 PATH_DELIM);
    return name;
}

void fillCmdStatement(char *cmd, const char *call, const char *archiv, const char *file, const char *path)
{
    const char *start, *tmp, *add;
    
    *cmd = '\0';  start = NULL;
    for (tmp = call; (start = strchr(tmp, '$')) != NULL; tmp = start + 2) {
        switch(*(start + 1)) {
        case 'a': add = archiv; break;
        case 'p': add = path; break;
        case 'f': add = file; break;
        default:
            strncat(cmd, tmp, (size_t) (start - tmp + 1));
            start--; continue;
        };
        strncat(cmd, tmp, (size_t) (start - tmp));
        strcat(cmd, add);
    };
    strcat(cmd, tmp);
}

char *changeFileSuffix(char *fileName, char *newSuffix) {

    int   i = 1;
    char  buff[255];

    char *beginOfSuffix = strrchr(fileName, '.')+1;
    char *newFileName;
    int  length = strlen(fileName)-strlen(beginOfSuffix)+strlen(newSuffix);

    newFileName = (char *) smalloc((size_t) length+1+2);
    memset(newFileName, '\0',length+1+2);
    strncpy(newFileName, fileName, length-strlen(newSuffix));
    strcat(newFileName, newSuffix);

#ifdef DEBUG_HPT
    printf("old: %s      new: %s\n", fileName, newFileName);
#endif

    while (fexist(newFileName) && (i<255)) {
	sprintf(buff, "%02x", i);
	beginOfSuffix = strrchr(newFileName, '.')+1;
	strncpy(beginOfSuffix+1, buff, 2);
	i++;
    }

    if (!fexist(newFileName)) {
	rename(fileName, newFileName);
	return newFileName;
    } else {
	w_log('9', "Could not change suffix for %s. File already there and the 255 files after", fileName);
	nfree(newFileName);
	return NULL;
    }
}

unsigned int dec2oct(unsigned int decimal)
{
    char tmpstr[6];
    unsigned int mode;

    mode = decimal;
    sprintf(tmpstr, "%u", mode);
    sscanf(tmpstr, "%o", &mode);
    return mode;
}