/*
 * Copyright (c) 2015 Tim Savannah <kata198@gmail.com> under GPLv3.
 * See LICENSE file for more information.
 *
 *
 * Code Style:
 * vim: ts=8:sw=8:expandtab
 * two-space indents between preprocessor conditionals. All binaries come from mutations on this one file. bwahahaha!!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>


#if defined(__CYGWIN__)
  #warning cygwin detected, if you get failures to link below for g_hash whatever, add -D NO_GLIB to the CFLAGS in the Makefile.
#endif

// Uncomment this line to build without glib.
//#define NO_GLIB

#ifdef ALL_PROCS
  #ifndef NO_GLIB
    #include <glib.h>
    GHashTable *pwdInfo;
  #else
    #warning COMPILING WITHOUT GLIB SUPPORT. ps* variants will be slower
  #endif
#endif

#ifdef _LINUX_LIMITS_H
  #undef _LINUX_LIMITS_H
  #include <linux/limits.h>
#endif

#ifndef ARG_MAX
  #define ARG_MAX 65535
#endif

// This is extra size to account for null terminators and command name in addition to the longest argv possible. If you need to use this on a system without a lot of memory for some reason, adjust this to a smaller value and define ARG_MAX to be something smaller.
#define EXTRA_ARG_BUFFER 2000

char *myUidStr;
unsigned int myUidStrLen;
unsigned int myUid;

volatile const char *author = "Created by Tim Savannah <kata198@gmail.com>. I love you all so much.";
volatile const char *version = "2.0";

#ifdef SHOW_THREADS
  #define THREADS_USE_TREE // For now, default to always showing tree format in threads mode. Comment out this line to get a less-pleasing view.
#endif

#ifdef QUOTE_ARGS
  char *escapeQuotes(char *input)
  {
          unsigned int len;
          char *ret, *retPtr;
          char cur;
  
          len = strlen(input);
  
          retPtr = ret = malloc( sizeof(char) * len * 2 + 1); // Maximum length
  
          while( (cur = *input) != 0)
          {
                  if(cur == '\"' || cur == '$')
                  {
                          *retPtr = '\\';
                          retPtr++;
                  }
                  *retPtr = cur;
                  retPtr++;
                  input++;
          }
          *retPtr = '\0';
  
          return ret;
  }
#endif

#if 0
  unsigned int getProcessOwner(char *pidStr)
  {
          FILE *statusFile;
          static char *statusFilename = NULL;
          static char *buffer = NULL;
  
          if(statusFilename == NULL)
          {
                  statusFilename = malloc(32);
                  buffer = malloc(256);
          }
          
          sprintf(statusFilename, "/proc/%s/status", pidStr);
          statusFile = fopen(statusFilename, "r");
          if(!statusFile)
                  return 65535;
  
          fread(buffer, 1, 256, statusFile);
  
          fclose(statusFile);
  
          register char *uidPortion;
          register unsigned int i;
  
          uidPortion = strstr(buffer, "\nUid:\t")+6;
  
          #ifndef ALL_PROCS
          // Optimization if we are only doing myps, we can check str here.
          if(uidPortion[myUidStrLen] != '\t')
                  return 65535;
  
  
          for(i=0; i < myUidStrLen; i++)
          {
                  if (myUidStr[i]        != uidPortion[i])
                          return 65535;
          }
          return myUid;
          #else
          // Here we actually need to care about the real return
          
          for(i=0; uidPortion[i] != '\t'; i++);
  
          uidPortion[i] = '\0';
          return atoi(uidPortion);
          #endif
  }
#endif
/* Super optimization: The majority of execution time was the above function
 * Turns out we can just stat the process director. 
 * The owner of the proc directory is process owner.
 */
unsigned int getProcessOwner(char *pidStr)
{
        static struct stat info;
        static char *path = NULL;
        if(path == NULL)
        {
                path = malloc(128);
                strcpy(path, "/proc/");
        }
        path[6] = '\0';
        strcat(path, pidStr);

        if( stat(path, &info) == -1 )
                return 65535;

        return info.st_uid;
}


void printCmdLineStr(char *pidStr 
#ifdef ALL_PROCS
                     ,unsigned int ownerUid 
#endif
#ifdef SHOW_THREADS
                     ,char *parentPidStr
#endif
)
{
        FILE *cmdlineFile;
        static char cmdlineFilename[128];
        static char *buffer = NULL;

        unsigned int bufferLen = 0;
        #ifdef QUOTE_ARGS
          char *tmpEscaped;
        #endif
        
        #ifdef ALL_PROCS
          char *pwName;
        #endif

        if(buffer == NULL)
        {
                buffer = malloc(ARG_MAX + EXTRA_ARG_BUFFER);
        }
        
        #ifndef SHOW_THREADS
          sprintf(cmdlineFilename, "/proc/%s/cmdline", pidStr);
        #else
          if(parentPidStr == NULL)
          {
                  sprintf(cmdlineFilename, "/proc/%s/cmdline", pidStr);
          }
          else
          {
                  sprintf(cmdlineFilename, "/proc/%s/task/%s/cmdline", parentPidStr, pidStr);
          }
        #endif
                
        cmdlineFile = fopen(cmdlineFilename, "r");
        if(!cmdlineFile)
                return;

        bufferLen = fread(buffer, 1, ARG_MAX + (EXTRA_ARG_BUFFER - 1), cmdlineFile);
        fclose(cmdlineFile);

        if(bufferLen == 0)
                return; // No cmdline, kthread and the like

        buffer[bufferLen] = '\0';

        #ifdef SHOW_THREADS
          if(parentPidStr != NULL)
                  putchar('\t');
        #endif

        #ifdef ALL_PROCS
          #ifndef NO_GLIB
            pwName = g_hash_table_lookup(pwdInfo, GINT_TO_POINTER(ownerUid));
            if(pwName == NULL)
            {
          #endif
                  struct passwd *pwinfo = getpwuid(ownerUid);
                  if(pwinfo == NULL)
                          return; // No longer active process
          #ifndef NO_GLIB
                  pwName = malloc(strlen(pwinfo->pw_name)+1);
                  strcpy(pwName, pwinfo->pw_name);
                  g_hash_table_insert(pwdInfo, GINT_TO_POINTER(ownerUid), pwName);
          }
          #else
                  pwName = pwinfo->pw_name;
          #endif


          printf("%8s %10s\t%s", pidStr, pwName, buffer);
        #else
          printf("%8s\t%s", pidStr, buffer);
        #endif

        #ifdef CMD_ONLY
          // Skip argument block in command only mode
          putchar('\n');
          if(0)
          {
        #endif

        register char *ptr = buffer;
        while(*(++ptr));
        if( (ptr - buffer) >= bufferLen + 1)
        {
                printf("\n");
                return;
        }
        ptr++;

        while( (ptr - buffer) < bufferLen )
        {
                #ifdef QUOTE_ARGS
                  tmpEscaped = escapeQuotes(ptr);
                  printf(" \"%s\"", tmpEscaped);
                  free(tmpEscaped);
                #else
                  printf(" %s", ptr);
                #endif
                while(*(++ptr));
                ptr++;
        }
        putchar('\n');
        #ifdef CMD_ONLY
        }
        #endif

        #ifdef SHOW_THREADS
          if(parentPidStr == NULL)
          {
                  DIR *taskDir;
                  struct dirent *dirInfo;
                  char *threadPid;
                  static char *parentDir = NULL;
                  #ifdef THREADS_USE_TREE
                    char descended = 0;
                  #endif
                  char hadThread = 0;

                  if(parentDir == NULL)
                        parentDir = malloc(128);

                  sprintf(parentDir, "/proc/%s/task", pidStr);

                  taskDir = opendir(parentDir);
                  while( dirInfo = readdir(taskDir) )
                  {
                          threadPid = dirInfo->d_name;
                          if(!isdigit(threadPid[0]) || strcmp(threadPid, pidStr) == 0)
                                  continue;
                          #ifdef THREADS_USE_TREE
                            if(descended == 0)
                            {
                                  printf("     |\n     \\ ____\n");
                                  descended = 1;
                            }
                            hadThread = 1;
                          #endif
                          printf("\t%8s\tThread [%2u] ( %s )\n", threadPid, descended, buffer);

                          #ifdef THREADS_USE_TREE
                            descended++;
                          #endif

                          #if 0
                          printCmdLineStr(threadPid
                          #ifdef ALL_PROCS
                                                          ,ownerUid
                          #endif
                          #ifdef SHOW_THREADS
                                                          ,pidStr
                          #endif
                          );
                          #endif
                  }
                  #ifdef THREADS_USE_TREE
                    if(hadThread == 1)
                          putchar('\n');
                  #endif

          }
        #endif

}

int main(int argc, char* argv[])
{
        unsigned int myPid, processedOwner;

        DIR *procDir;
        char *pid;
        char *myPidStr;
        struct dirent *dirInfo;

        #ifndef ALL_PROCS
          if(argc == 1)
          {
                  myUid = getuid();
          }
          else if(argc == 2)
          {
                  struct passwd *pwdReq;
                  pwdReq = getpwnam(argv[1]);
                  if(pwdReq == NULL)
                  {
                        fprintf(stderr, "Cannot resolve username: \"%s\"\n", argv[1]);
                        exit(127);
                  }
                  myUid = pwdReq->pw_uid;
          }
          else
          {
                  fprintf(stderr, "Too many arguments.\n");
                  exit(127);
          }
        #endif
              
        myUidStr = malloc(8);
        myPidStr = malloc(8);
        myPid = getpid();

        sprintf(myUidStr, "%u", myUid);
        sprintf(myPidStr, "%u", myPid);
        myUidStrLen = strlen(myUidStr);
        #ifdef ALL_PROCS
          #ifndef NO_GLIB
            pwdInfo = g_hash_table_new(NULL, NULL);
          #endif
        #endif

        procDir = opendir("/proc");
        while( dirInfo = readdir(procDir) )
        {
                pid = dirInfo->d_name;
                if(!isdigit(pid[0]) || strcmp(myPidStr, pid) == 0)
                        continue;
                processedOwner = getProcessOwner(pid);
                #ifndef ALL_PROCS
                  if(processedOwner == myUid)
                #endif
                printCmdLineStr(pid
                #ifdef ALL_PROCS
                                ,processedOwner
                #endif
                #ifdef SHOW_THREADS
                                ,NULL
                #endif
                );
        }

}

// vim: ts=8:sw=8:expandtab
