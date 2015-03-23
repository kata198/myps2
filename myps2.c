/*
 * Copyright 2015 Tim Savannah <kata198@gmail.com> under GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/types.h>
#include <dirent.h>


#if defined(__CYGWIN__)
#warning cygwin detected, if you get failures to link below for g_hash whatever, add -D NO_GLIB to the CFLAGS in the Makefile.
#endif

// Uncomment this line to build without glib.
//#define NO_GLIB

#ifdef ALL_PROCS
#include <pwd.h>
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
		if (myUidStr[i]	!= uidPortion[i])
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

#ifndef ALL_PROCS
void printCmdLineStr(char *pidStr)
#else
void printCmdLineStr(char *pidStr, unsigned int ownerUid)
#endif
{
	FILE *cmdlineFile;
	static char *cmdlineFilename = NULL;
	static char *buffer = NULL;

	unsigned int bufferLen = 0;
	#ifdef QUOTE_ARGS
	char *tmpEscaped;
	#endif
	
	#ifdef ALL_PROCS
	char *pwName;
	#endif

	if(cmdlineFilename == NULL)
	{
		cmdlineFilename = malloc(32);
		buffer = malloc(ARG_MAX + EXTRA_ARG_BUFFER);
	}
	
	sprintf(cmdlineFilename, "/proc/%s/cmdline", pidStr);
	cmdlineFile = fopen(cmdlineFilename, "r");
	if(!cmdlineFile)
		return;

	bufferLen = fread(buffer, 1, ARG_MAX + (EXTRA_ARG_BUFFER - 1), cmdlineFile);
	fclose(cmdlineFile);

    if(bufferLen == 0)
		return; // No cmdline, kthread and the like

	buffer[bufferLen] = '\0';

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
	putchar('\n');
	return;
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
}

int main(int argc, char* argv[])
{
	unsigned int myPid;

	DIR *procDir;
	char *pid;
	char *myPidStr;
	struct dirent *dirInfo;

	myUid = getuid();
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
		#ifdef ALL_PROCS
		printCmdLineStr(pid, getProcessOwner(pid));
		#else
		if(getProcessOwner(pid) == myUid)
			printCmdLineStr(pid);
		#endif
	}

}
