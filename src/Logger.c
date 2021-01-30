/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Logger.h"
#include "Directory.h"
#include "File.h"
#include "StringEx.h"
#include "StringList.h"
#include "Environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define END_OF_LINE "\n"
#define MAX_LOGGERS 512

static char log_level_names[5][16] = {"Information", "Error", "Warning", "Critical", "Panic"};
pthread_mutex_t mutex;

void normalize_function_name(char* func_name);

#pragma pack(1)
typedef struct logger_t
{
    size_t LogFileSizeMB;
    char FileName[1025];
    FILE* FileHandle;
    bool console_out;
    LogLevel log_level;
}logger_t;

logger_t*	logger_allocate_default()
{
    return logger_allocate(10, NULL);
}

logger_t*  logger_allocate_file(size_t flszmb, const char* filename)
{
    if(!filename)
    {
        return NULL;
    }

    logger_t* logger_ptr = (logger_t*)calloc(1, sizeof (logger_t));

    if(!logger_ptr)
    {
        return NULL;
    }

    logger_ptr->FileHandle = NULL;

    if(flszmb < 1 || flszmb > 10)
    {
        flszmb = 10;
    }

    logger_ptr->LogFileSizeMB = flszmb;
    strncpy(logger_ptr->FileName, filename, 1024);

    pthread_mutex_init(&mutex, NULL);
    logger_ptr->log_level = LOG_INFO;
    logger_ptr->console_out = false;

    return logger_ptr;
}


logger_t*	logger_allocate(size_t flszmb, const char* dirpath)
{
    logger_t* logger_ptr = (logger_t*)calloc(1, sizeof (logger_t));

    if(!logger_ptr)
    {
        return NULL;
    }

    logger_ptr->FileHandle = NULL;

    if(flszmb < 1 || flszmb > 10)
    {
        flszmb = 10;
    }

    logger_ptr->LogFileSizeMB = flszmb;

    if(dirpath != NULL)
    {
        strcat(logger_ptr->FileName, dirpath);

        if(dirpath[strlen(dirpath) - 1] != '/')
        {
            strcat(logger_ptr->FileName, "/");
        }
    }
    else
    {
        if(strcmp(getenv("USER"), "root") == 0)
        {
            strcat(logger_ptr->FileName, "/var/log/");
        }
        else
        {
            strcat(logger_ptr->FileName, getenv("HOME"));
            strcat(logger_ptr->FileName, "/log/");
        }
    }

    if(!dir_is_exists(logger_ptr->FileName))
    {
        dir_create_directory(logger_ptr->FileName);
    }

    char* proces_name = (char*)calloc(1, 1025);;
    proces_name = env_get_current_process_name(proces_name);
    strcat(logger_ptr->FileName, proces_name);
    strcat(logger_ptr->FileName, ".log");
    free(proces_name);

    pthread_mutex_init(&mutex, NULL);
    logger_ptr->log_level = LOG_INFO;
    logger_ptr->console_out = false;

    return logger_ptr;
}

const char* logger_filename(logger_t* loggerptr)
{
    if(!loggerptr)
    {
        return NULL;
    }

    return &loggerptr->FileName[0];
}

void logger_release(logger_t* loggerptr)
{
    if(!loggerptr)
    {
        return;
    }

    pthread_mutex_lock(&mutex);

    if(loggerptr->FileHandle)
    {
        fflush(loggerptr->FileHandle);
        fclose(loggerptr->FileHandle);
    }

    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    free(loggerptr);
}

bool logger_write(logger_t* loggerptr, const char* logentry, LogLevel llevel, const char* func, const char* file, int line)
{
    if(!loggerptr)
    {
        return false;
    }

    if(llevel < loggerptr->log_level)
    {
        return false;
    }

    pthread_mutex_lock(&mutex);

    if(loggerptr->FileHandle == NULL)
    {
        loggerptr->FileHandle = fopen(loggerptr->FileName, "w");

        if(loggerptr->FileHandle == NULL)
        {
            pthread_mutex_unlock(&mutex);
            return false;
        }
    }

    // Check the file size
    size_t sz = (size_t)ftell(loggerptr->FileHandle);

    // If it exceeds the set size
    if(sz >= loggerptr->LogFileSizeMB*1024*1024)
    {
        // Stop logging
        fflush(loggerptr->FileHandle);
        fclose(loggerptr->FileHandle);

        // Rename the file
        char old_log_filename[1025] = {0};
        strcat(old_log_filename, loggerptr->FileName);
        strcat(old_log_filename, ".old");

        rename(loggerptr->FileName, old_log_filename);

        // Reopen the log file with original name
        loggerptr->FileHandle = fopen(loggerptr->FileName, "w");

        if(loggerptr->FileHandle == NULL)
        {
            pthread_mutex_unlock(&mutex);
            return false;
        }
    }

    time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

    // Timestamp
    fprintf(loggerptr->FileHandle, "%02d-%02d-%04d %02d:%02d:%02d\t",
             tmp->tm_mday, (tmp->tm_mon+1), (tmp->tm_year+1900),
             tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    // Level
    fprintf(loggerptr->FileHandle, "%s\t", log_level_names[llevel]);

    // File
    char* base_file_name = file_get_basename(file);
    fprintf(loggerptr->FileHandle, "%s\t", base_file_name);

    // Line
    fprintf(loggerptr->FileHandle, "%d\t", line);

    // Function
    char* func_name = (char*)calloc(1, strlen(func)+1);
    strcpy(func_name, func);
    normalize_function_name(func_name);
    fprintf(loggerptr->FileHandle, "%s\t", func_name);

    // Message
    fprintf(loggerptr->FileHandle, "%s", logentry);

    // End of line
    fprintf(loggerptr->FileHandle, END_OF_LINE);

    // Flush th contents
    fflush(loggerptr->FileHandle);

    if(loggerptr->console_out)
    {
        printf("%s %d %s %s\n", base_file_name, line, func, logentry);
        fflush(stdout);
    }

    free(base_file_name);
    free(func_name);
    pthread_mutex_unlock(&mutex);

    return true;
}

void logger_enable_console_out(logger_t* loggerptr, bool consoleout)
{
    if(!loggerptr)
    {
        return;
    }

    loggerptr->console_out = consoleout;
}

void logger_set_log_level(logger_t* loggerptr, LogLevel llevel)
{
    if(!loggerptr)
    {
        return;
    }

    loggerptr->log_level = llevel;
}

void normalize_function_name(char* func_name)
{
    int len = (int)strlen(func_name);

    if(len < 2)
    {
        return;
    }

    long ctr = len - 1;

    long pos = 0;

    pos = strindexofchar(func_name, '(');

    if(pos > -1)
    {
        while(true)
        {
            func_name[ctr] = 0;
            ctr--;
            if(func_name[ctr] == '(')
            {
                func_name[ctr] = 0;
                break;
            }
        }
    }

    pos = strindexofchar(func_name, ' ');

    if(pos > -1)
    {
        ctr = pos;
        while(ctr > -1)
        {
            func_name[ctr] = 32;
            ctr--;
        }
    }

    strlefttrim(func_name);
}
