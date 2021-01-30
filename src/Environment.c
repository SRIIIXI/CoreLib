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

#include "Environment.h"
#include "StringEx.h"
#include "StringList.h"
#include "Directory.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

char* env_get_current_process_name(char* ptr)
{
    if(ptr == NULL)
    {
        return NULL;
    }

    char* buffer = (char*)calloc(1, 32);
    pid_t proc_id = getpid();

    char** cmd_args = NULL;
    char** dir_tokens = NULL;

    sprintf(buffer, "/proc/%d/cmdline", proc_id);

    FILE* fp = fopen(buffer, "r");
    free(buffer);
    buffer = NULL;

    if(fp)
    {
        buffer = (char*)calloc(1, 1025);

        if(fgets(buffer, 1024, fp))
        {
            long dir_sep_pos = strindexofchar(buffer, '/');

            if(dir_sep_pos < 0)
            {
                strcpy(ptr, buffer);
                free(buffer);
                fclose(fp);
                return ptr;
            }

            cmd_args = strsplitchar(buffer, ' ');

            if(cmd_args != NULL)
            {
                dir_tokens = strsplitchar(cmd_args[0], '/');
            }
            else
            {
                dir_tokens = strsplitchar(buffer, '/');
            }

            if(dir_tokens != NULL)
            {
                char* last_str = NULL;
                for(int index = 0; dir_tokens[index] != 0; index++)
                {
                    last_str = dir_tokens[index];
                }
                strcpy(ptr, last_str);
            }

            if(cmd_args)
            {
                strfreelist(cmd_args);
            }

            if(dir_tokens)
            {
                strfreelist(dir_tokens);
            }
        }
        else
        {
            printf("Could not read process commandline\n");
        }

        fclose(fp);
    }

    if(buffer)
    {
        free(buffer);
    }

    return ptr;
}

char* env_get_current_user_name(void)
{
    return getenv("USER");
}

char* env_get_lock_filename(void)
{
    char* lock_filename = (char*)calloc(1, 1025);
    char process_name[64] = {0};

    char temp[33] = {0};

    strcat(lock_filename, dir_get_temp_directory(temp));
    strcat(lock_filename, "/");
    strcat(lock_filename, env_get_current_process_name(process_name));
    strcat(lock_filename, ".");
    strcat(lock_filename, env_get_current_user_name());
    strcat(lock_filename, ".lock");

    return  lock_filename;
}

bool env_lock_process(const char* lock_filename)
{
    int lock_file = 0;

    if (lock_file != 0 && lock_file != -1)
    {
        //File is already open
        return false;
    }

    lock_file = open(lock_filename, O_CREAT | O_RDWR, 0666);
    if (lock_file != -1)
    {
        off_t sz = 0;
        int rc = lockf(lock_file, F_TLOCK, sz);
        if (rc == -1)
        {
            close(lock_file);
            lock_file = 0;
            return false;
        }

        // Okay! We got a lock
        return true;
    }
    else
    {
        lock_file = 0;
        return false;
    }

    return true;
}
