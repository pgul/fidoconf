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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <huskylib/compiler.h>

#ifdef HAS_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAS_IO_H
#  include <io.h>
#endif

#ifdef HAS_DOS_H
#include <dos.h>
#endif

#ifdef HAS_PROCESS_H
#include <process.h>
#endif

#ifdef __BEOS__
#include <KernelKit.h>
#endif

#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "fidoconf.h"
#include "common.h"


static char *attrStr[] = { "pvt", "crash", "read", "sent", "att",
                       "fwd", "orphan", "k/s", "loc", "hld",
                       "xx2",  "frq", "rrq", "cpt", "arq", "urq" };
static char *eattr[] = { "KFS", "TFS", "DIR", "IMM", "CFM", "NPD" };

long  str2attr(const char *str)
{
   int i;
   for (i = 0; i < sizeof(attrStr) / sizeof(char *); i++)
           if (strncasecmp(str, attrStr[i], strlen(attrStr[i]))==0)
                   return 1 << i;
   return -1L;
}

char *attr2str(long attr)
{
    char *flags = NULL;
    int  i;
    for (i = 0; i < sizeof(attrStr) / sizeof(char *); i++)
	if (attr & (1 << i))
	    xstrscat(&flags, flags ? " " : "", attrStr[i], NULL);
    return flags;
}

char *extattr(const char *line)
{
    int i;

    for (i=0; i<sizeof(eattr)/sizeof(eattr[0]); i++)
	if (stricmp(line, eattr[i]) == 0)
	    return eattr[i];
    return NULL;
}

int  addrComp(const hs_addr a1, const hs_addr a2)
{
   int rc = 0;

   rc =  a1.zone  != a2.zone;
   rc += a1.net   != a2.net;
   rc += a1.node  != a2.node;
   rc += a1.point != a2.point;

   return rc;
}


void string2addr(const char *string, hs_addr *addr) {
	char *endptr;
	const char *str = string;
	unsigned long t;

	memset(addr, '\0', sizeof(hs_addr));

	if (str == NULL) return;
	if (strchr(str,':')==NULL || strchr(str,'/')==NULL) return;

	/*  zone */
	if (NULL == strstr(str,":")) return;
	t = strtoul(str,&endptr,10);
	addr->zone = (hUINT16) t;
	if(!addr->zone) return; /*  there is no zero zones in practice */

	/*  net */
	str = endptr+1;
	if (NULL == strstr(str,"/")) return;
	t = strtoul(str,&endptr,10);
	addr->net = (hUINT16) t;

	/*  node */
	str = endptr+1;
	t = strtoul(str,&endptr,10);
	addr->node = (hUINT16) t;

	/*  point */
	if (*endptr && !isspace( endptr[0] )) str = endptr+1;
	else return; /*  end of string */
	t = strtoul(str,&endptr,10);
	addr->point = (hUINT16) t;
	
	return;
}


char *aka2str(const hs_addr aka) {
  static char straka[SIZE_aka2str];

    if (aka.point) sprintf(straka,"%u:%u/%u.%u",aka.zone,aka.net,aka.node,aka.point);
    else sprintf(straka,"%u:%u/%u",aka.zone,aka.net,aka.node);
	
    return straka;
}

/* Store 5d-address string into allocalted array (by malloc()).
 */
char *aka2str5d(hs_addr aka) {
  char *straka=NULL;

    if (aka.point) {
      if (aka.domain)
        xscatprintf( &straka, "%u:%u/%u.%u@%s", aka.zone, aka.net, aka.node,
                                                aka.point, aka.domain );
      else
        xscatprintf( &straka, "%u:%u/%u.%u", aka.zone, aka.net, aka.node,
                                                aka.point );
    }else
      if (aka.domain)
        xscatprintf( &straka, "%u:%u/%u@%s", aka.zone, aka.net, aka.node,
                                                aka.domain );
      else
        xscatprintf( &straka, "%u:%u/%u", aka.zone, aka.net, aka.node );

    return straka;
}

void freeGroups(char **grps, int numGroups)
{
	nfree (grps);
}

char **copyGroups(char **grps, int numGroups)
{
	char **dst;
	int  i, len = 0;

	if (grps == NULL || numGroups == 0) return NULL;
	for (i=0; i<numGroups; i++)
		len += sstrlen(grps[i]) + 1;
	dst = smalloc(sizeof(char *)*numGroups + len);
	dst[0] = (char *)(dst + numGroups);
	for (i=0; i<numGroups; i++) {
		if (i>0) dst[i] = dst[i-1] + strlen(dst[i-1]) + 1;
		sstrcpy(dst[i], grps[i]);
	}
	return dst;
}

void freeLink (s_link *link)
{

  if (link == NULL) return;

  nfree (link->hisAka.domain);
  nfree (link->hisPackAka.domain);
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

int e_readCheck(s_fidoconfig *config, s_area *echo, s_link *link) {

    /*  rc == '\x0000' access o'k */
    /*  rc == '\x0001' no access group */
    /*  rc == '\x0002' no access level */
    /*  rc == '\x0003' no access export */
    /*  rc == '\x0004' not linked */

    unsigned i, rc = 0;
    unsigned Pause = echo->areaType;
    /* check for OurAka */
    if(!isOurAka(config,link->hisAka))
    {
        for (i=0; i<echo->downlinkCount; i++) {
            if (link == echo->downlinks[i]->link) break;
        }
        if (i == echo->downlinkCount) return 4;
    } else if ( echo->msgbType  == MSGTYPE_PASSTHROUGH ) {
        return 4;
    }
    /*  pause */
    if (((link->Pause & Pause) == Pause) && echo->noPause==0) return 3;

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

int e_writeCheck(s_fidoconfig *config, s_area *echo, s_link *link) {

    /*  rc == '\x0000' access o'k */
    /*  rc == '\x0001' no access group */
    /*  rc == '\x0002' no access level */
    /*  rc == '\x0003' no access import */
    /*  rc == '\x0004' not linked */

    unsigned int i, rc = 0;
    /* check for OurAka */
    if(!isOurAka(config,link->hisAka))
    {
        for (i=0; i<echo->downlinkCount; i++) {
            if (link == echo->downlinks[i]->link) break;
        }
        if (i == echo->downlinkCount) return 4;
    } else if ( echo->msgbType  == MSGTYPE_PASSTHROUGH ) {
        return 4;
    }

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



char *makeMsgbFileName(ps_fidoconfig config, char *s) {
    /* allowed symbols: 0..9, a..z, A..Z, ".,!@#$^()~-_{}[]" */
    static char defstr[]="\"*/:;<=>?\\|%`'&+"; /*  not allowed */
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

int NCreateOutboundFileNameAka(ps_fidoconfig config, s_link *link, e_flavour prio, e_pollType typ, hs_addr *aka)
{
   int fd; /*  bsy file for current link */
   int nRet = 0;
   char *name=NULL, *sepDir=NULL, limiter=PATH_DELIM, *tmpPtr;
   e_bundleFileNameStyle bundleNameStyle = eUndef;

   if (link->linkBundleNameStyle!=eUndef) bundleNameStyle=link->linkBundleNameStyle;
   else if (config->bundleNameStyle!=eUndef) bundleNameStyle=config->bundleNameStyle;

   if (bundleNameStyle != eAmiga) {
	   if (aka->point) xscatprintf(&name, "%08x.", aka->point);
	   else xscatprintf(&name, "%04x%04x.", aka->net, aka->node);
   } else {
	   xscatprintf(&name, "%u.%u.%u.%u.", aka->zone,
				   aka->net, aka->node, aka->point);
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
	   default :    xstrcat(&name, (typ==PKT) ? "o" : "f");
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

   /*  create floFile */
   xstrcat(&link->floFile, config->outbound);

   /*  add suffix for other zones */
   if (aka->zone != config->addr[0].zone && bundleNameStyle != eAmiga) {
	   link->floFile[strlen(link->floFile)-1]='\0';
	   xscatprintf(&link->floFile, ".%03x%c", aka->zone, limiter);
   }

   if (aka->point && bundleNameStyle != eAmiga)
	   xscatprintf(&link->floFile, "%04x%04x.pnt%c",
				   aka->net, aka->node, limiter);

   _createDirectoryTree(link->floFile); /*  create directoryTree if necessary */
   xstrcat(&link->bsyFile, link->floFile);
   xstrcat(&link->floFile, name);

   /*  separate bundles */

   if (config->separateBundles && (bundleNameStyle!=eAmiga || (bundleNameStyle==eAmiga && link->packerDef==NULL))) {

       xstrcat(&sepDir, link->bsyFile);
       if (bundleNameStyle==eAmiga)
	   xscatprintf(&sepDir, "%u.%u.%u.%u.sep%c",
		       aka->zone, aka->net,
		       aka->node ,aka->point, limiter);
       else if (aka->point) xscatprintf(&sepDir, "%08x.sep%c",
						aka->point, limiter);
       else xscatprintf(&sepDir, "%04x%04x.sep%c", aka->net,
			aka->node, limiter);

       _createDirectoryTree(sepDir);
       nfree(sepDir);
   }

   /*  create bsyFile */
   if ((tmpPtr=strrchr(name, '.')) != NULL) *tmpPtr = '\0';
   xstrscat(&link->bsyFile, name, ".bsy", NULL);
   nfree(name);

   /*  maybe we have session with this link? */
   if ( (fd=open(link->bsyFile, O_CREAT | O_RDWR | O_EXCL, S_IREAD | S_IWRITE)) < 0 ) {
#if !defined(__WATCOMC__)	
     int save_errno = errno;
     if (save_errno != EEXIST) {
       w_log('7', "cannot create *.bsy file \"%s\" for %s (errno %d)\n", link->bsyFile, aka2str(*aka), (int)save_errno);
       nRet = -1;
       errno = save_errno;
     } else
#endif
     {
       w_log('7', "link %s is busy.", aka2str(*aka));
       nfree(link->floFile);
       nfree(link->bsyFile);
       nRet = 1;
     }
   } else  {
#ifdef HAS_getpid
      FILE *fdd;
      if((fdd = fdopen(fd,"w+"))) {
        fprintf(fd,"%u",getpid());
        fclose(fdd);
      }
#else
      close(fd);
#endif
      nRet = 0;
   }
   return nRet;
}

int NCreateOutboundFileName(ps_fidoconfig config, s_link *link, e_flavour prio, e_pollType typ)
{
  return NCreateOutboundFileNameAka(config, link, prio, typ, &(link->hisAka));
}

int needUseFileBoxForLinkAka(ps_fidoconfig config, s_link *link, hs_addr *aka)
{
    char limiter=PATH_DELIM;
    char *bsyFile = NULL;
    e_bundleFileNameStyle bundleNameStyle = eUndef;

    /* link->useFileBox means:
     * 0 - unknown, need to check
     * 1 - don't use
     * 2 - use box
     */

    if (link->useFileBox == 1) return 0; /*  Don't use */
    if (link->useFileBox == 2) return 1; /*  Use */

    /*  link->useFileBox == 0 -> still don't know */

    if ( (link->fileBox==NULL && config->fileBoxesDir==NULL) ||
         (theApp.module == M_HTICK  && !link->tickerPackToBox)
       )
    {
        link->useFileBox = 1;
        return 0;
    }

    if (link->fileBoxAlways) {
	link->useFileBox = 2;
	return 1;
    }

    /*  check if can we use outbound */
    xstrcat(&bsyFile, config->outbound);

    /*  add suffix for other zones */
    if (aka->zone != config->addr[0].zone && bundleNameStyle != eAmiga) {
	bsyFile[strlen(bsyFile)-1]='\0';
	xscatprintf(&bsyFile, ".%03x%c", aka->zone, limiter);
    }

    if (aka->point && bundleNameStyle != eAmiga)
	xscatprintf(&bsyFile, "%04x%04x.pnt%c",
		    aka->net, aka->node, limiter);

    _createDirectoryTree(bsyFile); /*  create directoryTree if necessary */

    if (link->linkBundleNameStyle!=eUndef) bundleNameStyle=link->linkBundleNameStyle;
    else if (config->bundleNameStyle!=eUndef) bundleNameStyle=config->bundleNameStyle;

    if (bundleNameStyle != eAmiga) {
	if (aka->point) xscatprintf(&bsyFile, "%08x", aka->point);
	else xscatprintf(&bsyFile, "%04x%04x", aka->net, aka->node);
    } else {
	xscatprintf(&bsyFile, "%u.%u.%u.%u", aka->zone,
		    aka->net, aka->node, aka->point);
    }

    xstrscat(&bsyFile, ".bsy", NULL);

    if (fexist(bsyFile)) {
	link->useFileBox = 2;
    } else {
	/*  link is not busy, use outrbound */
	link->useFileBox = 1;
    }

    nfree (bsyFile);

    return link->useFileBox - 1;
}

int needUseFileBoxForLink(ps_fidoconfig config, s_link *link)
{
  return needUseFileBoxForLinkAka(config, link, &(link->hisAka));
}

char *makeFileBoxNameAka (ps_fidoconfig config, s_link *link, hs_addr *aka)
{
    char *name=NULL;

    xscatprintf (&name, "%s%d.%d.%d.%d%s%c",
		 config->fileBoxesDir,
		 aka->zone,
		 aka->net,
		 aka->node,
		 aka->point,
		 (link->echoMailFlavour==hold) ? ".h" : "",
		 PATH_DELIM);
    return name;
}

char *makeFileBoxName (ps_fidoconfig config, s_link *link)
{
  return makeFileBoxNameAka(config, link, &(link->hisAka));
}

void fillCmdStatement(char *cmd, const char *call, const char *archiv, const char *file, const char *path)
{
    const char *start, *tmp, *add;
#ifdef __WIN32__
    char *p;
#endif
    char fullarch[256];
    char fullpath[256];

#ifdef __WIN32__
    GetFullPathName(archiv, sizeof(fullarch), fullarch, &p);
    if(*path)
    GetFullPathName(path, sizeof(fullpath), fullpath, &p);
#else
    strnzcpy(fullpath,path, 255);
    strnzcpy(fullarch,archiv, 255);
#endif

    *cmd = '\0';  start = NULL;
    for (tmp = call; (start = strchr(tmp, '$')) != NULL; tmp = start + 2) {
        switch(*(start + 1)) {
        case 'a': add = fullarch; break;
        case 'p': add = fullpath; break;
        case 'f': add = file; break;
        default:
            strncat(cmd, tmp, (size_t) (start - tmp + 1));
            start--; continue;
        };
        strnzcat(cmd, tmp, (size_t) (start - tmp));
        strcat(cmd, add);
    };
    strcat(cmd, tmp);
}


/*  Change file sufix (add if not present).
    inc = 1 - increment suffix of file if new file exist;
          rename file; return new file name or NULL; set errno
    inc = 0 - do not increment suffix, do not rename file, return new suffix only
    if 1st or 2nd parameter is NULL return NULL and set errno to EINVAL
*/
char *changeFileSuffix(char *fileName, char *newSuffix, int inc) {

    int  i;
    char buff[3];
    char *beginOfSuffix;
    char *newFileName=NULL;
    size_t  length;

    if(!(fileName && newSuffix)){
      w_log( LL_ERR, "changeFileSuffix() illegal call: %s parameter is NULL",
             fileName? "2nd" : "1st" );
      errno = EINVAL;
      return NULL;
    }

    beginOfSuffix = strrchr(fileName, '.');
    if(beginOfSuffix==NULL || beginOfSuffix<strrchr(fileName, '\\') || beginOfSuffix<strrchr(fileName, '/'))
      beginOfSuffix = fileName+strlen(fileName)+1; /* point char not found in filename, pointed to end of string */
    else beginOfSuffix++; /* pointed after point in 'name.suf' */

    length = beginOfSuffix-fileName;      /* length "name." */
    i=strlen(newSuffix);
    newFileName = (char *) scalloc( 1, (size_t)(length+i+(i>3?1:4-i)) );

    strncpy(newFileName, fileName, length);
    if( length > strlen(newFileName) )
      strcat(newFileName, ".");
    strcat(newFileName, newSuffix);

    if(inc == 0){
        w_log(LL_DEBUGF, __FILE__ ":%u: old: '%s' new: '%s'",__LINE__, fileName, newFileName);
        return newFileName;
    }

    beginOfSuffix = newFileName+length+1; /*last 2 chars*/
    for (i=1; fexist(newFileName) && (i<255); i++) {
#ifdef HAS_snprintf
        snprintf(buff, sizeof(buff), "%02x", i);
#else
        sprintf(buff, "%02x", i);
#endif
        strnzcpy(beginOfSuffix, buff, 2);
    }

    w_log(LL_DEBUGF, __FILE__ ":%u: old: '%s' new: '%s'",__LINE__, fileName, newFileName);
    if (!fexist(newFileName)) {
        if(rename(fileName, newFileName)){ /* -1 = error */
          w_log(LL_ERR, "Could not rename '%s' to '%s': %s", fileName, newFileName, strerror (errno));
          nfree(newFileName);
          return NULL;
        }
        return newFileName;
    } else {
        w_log(LL_ERR, "Could not change suffix for %s. File already there and the 255 files after", fileName);
        nfree(newFileName);
        errno = EEXIST;
        return NULL;
    }
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
#ifdef HAS_sleep
        sleep(1);
#else
        mysleep(1);
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
#ifdef HAS_sleep
        sleep(1);
#else
        mysleep(1);
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
#ifdef HAS_sleep
                   sleep(1);
#else
                   mysleep(1);
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

hs_addr *SelectPackAka(s_link *link)
{
 if (link->hisPackAka.zone != 0)
   return &(link->hisPackAka);
 else
   return &(link->hisAka);
}