/* $Id$

   This file lists all fidoconfig keywords.

   New keywords can be added to fidoconfig as follows:
   1. Define an ID for the keyword in the following enum. Place the new
      entry at the bottom of the list (or anywhere else, it does not
      really matter).
   2. Define the keyword in the parseline_tokens array which comes below
      the enum. Attention: The entries of this array must be sorted
      alphabetically. If they are not, fidoconf will still work, but a
      warning message will be printed at program runtime and performance will
      be slow.  This means:  Run tparser.exe to make sure you have inserted the
      keyword at the right place.
   3. In fidoconf.h, add the necessary variables to the fidoconfig structure.
      Note: You may only do this in the current branch. When you are
      inside a stable branch, the layout of the fidoconfig structure
      is FROZEN. You may only rename spare items (if there are any)
      for your purposes, but you must never add or remove entries of this
      structure inside a stable CVS branch!
   4. In line.c, function parseLine, add a "case ID_..." clause in the big
      switch construct for your new keyword, and fill the new entries
      of the fidoconfig structure.
   5. Update tparser.c to print the current value of your new configuration
      option.
   6. VERY IMPORTANT: Add docs about your keyword to doc/proposal.texi.
      If you dont want to write docs (because you don't know texinfo, or
      because you speak English very bad) AT LEAST add a line to TODO.C
      of the sort "- someone please write docs for keyword XXX", so that
      at least we know what keywords are not yet documented.
*/

enum {
    ID_VERSION = 0,
    ID_NAME,
    ID_LOCATION,
    ID_SYSOP,
    ID_ADDRESS,
    ID_INBOUND,
    ID_PROTINBOUND,
    ID_LISTINBOUND,
    ID_LOCALINBOUND,
    ID_TEMPINBOUND,
    ID_OUTBOUND,
    ID_TICOUTBOUND,
    ID_PUBLIC,
    ID_LOGFILEDIR,
    ID_DUPEHISTORYDIR,
    ID_NODELISTDIR,
    ID_FILEAREABASEDIR,
    ID_PASSFILEAREADIR,
    ID_BUSYFILEDIR,
    ID_MSGBASEDIR,
    ID_LINKMSGBASEDIR,
    ID_MAGIC,
    ID_SEMADIR,
    ID_BADFILESDIR,
    ID_NETAREA,
    ID_NETMAILAREA,
    ID_DUPEAREA,
    ID_BADAREA,
    ID_ECHOAREA,
    ID_FILEAREA,
    ID_BBSAREA,
    ID_LOCALAREA,
    ID_REMAP,
    ID_LINK,
    ID_PASSWORD,
    ID_AKA,
    ID_OURAKA,
    ID_AUTOAREACREATE,
    ID_AUTOFILECREATE,
    ID_FORWARDREQUESTS,
    ID_FORWARDFILEREQUESTS,
    ID_DENYFWDREQACCESS,
    ID_FORWARDPKTS,
    ID_ALLOWEMPTYPKTPWD,
    ID_PACKNETMAIL,
    ID_ALLOWPKTADDRDIFFER,
    ID_AUTOAREACREATEDEFAULTS,
    ID_AUTOFILECREATEDEFAULTS,
    ID_AREAFIX,
    ID_FILEFIX,
    ID_PAUSE,
    ID_NOTIC,
    ID_DELNOTRECIEVEDTIC,
    ID_DELNOTRECEIVEDTIC,
    ID_ADVANCEDAREAFIX,
    ID_AUTOPAUSE,
    ID_REMOTEROBOTNAME,
    ID_REMOTEFILEROBOTNAME,
    ID_FORWARDAREAPRIORITY,
    ID_FORWARDFILEPRIORITY,
    ID_DENYUNCONDFWDREQACCESS,
    ID_EXPORT,
    ID_IMPORT,
    ID_MANDATORY,
    ID_MANUAL,
    ID_OPTGRP,
    ID_FORWARDREQUESTMASK,
    ID_DENYFWDMASK,
    ID_LEVEL,
    ID_AREAFIXECHOLIMIT,
    ID_ARCMAILSIZE,
    ID_PKTSIZE,
    ID_MAXUNPACKEDNETMAIL,
    ID_PKTPWD,
    ID_TICPWD,
    ID_AREAFIXPWD,
    ID_FILEFIXPWD,
    ID_BBSPWD,
    ID_SESSIONPWD,
    ID_HANDLE,
    ID_EMAIL,
    ID_EMAILFROM,
    ID_EMAILSUBJ,
    ID_EMAILENCODING,
    ID_ECHOMAILFLAVOUR,
    ID_FILEECHOFLAVOUR,
    ID_ROUTE,
    ID_ROUTEMAIL,
    ID_ROUTEFILE,
    ID_PACK,
    ID_UNPACK,
    ID_PACKER,
    ID_INTAB,
    ID_OUTTAB,
    ID_AREAFIXHELP,
    ID_FILEFIXHELP,
    ID_FORWARDREQUESTFILE,
    ID_DENYFWDFILE,
    ID_FORWARDFILEREQUESTFILE,
    ID_AUTOAREACREATEFILE,
    ID_AUTOFILECREATEFILE,
    ID_LINKBUNDLENAMESTYLE,
    ID_ECHOTOSSLOG,
    ID_IMPORTLOG,
    ID_STATLOG,
    ID_LINKWITHIMPORTLOG,
    ID_KLUDGEAREANETMAIL,
    ID_FILEAREASLOG,
    ID_FILENEWAREASLOG,
    ID_LONGNAMELIST,
    ID_FILEARCLIST,
    ID_FILEPASSLIST,
    ID_FILEDUPELIST,
    ID_MSGIDFILE,
    ID_LOGLEVELS,
    ID_SCREENLOGLEVELS,
    ID_ACCESSGRP,
    ID_LINKGRP,
    ID_CARBONTO,
    ID_CARBONFROM,
    ID_CARBONADDR,
    ID_CARBONKLUDGE,
    ID_CARBONSUBJ,
    ID_CARBONTEXT,
    ID_CARBONCOPY,
    ID_CARBONMOVE,
    ID_CARBONEXTERN,
    ID_NETMAILEXTERN,
    ID_CARBONDELETE,
    ID_CARBONREASON,
    ID_EXCLUDEPASSTHROUGHCARBON,
    ID_LOCKFILE,
    ID_TEMPOUTBOUND,
    ID_AREAFIXFROMPKT,
    ID_AREAFIXKILLREPORTS,
    ID_AREAFIXKILLREQUESTS,
    ID_FILEFIXKILLREPORTS,
    ID_FILEFIXKILLREQUESTS,
    ID_CREATEDIRS,
    ID_LONGDIRNAMES,
    ID_SPLITDIRS,
    ID_ADDDLC,
    ID_FILESINGLEDESCLINE,
    ID_FILECHECKDEST,
    ID_PUBLICGROUP,
    ID_LOGECHOTOSCREEN,
    ID_SEPARATEBUNDLES,
    ID_CARBONANDQUIT,
    ID_CARBONKEEPSB,
    ID_CARBONOUT,
    ID_IGNORECAPWORD,
    ID_NOPROCESSBUNDLES,
    ID_REPORTTO,
    ID_EXECONFILE,
    ID_DEFARCMAILSIZE,
    ID_AREAFIXMSGSIZE,
    ID_AFTERUNPACK,
    ID_BEFOREPACK,
    ID_PROCESSPKT,
    ID_AREAFIXSPLITSTR,
    ID_AREAFIXORIGIN,
    ID_ROBOTSAREA,
    ID_FILEDESCPOS,
    ID_DLCDIGITS,
    ID_FILEMAXDUPEAGE,
    ID_FILEFILEUMASK,
    ID_FILEDIRUMASK,
    ID_ORIGININANNOUNCE,
    ID_MAXTICLINELENGTH,
    ID_FILELOCALPWD,
    ID_FILELDESCSTRING,
    ID_SAVETIC,
    ID_AREASMAXDUPEAGE,
    ID_DUPEBASETYPE,
    ID_FIDOUSERLIST,
    ID_NODELIST,
    ID_DIFFUPDATE,
    ID_FULLUPDATE,
    ID_DEFAULTZONE,
    ID_NODELISTFORMAT,
    ID_LOGOWNER,
    ID_LOGPERM,
    ID_LINKDEFAULTS,
    ID_CREATEAREASCASE,
    ID_AREASFILENAMECASE,
    ID_CONVERTLONGNAMES,
    ID_CONVERTSHORTNAMES,
    ID_DISABLEPID,
    ID_DISABLETID,
    ID_TOSSINGEXT,
    ID_SETCONSOLETITLE,
    ID_ADDTOSEEN,
    ID_IGNORESEEN,
    ID_TEARLINE,
    ID_ORIGIN,
    ID_BUNDLENAMESTYLE,
    ID_KEEPTRSMAIL,
    ID_KEEPTRSFILES,
    ID_FILELIST,
    ID_CREATEFWDNONPASS,
    ID_AUTOPASSIVE,
    ID_NETMAILFLAG,
    ID_AUTOAREACREATEFLAG,
    ID_MINDISKFREESPACE,
    ID_ECHOAREADEFAULT,
    ID_FILEAREADEFAULT,
    ID_CARBONRULE,
    ID_CARBONFROMAREA,
    ID_CARBONGROUPS,
    ID_AUTOAREACREATESUBDIRS,
    ID_AUTOFILECREATESUBDIRS,
    ID_ADVISORYLOCK,
    ID_AREAFIXNAMES,
    ID_REQIDXDIR,
    ID_SYSLOG_FACILITY,
    ID_FILEFIXFSC87SUBSET,
    ID_FILEBOX,
    ID_FILEBOXALWAYS,
    ID_FILEBOXESDIR,
    ID_CARBONEXCLUDEFWDFROM,
    ID_HPTPERLFILE,
    ID_READONLY,
    ID_WRITEONLY,
    ID_AREAFIXQUERYREPORTS,
    ID_NOTVALIDFILENAMECHARS,
    ID_ARCNETMAIL,
    ID_RULESDIR,
    ID_NORULES,
    ID_AREAFIXQUEUEFILE,
    ID_FILEDESCNAME,
    ID_FORWARDREQUESTTIMEOUT,
    ID_IDLEPASSTHRUTIMEOUT,
    ID_KILLEDREQUESTTIMEOUT,
    ID_PACKNETMAILONSCAN,
    ID_UUEECHOGROUP,
    ID_TICKERPACKTOBOX,
    ID_LINKFILEBASEDIR,
    ID_SENDMAILCMD,
    ID_TEMPDIR,
    ID_ANNOUNCESPOOL,
    ID_ANNAREATAG,
    ID_ANNINCLUDE,
    ID_ANNEXCLUDE,
    ID_ANNTO,
    ID_ANNFROM,
    ID_ANNSUBJ,
    ID_ANNADRTO,
    ID_ANNADRFROM,
    ID_ANNORIGIN,
    ID_ANNMESSFLAGS,
    ID_ANNFILEORIGIN,
    ID_ANNFILERFROM,
    ID_FILEAREACREATEPERMS,
    ID_NEWAREAREFUSEFILE,
    ID_REDUCEDSEENBY,
    ID_AREAFIXFROMNAME,
    ID_FILEFIXFROMNAME
};

/* The following list must be sorted alphabetically, and keywords must be all
   in lower case!!!. Each keyword must have a unique ID, but the ID's must
   not be sorted in any form. define the ID's in the enum above to make sure
   they are unique. */

token_t parseline_tokens[] =
{
    { "accessgrp", ID_ACCESSGRP },
    { "adddlc", ID_ADDDLC },
    { "address", ID_ADDRESS },
    { "addtoseen", ID_ADDTOSEEN },
    { "advancedareafix", ID_ADVANCEDAREAFIX },
    { "advisorylock", ID_ADVISORYLOCK },
    { "afterunpack", ID_AFTERUNPACK },
    { "aka", ID_AKA },
    { "allowemptypktpwd", ID_ALLOWEMPTYPKTPWD },
    { "allowpktaddrdiffer", ID_ALLOWPKTADDRDIFFER },
    /*  htick announcer */
    { "annadrfrom", ID_ANNADRFROM},
    { "annadrto", ID_ANNADRTO},
    { "annareatag", ID_ANNAREATAG},
    { "annexclude", ID_ANNEXCLUDE},
    { "annfileorigin", ID_ANNFILEORIGIN},
    { "annfilerfrom", ID_ANNFILERFROM},
    { "annfrom", ID_ANNFROM},
    { "anninclude", ID_ANNINCLUDE},
    { "annmessflags", ID_ANNMESSFLAGS},
    { "annorigin", ID_ANNORIGIN},
    { "announcespool", ID_ANNOUNCESPOOL},
    { "annsubj", ID_ANNSUBJ},
    { "annto", ID_ANNTO},

    { "arcmailsize", ID_ARCMAILSIZE },
    { "arcnetmail", ID_ARCNETMAIL },
    { "areafix", ID_AREAFIX },
    { "areafixecholimit", ID_AREAFIXECHOLIMIT },
    { "areafixfromname", ID_AREAFIXFROMNAME },
    { "filefixfromname", ID_FILEFIXFROMNAME },
    { "areafixfrompkt", ID_AREAFIXFROMPKT },
    { "areafixhelp", ID_AREAFIXHELP },
    { "areafixkillreports", ID_AREAFIXKILLREPORTS },
    { "areafixkillrequests", ID_AREAFIXKILLREQUESTS },
    { "areafixmsgsize", ID_AREAFIXMSGSIZE },
    { "areafixnames", ID_AREAFIXNAMES },
    { "areafixorigin", ID_AREAFIXORIGIN },
    { "areafixpwd", ID_AREAFIXPWD },
    { "areafixqueryreports", ID_AREAFIXQUERYREPORTS },
    { "areafixqueuefile",    ID_AREAFIXQUEUEFILE },
    { "areafixsplitstr", ID_AREAFIXSPLITSTR },
    { "areasfilenamecase", ID_AREASFILENAMECASE },
    { "areasmaxdupeage", ID_AREASMAXDUPEAGE },
    { "autoareacreate", ID_AUTOAREACREATE },
    { "autoareacreatedefaults", ID_AUTOAREACREATEDEFAULTS },
    { "autoareacreatefile", ID_AUTOAREACREATEFILE },
    { "autoareacreateflag", ID_AUTOAREACREATEFLAG },
    { "autoareacreatesubdirs", ID_AUTOAREACREATESUBDIRS },
    { "autofilecreate", ID_AUTOFILECREATE },
    { "autofilecreatedefaults", ID_AUTOFILECREATEDEFAULTS },
    { "autofilecreatefile", ID_AUTOFILECREATEFILE },
    { "autofilecreatesubdirs", ID_AUTOFILECREATESUBDIRS },
    { "autopassive", ID_AUTOPASSIVE },
    { "autopause", ID_AUTOPAUSE },
    { "badarea", ID_BADAREA },
    { "badfilesdir", ID_BADFILESDIR },
    { "bbsarea", ID_BBSAREA },
    { "bbspwd", ID_BBSPWD },
    { "beforepack", ID_BEFOREPACK },
    { "bundlenamestyle", ID_BUNDLENAMESTYLE },
    { "busyfiledir", ID_BUSYFILEDIR },
    { "carbonaddr", ID_CARBONADDR },
    { "carbonandquit", ID_CARBONANDQUIT },
    { "carboncopy", ID_CARBONCOPY },
    { "carbondelete", ID_CARBONDELETE },
    { "carbonexcludefwdfrom", ID_CARBONEXCLUDEFWDFROM },
    { "carbonextern", ID_CARBONEXTERN },
    { "carbonfrom", ID_CARBONFROM },
    { "carbonfromarea", ID_CARBONFROMAREA },
    { "carbongroups", ID_CARBONGROUPS },
    { "carbonkeepsb", ID_CARBONKEEPSB },
    { "carbonkludge", ID_CARBONKLUDGE },
    { "carbonmove", ID_CARBONMOVE },
    { "carbonout", ID_CARBONOUT },
    { "carbonreason", ID_CARBONREASON },
    { "carbonrule", ID_CARBONRULE },
    { "carbonsubj", ID_CARBONSUBJ },
    { "carbontext" , ID_CARBONTEXT },
    { "carbonto", ID_CARBONTO },
    { "convertlongnames", ID_CONVERTLONGNAMES },
    { "convertshortnames", ID_CONVERTSHORTNAMES },
    { "createareascase", ID_CREATEAREASCASE },
    { "createdirs", ID_CREATEDIRS },
    { "createfwdnonpass", ID_CREATEFWDNONPASS },
    { "defarcmailsize", ID_DEFARCMAILSIZE },
    { "defaultzone", ID_DEFAULTZONE },
    { "delnotreceivedtic", ID_DELNOTRECEIVEDTIC },
    { "delnotrecievedtic", ID_DELNOTRECIEVEDTIC }, /* wrong spelling - supported for downwards compatiblity */
    { "denyfwdfile", ID_DENYFWDFILE },
    { "denyfwdmask", ID_DENYFWDMASK },
    { "denyfwdreqaccess", ID_DENYFWDREQACCESS },
    { "denyuncondfwdreqaccess", ID_DENYUNCONDFWDREQACCESS },
    { "diffupdate", ID_DIFFUPDATE },
    { "disablepid", ID_DISABLEPID },
    { "disabletid", ID_DISABLETID },
    { "dlcdigits", ID_DLCDIGITS },
    { "dupearea", ID_DUPEAREA },
    { "dupebasetype", ID_DUPEBASETYPE },
    { "dupehistorydir", ID_DUPEHISTORYDIR },
    { "echoarea", ID_ECHOAREA },
    { "echoareadefaults", ID_ECHOAREADEFAULT },
    { "echomailflavour", ID_ECHOMAILFLAVOUR },
    { "echotosslog", ID_ECHOTOSSLOG },
    { "email", ID_EMAIL },
    { "emailencoding", ID_EMAILENCODING },
    { "emailfrom", ID_EMAILFROM },
    { "emailsubj", ID_EMAILSUBJ },
    { "excludepassthroughcarbon", ID_EXCLUDEPASSTHROUGHCARBON },
    { "execonfile", ID_EXECONFILE },
    { "export", ID_EXPORT },
    { "fidouserlist", ID_FIDOUSERLIST },
    { "filearclist", ID_FILEARCLIST },
    { "filearea", ID_FILEAREA },
    { "fileareabasedir", ID_FILEAREABASEDIR },
    { "fileareacreateperms", ID_FILEAREACREATEPERMS },
    { "fileareadefaults", ID_FILEAREADEFAULT },
    { "fileareaslog", ID_FILEAREASLOG },
    { "filebox", ID_FILEBOX },
    { "fileboxalways", ID_FILEBOXALWAYS },
    { "fileboxesdir", ID_FILEBOXESDIR },
    { "filecheckdest", ID_FILECHECKDEST },
    { "filedescname", ID_FILEDESCNAME },
    { "filedescpos", ID_FILEDESCPOS },
    { "filedirumask", ID_FILEDIRUMASK },
    { "filedupelist", ID_FILEDUPELIST },
    { "fileechoflavour", ID_FILEECHOFLAVOUR },
    { "filefileumask", ID_FILEFILEUMASK },
    { "filefix", ID_FILEFIX },
    { "filefixfsc87subset", ID_FILEFIXFSC87SUBSET },
    { "filefixhelp", ID_FILEFIXHELP },
    { "filefixkillreports", ID_FILEFIXKILLREPORTS },
    { "filefixkillrequests", ID_FILEFIXKILLREQUESTS },
    { "filefixpwd", ID_FILEFIXPWD },
    { "fileldescstring", ID_FILELDESCSTRING },
    { "filelist", ID_FILELIST },
    { "filelocalpwd", ID_FILELOCALPWD },
    { "filemaxdupeage", ID_FILEMAXDUPEAGE },
    { "filenewareaslog", ID_FILENEWAREASLOG },
    { "filepasslist", ID_FILEPASSLIST },
    { "filesingledescline", ID_FILESINGLEDESCLINE },
    { "forwardareapriority", ID_FORWARDAREAPRIORITY },
    { "forwardfilepriority", ID_FORWARDFILEPRIORITY },
    { "forwardfilerequestfile", ID_FORWARDFILEREQUESTFILE },
    { "forwardfilerequests", ID_FORWARDFILEREQUESTS },
    { "forwardpkts", ID_FORWARDPKTS },
    { "forwardrequestfile", ID_FORWARDREQUESTFILE },
    { "forwardrequestmask", ID_FORWARDREQUESTMASK },
    { "forwardrequests", ID_FORWARDREQUESTS },
    { "forwardrequesttimeout", ID_FORWARDREQUESTTIMEOUT },
    { "fullupdate", ID_FULLUPDATE },
    { "handle", ID_HANDLE },
    { "hptperlfile", ID_HPTPERLFILE },
    { "idlepassthrutimeout" , ID_IDLEPASSTHRUTIMEOUT },
    { "ignorecapword", ID_IGNORECAPWORD },
    { "ignoreseen", ID_IGNORESEEN },
    { "import", ID_IMPORT },
    { "importlog", ID_IMPORTLOG },
    { "inbound", ID_INBOUND },
    { "intab", ID_INTAB },
    { "keeptrsfiles", ID_KEEPTRSFILES },
    { "keeptrsmail", ID_KEEPTRSMAIL },
    { "killedrequesttimeout", ID_KILLEDREQUESTTIMEOUT },
    { "kludgeareanetmail", ID_KLUDGEAREANETMAIL },
    { "level", ID_LEVEL },
    { "link", ID_LINK },
    { "linkbundlenamestyle", ID_LINKBUNDLENAMESTYLE },
    { "linkdefaults", ID_LINKDEFAULTS },
    { "linkfilebasedir", ID_LINKFILEBASEDIR },
    { "linkgrp", ID_LINKGRP },
    { "linkmsgbasedir", ID_LINKMSGBASEDIR },
    { "linkwithimportlog", ID_LINKWITHIMPORTLOG },
    { "listinbound", ID_LISTINBOUND },
    { "localarea", ID_LOCALAREA },
    { "localinbound", ID_LOCALINBOUND },
    { "location", ID_LOCATION },
    { "lockfile", ID_LOCKFILE },
    { "logechotoscreen", ID_LOGECHOTOSCREEN },
    { "logfiledir", ID_LOGFILEDIR },
    { "loglevels", ID_LOGLEVELS },
    { "logowner", ID_LOGOWNER },
    { "logperm", ID_LOGPERM },
    { "longdirnames", ID_LONGDIRNAMES },
    { "longnamelist", ID_LONGNAMELIST },
    { "magic", ID_MAGIC },
    { "mandatory", ID_MANDATORY },
    { "manual", ID_MANUAL },
    { "maxticlinelength", ID_MAXTICLINELENGTH },
    { "maxunpackednetmail", ID_MAXUNPACKEDNETMAIL },
    { "mindiskfreespace", ID_MINDISKFREESPACE },
    { "msgbasedir", ID_MSGBASEDIR },
    { "msgidfile", ID_MSGIDFILE },
    { "name", ID_NAME },
    { "netarea", ID_NETAREA },
    { "netmailarea", ID_NETMAILAREA },
    { "netmailextern", ID_NETMAILEXTERN },
    { "netmailflag", ID_NETMAILFLAG },
    { "newarearefusefile", ID_NEWAREAREFUSEFILE },
    { "nodelist", ID_NODELIST },
    { "nodelistdir", ID_NODELISTDIR },
    { "nodelistformat", ID_NODELISTFORMAT },
    { "noprocessbundles", ID_NOPROCESSBUNDLES },
    { "norules", ID_NORULES },
    { "notic", ID_NOTIC },
    { "notvalidfilenamechars", ID_NOTVALIDFILENAMECHARS },
    { "optgrp", ID_OPTGRP },
    { "origin", ID_ORIGIN },
    { "origininannounce", ID_ORIGININANNOUNCE },
    { "ouraka", ID_OURAKA },
    { "outbound", ID_OUTBOUND },
    { "outtab", ID_OUTTAB },
    { "pack", ID_PACK },
    { "packer", ID_PACKER },
    { "packnetmail", ID_PACKNETMAIL },
    { "packnetmailonscan", ID_PACKNETMAILONSCAN },
    { "passfileareadir", ID_PASSFILEAREADIR },
    { "password", ID_PASSWORD },
    { "pause", ID_PAUSE },
    { "pktpwd", ID_PKTPWD },
    { "pktsize", ID_PKTSIZE },
    { "processpkt", ID_PROCESSPKT },
    { "protinbound", ID_PROTINBOUND },
    { "public", ID_PUBLIC },
    { "publicgroup", ID_PUBLICGROUP },
    { "readonly", ID_READONLY },
    { "reducedseenby", ID_REDUCEDSEENBY },
    { "remap", ID_REMAP },
    { "remotefilerobotname", ID_REMOTEFILEROBOTNAME },
    { "remoterobotname", ID_REMOTEROBOTNAME },
    { "reportto", ID_REPORTTO },
    { "reqidxdir", ID_REQIDXDIR },
    { "robotsarea", ID_ROBOTSAREA },
    { "route", ID_ROUTE },
    { "routefile", ID_ROUTEFILE },
    { "routemail", ID_ROUTEMAIL },
    { "rulesdir", ID_RULESDIR },
    { "savetic", ID_SAVETIC },
    { "screenloglevels", ID_SCREENLOGLEVELS },
    { "semadir", ID_SEMADIR },
    { "sendmailcmd", ID_SENDMAILCMD },
    { "separatebundles", ID_SEPARATEBUNDLES },
    { "sessionpwd", ID_SESSIONPWD },
    { "setconsoletitle", ID_SETCONSOLETITLE },
    { "splitdirs", ID_SPLITDIRS },
    { "statlog", ID_STATLOG },
    { "syslogfacility", ID_SYSLOG_FACILITY },
    { "sysop", ID_SYSOP },
    { "tearline", ID_TEARLINE },
    { "tempdir", ID_TEMPDIR },
    { "tempinbound", ID_TEMPINBOUND },
    { "tempoutbound", ID_TEMPOUTBOUND },
    { "tickerpacktobox", ID_TICKERPACKTOBOX },
    { "ticoutbound", ID_TICOUTBOUND },
    { "ticpwd", ID_TICPWD },
    { "tossingext", ID_TOSSINGEXT },
    { "unpack", ID_UNPACK },
    { "uueechogroup", ID_UUEECHOGROUP },
    { "version", ID_VERSION },
    { "writeonly", ID_WRITEONLY },
    { NULL, -1 }
};
