/*
 * Copyright (c) 2015, 2016 Tim Savannah <kata198@gmail.com> under GPLv3.
 * See LICENSE file for more information.
 *
 *
 * Code Style:
 * vim: ts=8:sw=8:st=8:expandtab
 * two-space indents between preprocessor conditionals. All binaries come from mutations on this one file. bwahahaha!!
 */

/* vim: set ts=8 sw=8 st=8 expandtab : */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>


#ifdef ALL_PROCS

        /* Optimization IMPL:
 *
 *              Special Linked lists, tuned to the application of looking up uint (uid) -> char* (username)
 *
 *              Broken up into 10 linked lists, one for each ending digit of the uid. This prevents systems running many users
 *              from having long chains of lookups.
 *
 *              Since procs with children tend to have close pids, we keep track of the index of a "hot" last key (lastMatch/lastMatchKey).
 *               On a test system with many users, this had a 60% hit rate.
 */


        /* The number of linked lists, set this to 0 to disable last-digit hashing */
        #define LL_NUM_LISTS 10


        /* First element of empty list has "next" set to (void *1), which is a cheap check, and prevents additional assignments */
        #define _EMPTY_LL_MARKER (void *)1
        #define IS_EMPTY_LL(myll) myll->next == _EMPTY_LL_MARKER

        typedef struct {
                unsigned int key;
                char *value;
                void *next;
        }Myps2LinkedList;


        /* pwdInfo - The root pointer to LL_NUM_LISTS linked lists */
        static Myps2LinkedList *pwdInfo;
        /* lastMatch - The linked list value that is the current "hot" (last) match */
        static Myps2LinkedList *lastMatch;
        /* lastMatchKey - The last key to match, saves a load in "hot test" */
        static unsigned int lastMatchKey;

        /* linked_list_new - Create the root lists */
        Myps2LinkedList *linked_list_new() 
        {
                Myps2LinkedList *ret;
                unsigned int i;

                ret = malloc(sizeof(Myps2LinkedList) * LL_NUM_LISTS);
                #if LL_NUM_LISTS > 0
                for(i=0; i < LL_NUM_LISTS; i++)
                {
                        ret[i].next = _EMPTY_LL_MARKER;
                }
                #endif

                return ret;
        }

        char *linked_list_search(Myps2LinkedList *ll, unsigned int searchKey)
        {
                if ( lastMatchKey == searchKey && lastMatch != NULL )
                {
                        return lastMatch->value;
                }

                #if LL_NUM_LISTS > 0
                ll = &ll[searchKey % LL_NUM_LISTS];
                #endif

                if ( IS_EMPTY_LL(ll) )
                {
                        /* Special case -- linked list has no values */
                        return NULL;
                }

                do
                {
                        if ( ll->key == searchKey )
                        {
                                lastMatch = ll;
                                lastMatchKey = searchKey;
                                return ll->value;
                        }                        
                        ll = ll->next;
                }while(ll != NULL);
                
                return NULL;
        }

        void linked_list_insert(Myps2LinkedList *ll, unsigned int insertKey, char *insertValue)
        {
                #if LL_NUM_LISTS > 0
                ll = &ll[insertKey % LL_NUM_LISTS];
                #endif

                if( IS_EMPTY_LL(ll) )
                {
                        /* Special case -- linked list has no values yet */
                        ll->key = insertKey;
                        ll->value = insertValue;
                        ll->next = NULL;
                        return;
                }

                /* Move to next link which has no "next" chain */
                while(ll->next != NULL)
                {
                        ll = (Myps2LinkedList *)ll->next;
                }

                /* Allocate and assign the next chain */
                ll->next = malloc(sizeof(Myps2LinkedList));
                ll = (Myps2LinkedList *)ll->next;

                ll->key = insertKey;
                ll->value = insertValue;
                ll->next = NULL;
        }

#endif

#ifdef _LINUX_LIMITS_H
  #undef _LINUX_LIMITS_H
  #include <linux/limits.h>
#endif

#ifndef ARG_MAX
  #define ARG_MAX 65535
#endif

#ifndef PATH_MAX
  #define PATH_MAX 4096
#endif

// This is extra size to account for null terminators and command name in addition to the longest argv possible. If you need to use this on a system without a lot of memory for some reason, adjust this to a smaller value and define ARG_MAX to be something smaller.
#define EXTRA_ARG_BUFFER 2000

#ifndef ALL_PROCS
char *myUidStr;
unsigned int myUidStrLen;
unsigned int myUid;
#endif


volatile const char *author = "Created by Tim Savannah <kata198@gmail.com>. I love you all so much.";
volatile const char *version = "7.0";

static char **searchItems = NULL;

char *strnstr(char *haystack, char *needle, unsigned int len)
{
    unsigned int i, j;
    char *ret;
    for(i=0; i < len; i++)
    {
        if(haystack[i] == needle[0])
        {
            ret = &haystack[i];
            i++;
            for(j=1; i < len; i++, j++)
            {
                if(needle[j] == '\0')
                    return ret;
                if(needle[j] != haystack[i])
                    break;
            }
            /* Check if we are matching on the end of both */
            if(i == len && needle[j] == '\0' && haystack[i] == '\0')
                return ret;
         }
    }
    return NULL;
}

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

#ifdef REPLACE_EXE_NAME
char *getCommandName(char* pidStr)
{
        static char ret[PATH_MAX];
        static char exeFilePath[128];

        ssize_t len;

        sprintf(exeFilePath, "/proc/%s/exe", pidStr);

        len = readlink(exeFilePath, ret, PATH_MAX);
        if ( len == -1 )
        {
                if ( errno == EACCES )
                        return (char *)-EACCES;
                return NULL;
        }

        ret[len] = '\0';

        return ret;
}
#endif


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
        char *cmdName;

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

        #ifndef REPLACE_EXE_NAME
          cmdName = buffer;
        #else
          cmdName = getCommandName(pidStr);
          if(cmdName == NULL)
                return; // exe is gone, proc must have died.
          else if (cmdName == (char*)-EACCES)
                cmdName = buffer; // Permission denied to readlink, so retain reported name via cmdline
        #endif

        //printf("searchITem is: %s\n", searchItem);
        //printf("buffer is: %s\n", buffer);
        if(searchItems != NULL)
        {
                unsigned int searchI = 0;
                #ifdef CMD_ONLY
                  unsigned int cmdNameLen = strlen(cmdName);
                #endif
                while(searchItems[searchI] != NULL)
                {
                        #ifdef CMD_ONLY
                          // Command only we want the first arg only
                          if(strnstr(cmdName, searchItems[searchI], cmdNameLen) == NULL)
                                  return;
                        #else
                          if(strnstr(buffer, searchItems[searchI], bufferLen) == NULL)
                                  return;
                        #endif
                        searchI++;
                }
        }
                

        #ifdef SHOW_THREADS
          if(parentPidStr != NULL)
                  putchar('\t');
        #endif

        #ifdef ALL_PROCS
            pwName = linked_list_search(pwdInfo, ownerUid);

          if(pwName == NULL)
          {
                  struct passwd *pwinfo = getpwuid(ownerUid);
                  if(pwinfo == NULL)
                          return; // No longer active process
                  pwName = malloc(strlen(pwinfo->pw_name)+1);
                  strcpy(pwName, pwinfo->pw_name);
                  linked_list_insert(pwdInfo, ownerUid, pwName);
          }

          printf("%8s %10s\t%s", pidStr, pwName, cmdName);
        #else
          printf("%8s\t%s", pidStr, cmdName);
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
                  while( (dirInfo = readdir(taskDir)) )
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
                          printf("\t%8s\tThread [%2u] ( %s )\n", threadPid, descended, cmdName);

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
          #ifdef OTHER_USER_PROCS
            if(argc == 1)
            {
                fprintf(stderr, "You must provide another username.\n");
                exit(127);
            }
            else if(argc >= 2)
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
            #define NUM_ARGS_SEARCH 3
            #define NON_SEARCH_ARGS 2
          #else
            myUid = getuid();
            #define NUM_ARGS_SEARCH 2
            #define NON_SEARCH_ARGS 1
          #endif
          
        #else
          #define NUM_ARGS_SEARCH 2
          #define NON_SEARCH_ARGS 1
        #endif
   
        if(argc >= NUM_ARGS_SEARCH)
        {
                unsigned int lastSlot = argc - NON_SEARCH_ARGS;
                unsigned int searchI = 0;
                searchItems = malloc(sizeof(char*) * (lastSlot + 1));
                memcpy(searchItems, &argv[NON_SEARCH_ARGS], lastSlot * sizeof(char*));
                searchItems[lastSlot] = NULL;
        }

        #ifndef ALL_PROCS
        myUidStr = malloc(8);
        sprintf(myUidStr, "%u", myUid);
        myUidStrLen = strlen(myUidStr);
        #endif

        myPidStr = malloc(8);
        myPid = getpid();

        sprintf(myPidStr, "%u", myPid);
        #ifdef ALL_PROCS
            pwdInfo = linked_list_new();
            lastMatch = NULL;
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
        return 0;

}

// vim: ts=8:sw=8:expandtab
