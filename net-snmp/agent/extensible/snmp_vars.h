u_char *var_extensible_shell();
u_char *var_extensible_relocatable();
u_char *var_extensible_disk();
u_char *var_extensible_version();
u_char *var_extensible_hp();
u_char *var_extensible_lockd_test();
u_char *var_extensible_loadave();
u_char *var_extensible_proc();
u_char *var_extensible_mem();
u_char *var_extensible_errors();

#ifdef PROCMIBNUM
/* the variable that stores the process watching mib info */
struct variable2 extensible_proc_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_proc, 1, {MIBINDEX}},
  {ERRORNAME, STRING, RONLY, var_extensible_proc, 1, {ERRORNAME}}, 
    {PROCMIN, INTEGER, RONLY, var_extensible_proc, 1, {PROCMIN}}, 
    {PROCMAX, INTEGER, RONLY, var_extensible_proc, 1, {PROCMAX}},
    {PROCCOUNT, INTEGER, RONLY, var_extensible_proc, 1, {PROCCOUNT}},
    {ERRORFLAG, INTEGER, RONLY, var_extensible_proc, 1, {ERRORFLAG}},
    {ERRORMSG, STRING, RONLY, var_extensible_proc, 1, {ERRORMSG}},
  {ERRORFIX, INTEGER, RWRITE, var_extensible_proc, 1, {ERRORFIX }}
};
#endif

#ifdef SHELLMIBNUM
/* the extensible commands variables */
struct variable2 extensible_extensible_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_shell, 1, {MIBINDEX}},
  {ERRORNAME, STRING, RONLY, var_extensible_shell, 1, {ERRORNAME}}, 
    {SHELLCOMMAND, STRING, RONLY, var_extensible_shell, 1, {SHELLCOMMAND}}, 
    {ERRORFLAG, INTEGER, RONLY, var_extensible_shell, 1, {ERRORFLAG}},
    {ERRORMSG, STRING, RONLY, var_extensible_shell, 1, {ERRORMSG}},
  {ERRORFIX, INTEGER, RWRITE, var_extensible_shell, 1, {ERRORFIX }}
};
#endif

#ifdef LOCKDMIBNUM
/* the lockd test variables */
struct variable2 extensible_lockd_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_lockd_test, 1, {MIBINDEX}},
    {ERRORFLAG, INTEGER, RONLY, var_extensible_lockd_test, 1, {ERRORFLAG}},
    {ERRORMSG, STRING, RONLY, var_extensible_lockd_test, 1, {ERRORMSG}}
};
#endif

#ifdef MEMMIBNUM
#ifdef hpux
struct variable2 extensible_mem_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_mem,1,{MIBINDEX}},
  {ERRORNAME, STRING, RONLY, var_extensible_mem, 1, {ERRORNAME }},
  {MEMTOTALSWAP, INTEGER, RONLY, var_extensible_mem, 1, {MEMTOTALSWAP}},
  {MEMUSEDSWAP, INTEGER, RONLY, var_extensible_mem, 1, {MEMUSEDSWAP}},
  {MEMTOTALREAL, INTEGER, RONLY, var_extensible_mem, 1, {MEMTOTALREAL}},
  {MEMUSEDREAL, INTEGER, RONLY, var_extensible_mem, 1, {MEMUSEDREAL}},
  {MEMTOTALSWAPTXT, INTEGER, RONLY, var_extensible_mem, 1, {MEMTOTALSWAPTXT}},
  {MEMUSEDSWAPTXT, INTEGER, RONLY, var_extensible_mem, 1, {MEMUSEDSWAPTXT}},
  {MEMTOTALREALTXT, INTEGER, RONLY, var_extensible_mem, 1, {MEMTOTALREALTXT}},
  {MEMUSEDREALTXT, INTEGER, RONLY, var_extensible_mem, 1, {MEMUSEDREALTXT}},
  {MEMTOTALFREE, INTEGER, RONLY, var_extensible_mem, 1, {MEMTOTALFREE}},
  {ERRORFLAG, INTEGER, RONLY, var_extensible_mem, 1, {ERRORFLAG }},
  {ERRORMSG, STRING, RONLY, var_extensible_mem, 1, {ERRORMSG }}
};
#endif
#endif

#ifdef DISKMIBNUM
#if defined(hpux) || defined(ultrix)
struct variable2 extensible_disk_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_disk, 1, {MIBINDEX}},
  {ERRORNAME, STRING, RONLY, var_extensible_disk, 1, {ERRORNAME}},
  {DISKDEVICE, STRING, RONLY, var_extensible_disk, 1, {DISKDEVICE}},
  {DISKMINIMUM, INTEGER, RONLY, var_extensible_disk, 1, {DISKMINIMUM}},
  {DISKTOTAL, INTEGER, RONLY, var_extensible_disk, 1, {DISKTOTAL}},
  {DISKAVAIL, INTEGER, RONLY, var_extensible_disk, 1, {DISKAVAIL}},
  {DISKUSED, INTEGER, RONLY, var_extensible_disk, 1, {DISKUSED}},
  {DISKPERCENT, INTEGER, RONLY, var_extensible_disk, 1, {DISKPERCENT}},
  {ERRORFLAG, INTEGER, RONLY, var_extensible_disk, 1, {ERRORFLAG }},
  {ERRORMSG, STRING, RONLY, var_extensible_disk, 1, {ERRORMSG }}
};
#endif
#endif

#ifdef VERSIONMIBNUM
struct variable2 extensible_version_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_version, 1, {MIBINDEX}},
  {VERTAG, STRING, RONLY, var_extensible_version, 1, {VERTAG}},
  {VERDATE, STRING, RONLY, var_extensible_version, 1, {VERDATE}},
  {VERCDATE, STRING, RONLY, var_extensible_version, 1, {VERCDATE}},
  {VERIDENT, STRING, RONLY, var_extensible_version, 1, {VERIDENT}},
  {VERCLEARCACHE, INTEGER, RONLY, var_extensible_version, 1, {VERCLEARCACHE}}
};
#endif

#ifdef LOADAVEMIBNUM
struct variable2 extensible_loadave_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_loadave, 1, {MIBINDEX}},
  {ERRORNAME, STRING, RONLY, var_extensible_loadave, 1, {ERRORNAME}},
  {LOADAVE, STRING, RONLY, var_extensible_loadave, 1, {LOADAVE}},
  {LOADMAXVAL, STRING, RONLY, var_extensible_loadave, 1, {LOADMAXVAL}},
    {ERRORFLAG, INTEGER, RONLY, var_extensible_loadave, 1, {ERRORFLAG}},
    {ERRORMSG, STRING, RONLY, var_extensible_loadave, 1, {ERRORMSG}}
};
#endif

#ifdef ERRORMIBNUM
struct variable2 extensible_error_variables[] = {
  {MIBINDEX, INTEGER, RONLY, var_extensible_errors, 1, {MIBINDEX}},
  {ERRORNAME, STRING, RONLY, var_extensible_errors, 1, {ERRORNAME}},
    {ERRORFLAG, INTEGER, RONLY, var_extensible_errors, 1, {ERRORFLAG}},
    {ERRORMSG, STRING, RONLY, var_extensible_errors, 1, {ERRORMSG}}
};
#endif

/* mimics part of the hpux tree */
#ifdef hpux  
struct variable2 extensible_hp_variables[] = {
  {HPCONF, INTEGER, RWRITE, var_extensible_hp, 1, {HPCONF}},
  {HPRECONFIG, INTEGER, RWRITE, var_extensible_hp, 1, {HPRECONFIG}},
  {HPFLAG, INTEGER, RWRITE, var_extensible_hp, 1, {HPFLAG}},
  {HPLOGMASK, INTEGER, RWRITE, var_extensible_hp, 1, {ERRORFLAG}},
  {HPSTATUS, INTEGER, RWRITE, var_extensible_hp, 1, {ERRORMSG}}
};

struct variable2 extensible_hptrap_variables[] = {
  {HPTRAP, IPADDRESS, RWRITE, var_extensible_hp, 1, {HPTRAP }},
};
#endif
