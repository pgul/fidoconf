#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "fidoconf.h"
#include "typesize.h"
#include "common.h"
#include "xstr.h"

#define setcond for (i=0, condition=1; i<=iflevel; condition=ifstack[i++].state && condition);

static char *curconfname=NULL;
static long curconfpos=0;
static FILE *hcfg=NULL;
static short condition;
static int  iflevel, nvars, sp;
static int  maxnvars, maxsp, maxif;
static struct { short state, inelse, wastrue;
              } *ifstack=NULL;
static struct { char *var, *value;
              } *set=NULL;
static struct {
        FILE *farr;
        int  curline;
        char *confname;
      } *incstack=NULL;

static unsigned int cfgNamesCount;
static char **cfgNames=NULL;

int init_conf(char *conf_name)
{
  iflevel=-1;
  condition=1;
  sp=0;
  cfgNamesCount=0;
  hcfg=fopen(conf_name, "rb");
  if (hcfg==NULL)
  {
    fprintf(stderr, "Can't open config file %s: %s!\n",
            conf_name, strerror(errno));
    wasError = 1;
    return -1;
  }
  curconfname=sstrdup(conf_name);
  actualLineNr=0;
#if defined(UNIX)
  setvar("OS", "UNIX");
#elif defined(OS2)
  setvar("OS", "OS/2");
#elif defined(NT) || defined(WINNT) || defined(__NT__)
  setvar("OS", "WIN");
#elif defined(MSDOS)
  setvar("OS", "MSDOS");
#endif
  setvar("[", "[");
  setvar("`", "`");
  return 0;
}

char *getvar(char *name)
{ int i;

  for (i=0; i<nvars; i++)
    if (sstricmp(name, set[i].var)==0)
    { if (set[i].value[0]==0)
        return NULL;
      return set[i].value;
    }
  return getenv(name);
}

void setvar(char *name, char *value)
{ int i, j;

  /* find var */
  for (i=0; i<nvars; i++)
    if (sstricmp(set[i].var, name)==0)
      break;
  if (i<nvars)
  { /* remove var */
    nfree(set[i].var);
    for (j=i; j<nvars-1; j++)
    { set[j].var=set[j+1].var;
      set[j].value=set[j+1].value;
    }
    nvars--;
  }
  if (value==NULL) value="";
  if (value[0]==0)
    if (getvar(value)==NULL)
      return;
  if (nvars==maxnvars)
    set = srealloc(set, (maxnvars+=10)*sizeof(*set));
  set[nvars].var=smalloc(sstrlen(name)+sstrlen(value)+2);
  sstrcpy(set[nvars].var, name);
  set[nvars].value=set[nvars].var+sstrlen(name)+1;
  sstrcpy(set[nvars].value, value);
  nvars++;
  return;
}

void close_conf(void)
{
  int i;
  char *module;

  module = getvar("module");
  if (module) module = sstrdup(module);
  for(i=0; i<nvars; i++)
    nfree(set[i].var);
  maxnvars=nvars=0;
  nfree(set);
  if (module)
  { setvar("module", module);
    nfree(module);
  }
  nfree(ifstack);
  maxif=0;
  if (hcfg) fclose(hcfg);
  hcfg=NULL;
  for (i=0; i<sp; i++) {
    fclose(incstack[i].farr);
    nfree(incstack[i].confname);
  }
  nfree(curconfname);
  nfree(incstack);
  sp=maxsp=0;
  for (i=0; i<(int)cfgNamesCount; i++) nfree(cfgNames[i]);
  nfree(cfgNames);
  cfgNamesCount=0;
}

static char *_configline(void)
{
  char *line;

  curconfpos = ftell(hcfg);
  line = readLine(hcfg);
  if (line == NULL)
    return NULL;
  actualLineNr++;
  return line;
}

char *vars_expand(char *line)
{
  int  curlen;
  char *parsed, *src, *dest, *p, *p1, *newparsed;
#if defined(UNIX) || (defined(OS2) && defined(__EMX__))
  FILE *f;
  int  i;
#endif

#if defined(UNIX) || (defined(OS2) && defined(__EMX__))
  if (strpbrk(line, "[`")==NULL)
#else
  if (strchr(line, '[')==NULL)
#endif
     return line;
  curlen = sstrlen(line)+1;
  parsed = dest = smalloc(curlen);
  for (src = line; *src; src++)
  {
    if (dest-parsed >= curlen-2)
    {
      size_t offset = (size_t) (dest - parsed);
             /* we need this to fake around boundary checking */

      newparsed = srealloc(parsed, curlen+=80);
      dest = newparsed + offset;
      parsed = newparsed;
    }
    switch (*src)
    {
#if defined(UNIX) || (defined(OS2) && defined(__EMX__))
      case '`':
        p = strchr(src+1, '`');
        if (p == NULL)
        {
          *dest++ = *src;
          continue;
        }
        *p = '\0';
	src++;
	f = popen(src, "r");
        *p = '`';
        src = p;
        while ((i = fgetc(f)) != EOF)
        {
          if (dest-parsed >= curlen-2)
          {
            newparsed = srealloc(parsed, curlen+=80);
            dest = newparsed+(unsigned)(dest-parsed);
            parsed = newparsed;
          }
          if (i!='\n') *dest++ = (char)i;
        }
        pclose(f);
        continue;
#endif
      case '[':
        p = strchr(src, ']');
        if (p)
        {
          src++;
          *p = '\0';
          if ((p1 = getvar(src)) == NULL)
            p1 = src;
          if (sstrlen(p1) > sstrlen(src)+2)
          {
            newparsed = srealloc(parsed, curlen += sstrlen(p1)-sstrlen(src)-2);
            dest = newparsed+(unsigned)(dest-parsed);
            parsed = newparsed;
          }
          sstrcpy(dest, p1);
          dest += sstrlen(p1);
          *p = ']';
          src = p;
          continue;
        }
      default:
        *dest++ = *src;
        continue;
    }
  }
  *dest++ = '\0';
  if (curlen != dest-parsed)
    parsed = srealloc(parsed, (unsigned)(dest-parsed));
  nfree(line);
  return parsed;
}

static short boolexpr(char *str)
{ char *p, *p1, *p2;
  short ret, inquote, relax;

  ret=1;
  for (p=str; isspace(*p); p++);
  if (strncasecmp(p, "not ", 4)==0)
  { ret=0;
    for (p+=4; isspace(*p); p++);
  }
  inquote=0;
  for (p1=p; *p1; p1++)
  {
    if (p1[0]=='\\' && (p1[1]=='\\' || p1[1]=='\"'))
    { p1++;
      continue;
    }
    if (*p1=='\"')
    { inquote = !inquote;
      continue;
    }
    if (!inquote)
      if ((p1[0] == '=' || p1[0] == '!') && (p1[1] == '=' || p1[1] == '~'))
        break;
  }
  if (*p1==0)
  { fprintf(stderr, "Bad if expression in config %s, line %d: '%s'\n",
            curconfname, actualLineNr, str);
    wasError = 1;
    return ret;
  }
  if (p1[0]=='!') ret=!ret;
  relax=(p1[1]=='~');
  *p1=0;
  for (p2=p1-1; isspace(*p2); *p2--=0);
  for (p1+=2; isspace(*p1); p1++);
  for (p2=p1+sstrlen(p1)-1; isspace(*p2); *p2--=0);
  if (relax ? patimat(p, p1) : sstricmp(p, p1))
    ret=!ret;
  return ret;
}

char *configline(void)
{ int  i;
  char *p, *p1, *p2, *str, *line=NULL;

  for (;;) {
    nfree(line);
    line=str=_configline();
    if (str==NULL) {
       // save parsed config name
       cfgNames = srealloc(cfgNames, sizeof(char*)*(cfgNamesCount+1));
       cfgNames[cfgNamesCount] = NULL;
       xstrcat(&cfgNames[cfgNamesCount], curconfname);
       cfgNamesCount++;
       if (sp) {
          fclose(hcfg);
          nfree(curconfname);
          hcfg=incstack[--sp].farr;
          actualLineNr=incstack[sp].curline;
          curconfname=incstack[sp].confname;
          continue;
       }
       return NULL;
    }
    while (*str && isspace(*str)) str++;
    if (strncasecmp(str, "if ", 3)==0)
    {
      p=vars_expand(line); str+=(p-line); line=p;
      iflevel++;
      if (iflevel==maxif)
        ifstack=srealloc(ifstack, (maxif+=10)*sizeof(*ifstack));
      ifstack[iflevel].inelse=0;
      ifstack[iflevel].state=ifstack[iflevel].wastrue=boolexpr(str+3);
      condition = condition && ifstack[iflevel].state;
      continue;
    }
    if ((strncasecmp(str, "ifdef ",  6)==0) ||
        (strncasecmp(str, "ifndef ", 7)==0))
    {
      p=vars_expand(line); str+=(p-line); line=p;
      for (p1=str+sstrlen(str)-1; isspace(*p1); *p1--='\0');
      for (p=str+6; isspace(*p); p++);
      if (*p=='\0')
      { fprintf(stderr, "Bad %s in config %s line %d!\n",
                str, curconfname, actualLineNr);
        wasError = 1;
        continue;
      }
      iflevel++;
      if (iflevel==maxif)
        ifstack=srealloc(ifstack, (maxif+=10)*sizeof(*ifstack));
      ifstack[iflevel].inelse=0;
      ifstack[iflevel].state=(getvar(p)!=NULL);
      if (tolower(str[2])=='n') /* ifndef */
        ifstack[iflevel].state=!ifstack[iflevel].state;
      ifstack[iflevel].wastrue=ifstack[iflevel].state;
      condition = condition && ifstack[iflevel].state;
      continue;
    }
    if (strncasecmp(str, "elseif ", 7)==0)
    {
      if ((iflevel==-1) || ifstack[iflevel].inelse)
      { fprintf(stderr, "Misplaces elseif in config %s line %d ignored!\n",
                curconfname, actualLineNr);
        wasError = 1;
        continue;
      }
      p=vars_expand(line); str+=(p-line); line=p;
      if (ifstack[iflevel].wastrue)
        ifstack[iflevel].state=0;
      else
        ifstack[iflevel].state=ifstack[iflevel].wastrue=boolexpr(str+6);
      setcond;
      continue;
    }
    if (strncasecmp(str, "else", 4)==0)
    {
      if ((iflevel==-1) || ifstack[iflevel].inelse)
      { fprintf(stderr, "Misplaces else in config %s line %d ignored!\n",
                curconfname, actualLineNr);
        wasError = 1;
        continue;
      }
      ifstack[iflevel].inelse=1;
      ifstack[iflevel].state=!ifstack[iflevel].wastrue;
      setcond;
      continue;
    }
    if (strncasecmp(str, "endif", 5)==0)
    {
      if (iflevel==-1)
      { fprintf(stderr, "Misplaced endif in config %s line %d ignored!\n",
                curconfname, actualLineNr);
        wasError = 1;
        continue;
      }
      iflevel--;
      setcond;
      continue;
    }
    if (!condition)
      continue;
    if (strncasecmp(str, "set ", 4)==0)
    {
      p=vars_expand(line); str+=(p-line); line=p;
      p=strchr(str, '\n');
      if (p) *p=0;
      p1=strchr(str+4, '=');
      if (p1==NULL)
      { fprintf(stderr, "Incorrect set in config %s line %d!\n",
                curconfname, actualLineNr);
        wasError = 1;
        continue;
      }
      *p1=0;
      for (p=p1-1; isspace(*p); *p--='\0');
      for (p=str+4; isspace(*p); p++);
      /* now p - name of var */
      for (p1++; isspace(*p1); p1++);
      if (*p1=='\"')
      { /* remove quote chars */
        for (p2=p1; (p2=strchr(p2+1, '\"'))!=NULL;)
          if (*(p2-1)!='\\')
            *p2--='\0';
        p1++;
      }
      setvar(p, p1);
      continue;
    }
    if (strncasecmp(str, "include", 7)==0)
    {
      p=vars_expand(line); str+=(p-line); line=p;
      for (p=str+7; (*p==' ') || (*p=='\t'); p++);
      for (p1=p+sstrlen(p)-1; isspace(*p1); *p1--=0);
      for (i=0; i<sp; i++)
        if (sstrcmp(incstack[i].confname, p) == 0)
        { fprintf(stderr, "Line %d: WARNING: recursive include of file %s detected and fixed!\n", actualLineNr, p);
          continue;
        }
      if (sp==maxsp)
        incstack=srealloc(incstack, (maxsp+=10)*sizeof(*incstack));
      incstack[sp].farr=hcfg;
      hcfg=fopen(p, "rb");
      if (hcfg==NULL)
      { fprintf(stderr, "Can't open include file %s: %s!\n", p, strerror(errno));
        hcfg=incstack[sp].farr;
        wasError = 1;
        continue;
      }
      incstack[sp].confname=curconfname;
      incstack[sp].curline=actualLineNr;
      sp++;
      curconfname=sstrdup(p);
      actualLineNr=0;
      continue;
    }
    if ((strncasecmp(str, "commentchar", 11) == 0) && isspace(str[11]))
    {
      for (p=str+11; isspace(*p); p++);
      if (!*p)
      { printf("\"%s\", line %d: There is a comment character missing after CommentChar!\n", curconfname, actualLineNr);
        continue;
      }
      if (!strchr(TRUE_COMMENT, *p))
      { printf("\"%s\", line %d: CommentChar - '%c' is not valid comment characters!\n", curconfname, actualLineNr, *p);
      } else
      { CommentChar = *p;
      }
      continue;
    }
    return line;
  }
}

#if defined (UNIX)
static int cmpfnames(char *file1, char *file2)
{
    struct stat st1, st2;
    if (stat(file1, &st1) || stat(file2, &st2))
	return 1;
    if (st1.st_dev!=st2.st_dev || st1.st_ino!=st2.st_ino)
	return 1;
    return 0;
}
#elif defined(NT) || defined(WINNT) || defined(__NT__) || defined(__MINGW32__)
#ifdef __MINGW32__
typedef unsigned long DWORD;
typedef char        *LPSTR;
typedef const char  *LPCSTR;
DWORD __stdcall GetFullPathNameA(LPCSTR,DWORD,LPSTR,LPSTR*);
DWORD __stdcall GetShortPathNameA(LPCSTR,LPSTR,DWORD);
#define GetFullPathName     GetFullPathNameA
#define GetShortPathName    GetShortPathNameA
#else
#include <windows.h>
#endif
int cmpfnames(char *file1, char *file2)
{
    char buf[256], path1[256], path2[256], *p;

    if (sstricmp(file1, file2) == 0) return 0;
    if (!GetShortPathName(file1, buf, sizeof(buf)))
	sstrncpy(buf, file1, sizeof(buf));
    if (!GetFullPathName(buf, sizeof(path1), path1, &p)) return 1;
    if (!GetShortPathName(file2, buf, sizeof(buf)))
	sstrncpy(buf, file2, sizeof(buf));
    if (!GetFullPathName(buf, sizeof(path2), path2, &p)) return 1;

    return sstricmp(path1, path2);
}
#elif defined (OS2)
#define INCL_DOSFILEMGR
#include <os2.h>
static int cmpfnames(char *file1, char *file2)
{
    char path1[256], path2[256];
    if (DosQueryPathInfo(file1,FIL_QUERYFULLNAME,path1,sizeof(path1))) return 1;
    if (DosQueryPathInfo(file2,FIL_QUERYFULLNAME,path2,sizeof(path2))) return 1;
    return sstricmp(path1, path2);
}
#elif defined (__DJGPP__)
#include <dos.h>
static int cmpfnames(char *file1, char *file2)
{
    char *path1 = NULL, *path2 = NULL;
    int result;

  /* _truename() call DOS FN 0x60 (undocumented: return real file name)
     and store malloc'ed string into 2nd parameter.
     Return value is pointer to this string or NULL.
     See c:\djgpp\src\libc\dos\dos\truename.c for details */
    _truename(file1, path1);
    _truename(file2, path2);
    if( path1==NULL && path2==NULL )  /* if both filename is NULL return not eq */
      return = -1;
    result = sstricmp(path1, path2);  /* sstricmp() compare NULL strings also */
    nfree(path1);
    nfree(path2);
    return result;
}
#elif (defined(MSDOS) || defined(__MSDOS__)) && !defined(__DJGPP__)
#include <dos.h>
static int cmpfnames(char *file1, char *file2)
{
    struct REGPACK r;
    char path1[128], path2[128];
    r.r_ds = FP_SEG(file1);
    r.r_si = FP_OFF(file1);
    r.r_es = FP_SEG(path1);
    r.r_di = FP_OFF(path1);
    r.r_ax = 0x6000;
    intr(0x21, &r);
    r.r_ds = FP_SEG(file2);
    r.r_si = FP_OFF(file2);
    r.r_es = FP_SEG(path2);
    r.r_di = FP_OFF(path2);
    r.r_ax = 0x6000;
    intr(0x21, &r);
    return sstricmp(path1, path2);
}
#else /* Unknown OS */
static int cmpfnames(char *file1, char *file2)
{
    return sstricmp(file1, file2);
}
#endif

void checkIncludeLogic(ps_fidoconfig config)
{
    unsigned int i, j;

    for (j=0; j<config->linkCount; j++) {
	if (config->links[j].autoAreaCreateFile==NULL) continue;
	for (i=0; i<cfgNamesCount; i++) {
	    if (cmpfnames(cfgNames[i],config->links[j].autoAreaCreateFile)==0)
		break;
	}
	// if not found include file - return error
	if (i==cfgNamesCount) {
	    printf("AutoAreaCreateFile %s has never been included in config!\n",
		   config->links[j].autoAreaCreateFile);
	    exit(EX_CONFIG);
	}
    }

    for (j=0; j<config->linkCount; j++) {
	if (config->links[j].autoFileCreateFile==NULL) continue;
	for (i=0; i<cfgNamesCount; i++) {
	    if (cmpfnames(cfgNames[i],config->links[j].autoFileCreateFile)==0) break;
	}
	// if not found include file - return error
	if (i==cfgNamesCount) {
	    printf("AutoFileCreateFile %s has never been included in config!\n",
		   config->links[j].autoFileCreateFile);
	    exit(EX_CONFIG);
	}
    }
}

const char* getCurConfName()
{
    return curconfname;
}

long getCurConfPos()
{
    return curconfpos;
}

long get_hcfgPos()
{
    return ftell(hcfg);
}

FILE *get_hcfg()
{
    return hcfg;
}
