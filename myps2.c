/*
 * Copyright (c) 2015, 2016, 2017, 2018 Tim Savannah <kata198@gmail.com> under GPLv3.
 * See LICENSE file for more information.
 *
 *
 * Code Style:
 * vim: ts=8:sw=8:st=8:expandtab
 * two-space indents between preprocessor conditionals. All binaries come from mutations on this one file. bwahahaha!!
 *
 * Code is designed with as much static and reuse as possible for speed sake. Optimizations may seem silly,
 *   and arguments instead of different commands may seem 'better', but the goal here is fast-as-possible.
 *   The actual program runs for me in 1ms user time, with lots of processes and several users.
 *   Other optimizations try to control the 'sys' latency, by limiting calls to the minimum necessary.
 *
 *   This should run in all forms with the only memory left in "still reachable" category of valgrind.
 */

/* vim: set ts=8 sw=8 st=8 expandtab : */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/mman.h>

/* Include optional config overrides ( by ./configure ) */
#include "myps2_config.h"

/* Maximum length of a /proc/PID/item path, with plenty of room to spare.
 *   In usage, is doubled if going into deep subdirs.
 */
#define PROC_PATH_LEN 32

#ifdef __GNUC__
  #define likely(x)    __builtin_expect(!!(x),1)
  #define unlikely(x)  __builtin_expect(!!(x),0)
  #define __hot __attribute__((hot))
#else
  #define likely(x)   x
  #define unlikely(x) x
  #define __hot
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

/* Don't use output buffer on "pidof" commands */
#ifdef PIDOF
#define NO_OUTPUT_BUFFER
#endif

#if !defined(NO_OUTPUT_BUFFER) && ( !defined(OUTPUT_BUFFER_SIZE) || OUTPUT_BUFFER_SIZE > 0)
  #ifndef OUTPUT_BUFFER_SIZE
    /* Default 16K */
    #define OUTPUT_BUFFER_SIZE (1024 * 16)
  #endif
#endif

volatile const char *author = "Created by Tim Savannah <kata198@gmail.com>. I love you all so much.";
volatile const char *version = "7.2.3";

static char **searchItems = NULL;
#ifdef PIDOF
static int anyResults = 0;
#endif

#if defined(ALL_PROCS) && !defined(PIDOF)

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


        /* The number of linked lists, set this to 1 to disable last-digit hashing */
        #ifndef LL_NUM_LISTS
          #define LL_NUM_LISTS 10
        #endif

        #if LL_NUM_LISTS < 1
          #error "LL_NUM_LISTS MUST BE >= 1"
        #endif


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
        static Myps2LinkedList *linked_list_new() 
        {
                Myps2LinkedList *ret;
                unsigned int i;

                ret = malloc(sizeof(Myps2LinkedList) * LL_NUM_LISTS);
                for(i=0; i < LL_NUM_LISTS; i++)
                {
                        ret[i].next = _EMPTY_LL_MARKER;
                }

                return ret;
        }

        static char *linked_list_search(Myps2LinkedList *ll, unsigned int searchKey)
        {
                if ( lastMatchKey == searchKey && lastMatch != NULL )
                {
                        /* Hot match */
                        return lastMatch->value;
                }

                #if LL_NUM_LISTS > 1
                  ll = &ll[searchKey % LL_NUM_LISTS];
                #endif

                if ( unlikely( IS_EMPTY_LL(ll) ) )
                {
                        /* Special case -- linked list has no values. 
                         * Unlikely because at max will only be true once per user. */
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

        static void linked_list_insert(Myps2LinkedList *ll, unsigned int insertKey, char *insertValue)
        {
                #if LL_NUM_LISTS > 1
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

/* On cygwin and BSD the strnstr function is already defined in string.h 
     So prefix our function as myps2_strnstr
*/
static char *myps2_strnstr(const char *haystack, const char *needle, size_t len)
{
    unsigned int i, j;
    char *ret;

    for(i=0; i < len; i++)
    {
        if(haystack[i] == needle[0])
        {
            ret = (char *)&haystack[i];
            i++;
            for(j=1; i < len; i++, j++ )
            {
                if(needle[j] == '\0')
                    return ret;
                if(needle[j] != haystack[i])
                    break;
            }
            /* Check if we are matching on the end of both */
            if( unlikely( i == len ) && needle[j] == '\0' && haystack[i] == '\0' )
                return ret;
         }
    }
    return NULL;
}

#ifdef SHOW_THREADS
  #ifndef THREADS_VIEW_FLAT
    #define THREADS_USE_TREE // For now, default to always showing tree format in threads mode. Comment out this line to get a less-pleasing view.
  #endif
#endif

#ifdef QUOTE_ARGS
static char *escapeQuotes(char *input)
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

__hot static unsigned int getProcessOwner(char *pidStr)
{
        static struct stat info;
        /* Reuse "path" variable and /proc/ portion, */
        static char path[PROC_PATH_LEN] = { '/', 'p', 'r', 'o', 'c', '/', 0 };

//        path[6] = '\0';
        strcpy(&path[6], pidStr);

        if( unlikely( stat(path, &info) == -1 ) )
                return 65535;

        return info.st_uid;
}

#ifdef REPLACE_EXE_NAME
static char *getCommandName(char* pidStr)
{
        static char ret[PATH_MAX + 1];;
        static char exeFilePath[PROC_PATH_LEN] = { '/', 'p', 'r', 'o', 'c', '/', 0 };

        ssize_t len;

        sprintf(&exeFilePath[6], "%s/exe", pidStr);

        len = readlink(exeFilePath, ret, PATH_MAX);
        if ( len == -1 )
        {
                return NULL;
        }

        ret[len] = '\0';

        return ret;
}
#endif


__hot static void printCmdLineStr(char *pidStr 
#ifdef ALL_PROCS
                     ,unsigned int ownerUid 
#endif
#ifdef SHOW_THREADS
                     ,char *parentPidStr
#endif
)
{
        int cmdlineFileFD;
        /* Reuse buffers each run */
        static char buffer[ARG_MAX + EXTRA_ARG_BUFFER];
        static char cmdlineFilename[PROC_PATH_LEN * 2] = {'/', 'p', 'r', 'o', 'c', '/', 0};
        #ifndef PIDOF
        static char *cmdName;
        #endif

        static ssize_t bufferLen;
        #ifdef QUOTE_ARGS
          char *tmpEscaped;
        #endif
        
        #if defined(ALL_PROCS) && !defined(PIDOF)
          static char *pwName;
        #endif

        #ifdef SHOW_THREADS
          static char parentDir[PROC_PATH_LEN] = {'/', 'p', 'r', 'o', 'c', '/', 0 };
        #endif

        #if defined(SHOW_THREADS)
          /* If we are thread, don't bother reading "cmdline" nor matching again, it will be the same. */
          if(!parentPidStr)
          {
                        
        #endif
                // cmdlineFilename[6] = '\0';
                
                sprintf(&cmdlineFilename[6], "%s/cmdline", pidStr);

                        
                cmdlineFileFD = open(cmdlineFilename, O_RDONLY);
                if ( unlikely( cmdlineFileFD == -1 ) )
                        return;

                bufferLen = read(cmdlineFileFD, buffer, ARG_MAX + EXTRA_ARG_BUFFER - 1);
                close(cmdlineFileFD);

                if ( unlikely( bufferLen <= 0 ) )
                        return; // No cmdline, kthread and the like

                buffer[bufferLen] = '\0';

                #ifndef PIDOF
                #ifndef REPLACE_EXE_NAME
                  cmdName = buffer;
                #else
                  errno = 0;
                  cmdName = getCommandName(pidStr);
                  if ( unlikely( cmdName == NULL ) )
                  {
                        if ( errno == EACCES )
                                cmdName = buffer; // Permission denied to readlink, so retain reported name via cmdline
                        else
                                return; // exe is gone, proc must have died.
                  }
                #endif
                #endif

                //printf("searchITem is: %s\n", searchItem);
                //printf("buffer is: %s\n", buffer);
                if (searchItems != NULL)
                {
                        unsigned int searchI = 0;
                        #ifdef CMD_ONLY
                          unsigned int cmdNameLen = strlen(cmdName);
                        #endif
                        #if defined(REPLACE_EXE_NAME) && !defined(CMD_ONLY)
                          /* If we potentially changed the cmdName, then we must scan the NEW cmdName
                           *  for match, and exclude the OLD cmdName from the buffer scan
                           */
                          char *useBuffer;
                          unsigned int useBufferLen;

                          if ( buffer == cmdName )
                          {
                                useBuffer = (char *)buffer;
                                useBufferLen = bufferLen;
                          }
                          else
                          {
                                useBuffer = (char *) ( &buffer[ strlen(buffer) + 1 ] );
                                useBufferLen = bufferLen - ( useBuffer - ((char *)buffer) );
                          }
                        #endif
                        while(searchItems[searchI] != NULL)
                        {
                                #ifdef CMD_ONLY
                                  // Command only we want the first arg only
                                  if(myps2_strnstr(cmdName, searchItems[searchI], cmdNameLen) == NULL)
                                          return;
                                #else
                                  #if defined(REPLACE_EXE_NAME) && !defined(CMD_ONLY)
                                    if( myps2_strnstr(useBuffer, searchItems[searchI], useBufferLen ) == NULL && strstr(cmdName, searchItems[searchI]) == NULL )
                                        return;
                                  #else
                                    if(myps2_strnstr(buffer, searchItems[searchI], bufferLen) == NULL)
                                          return;
                                  #endif
                                #endif
                                searchI++;
                        }
                }

        #if defined(SHOW_THREADS)
          }
        #endif

        #if defined(SHOW_THREADS) && !defined(THREADS_VIEW_FLAT)
          /* Tab-indent next level for threads */
          if (parentPidStr != NULL)
                  putchar('\t');
        #endif

        #ifdef PIDOF
          if ( anyResults )
                putchar(' ');
          anyResults = 1;
          printf("%s", pidStr);
          #ifndef SHOW_THREADS
            /* If we aren't showing thread pids, can skip remainder of function */
            return;
          #endif
        #else

          #ifdef ALL_PROCS
            #if defined(SHOW_THREADS)
            /* Here we will use the already-found pwName from the parent process */
            if (!parentPidStr)
            {
            #endif
                    pwName = linked_list_search(pwdInfo, ownerUid);

                    if ( unlikely( pwName == NULL ) )
                    {
                            struct passwd *pwinfo = getpwuid(ownerUid);
                            if ( unlikely( pwinfo == NULL ) )
                                    return; // No longer active process
                            pwName = malloc(strlen(pwinfo->pw_name)+1);
                            strcpy(pwName, pwinfo->pw_name);
                            linked_list_insert(pwdInfo, ownerUid, pwName);
                    }
            #if defined(SHOW_THREADS)
            }
            #endif

            printf("%8s %10s\t%s", pidStr, pwName, cmdName);
          #else
            printf("%8s\t%s", pidStr, cmdName);
          #endif


          #ifndef CMD_ONLY
            // Skip argument block in command only mode

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
          #endif
          putchar('\n');

        #endif

        #ifdef SHOW_THREADS
          if(parentPidStr == NULL)
          {
                  DIR *taskDir;
                  struct dirent *dirInfo;
                  char *threadPid;
                  #ifndef PIDOF
                    short threadID = 0;
                  #endif

                  sprintf(&parentDir[6], "%s/task", pidStr);

                  taskDir = opendir(parentDir);
                  while( (dirInfo = readdir(taskDir)) )
                  {
                          threadPid = dirInfo->d_name;
                          if(!isdigit(threadPid[0]) || strcmp(threadPid, pidStr) == 0)
                                  continue;
                          #ifdef PIDOF

                            printf(" %s", threadPid);
                          #else
                            if(threadID == 0)
                            {
                                    #ifdef THREADS_USE_TREE
                                      printf("     |\n     \\ ____\n");
                                    #endif
                                    threadID = 1;
                            }
                            #ifdef THREADS_VIEW_FULL
                              #ifdef THREADS_USE_TREE
                                printf("\t%8s\tThread [%2u] ( %s )\n", threadPid, threadID, cmdName);
                              #else
                                #ifdef ALL_PROCS
                                  printf("%8s %10s\t(Thread [%2u] of %s)\n", threadPid, pwName, threadID, pidStr);
                                #else
                                  printf("%8s\t(Thread [%2u] of %s)\n", threadPid, threadID, pidStr);
                                #endif
                              #endif
                            #else
                            /* THREADS_VIEW_FULL - recurse */
                            printCmdLineStr(threadPid
                                                        #ifdef ALL_PROCS
                                                          ,ownerUid
                                                        #endif
                                                          ,pidStr
                            );
                            #endif
                            threadID++;
                          #endif
                  }
                  free(taskDir);
                  #ifndef PIDOF
                    #ifdef THREADS_USE_TREE
                    if(threadID >= 1)
                          putchar('\n');
                    #endif
                  #endif

          }
        #endif
}

int main(int argc, char* argv[])
{
        unsigned int myPid, processOwner;

        DIR *procDir;
        char *pid;
        char myPidStr[8];
        struct dirent *dirInfo;
        #ifndef ALL_PROCS
          unsigned int myUid;
        #endif

        #ifndef ALL_PROCS
          #ifdef OTHER_USER_PROCS
            if(argc <= 1)
            {
                fprintf(stderr, "You must provide another username.\n");
                exit(127);
            }
            else
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
                searchItems = malloc(sizeof(char*) * (lastSlot + 1));
                memcpy(searchItems, &argv[NON_SEARCH_ARGS], lastSlot * sizeof(char*));
                searchItems[lastSlot] = NULL;
        }

        #ifndef NO_OUTPUT_BUFFER
          char *outputBuffer = NULL;
          #ifndef MAP_ANONYMOUS
            int outputBufferFD;
            outputBufferFD = open("/dev/zero", O_RDONLY);
            if ( likely( outputBufferFD > 0 ) )
            {
                outputBuffer = mmap(NULL, OUTPUT_BUFFER_SIZE + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, outputBufferFD, 0);
          #else
                outputBuffer = mmap(NULL, OUTPUT_BUFFER_SIZE + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
          #endif
                if ( unlikely( outputBuffer == MAP_FAILED ) )
                {
                        #ifndef MAP_ANONYMOUS
                          close(outputBufferFD);
                          outputBufferFD = -1;
                        #endif
                        outputBuffer = NULL;
                }
                else
                {
                        setvbuf(stdout, outputBuffer, _IOFBF, OUTPUT_BUFFER_SIZE);
                }
          #ifndef MAP_ANONYMOUS
            }
          #endif
        #endif

        myPid = getpid();

        sprintf(myPidStr, "%u", myPid);
        #if defined(ALL_PROCS) && !defined(PIDOF)
            pwdInfo = linked_list_new();
            lastMatch = NULL;
        #endif

        procDir = opendir("/proc");
        while( (dirInfo = readdir(procDir)) )
        {
                pid = dirInfo->d_name;
                if(!isdigit(pid[0]) || unlikely( strcmp(myPidStr, pid) == 0 ) )
                        continue;
                processOwner = getProcessOwner(pid);
                #ifndef ALL_PROCS
                  if(processOwner == myUid)
                #endif
                printCmdLineStr(pid
                #ifdef ALL_PROCS
                                ,processOwner
                #endif
                #ifdef SHOW_THREADS
                                ,NULL
                #endif
                );
        }
        #ifdef PIDOF
          if ( anyResults )
            putchar('\n');
        #endif
        fflush(stdout);

        free(procDir);
        if(searchItems != NULL)
        {
            free(searchItems);
        }

        #ifndef NO_OUTPUT_BUFFER
          if( likely( outputBuffer != NULL ) )
          {
                #if __GNUC__ >= 5
                /* Silence stupid GCC warning. In all cases that outputBufferFD != NULL here, outputBuffer is initialized */
                #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
                #else
                #warning GCC is wrong about unused variable, and cannot disable warning on your gcc version.
                #endif
                munmap(outputBuffer, OUTPUT_BUFFER_SIZE);
                #if __GNUC__ >= 5
                #pragma GCC diagnostic pop
                #endif

                #ifndef MAP_ANONYMOUS
                  close(outputBufferFD);
                #endif
          }
        #endif
        
        return 0;

}

// vim: ts=8:sw=8:expandtab
