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

#include "Responder.h"
#include "StringEx.h"
#include "StringList.h"

#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#if !defined (_WIN32) && !defined (_WIN64)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#pragma pack(1)
typedef struct responder_t
{
    bool			connected;
    socket_t 		socket;
    struct sockaddr_in		server_address;
    char			server_name[33];
    int				server_port;
    size_t			prefetched_buffer_size;
    unsigned char*	prefetched_buffer;
    int             error_code;
}responder_t;

bool is_ip4_address(char* str);

bool is_ip4_address(char* str)
{
    size_t slen = strlen(str);

    // Check the string length, for the range ...
    // 0.0.0.0 and 255.255.255.255
    if(slen < 7 || slen > 15)
    {
        // Bail out
        return false;
    }

    int ctr;
    bool isdelimeter = false;
    char nibble[4];
    memset((char*)&nibble[0],0,4);
    int nbindex = 0;
    for(ctr = 0 ; str[ctr] != '\0' ; ctr++)
    {
        // Check for permitted characters
        if(str[ctr] != '.' && isdigit(str[ctr]) <= 0)
        {
            // Bail out
            return false;
        }

        // '.' Delimeter case
        if(str[ctr] == '.')
        {
            if(isdelimeter)
            {
                // The flag was set in last iteration
                // This means ".." type of expression was found
                // Bail out
                return false;
            }

            // We have read a complete nibble
            // The characters in the nibble must represent a permissible value
            int numval = atoi(nibble);
            if(numval < 0 || numval > 255)
            {
                return false;
            }

            // Set the flag and continue
            memset((char*)&nibble[0],0,4);
            nbindex = 0;
            isdelimeter = true;
            continue;
        }

        if(isdigit(str[ctr])> 0)
        {
            isdelimeter = false;
            nibble[nbindex] = str[ctr];
            nbindex++;
            continue;
        }

    }

    return true;
}

responder_t* responder_allocate()
{
    responder_t* ptr = (responder_t*)calloc(1, sizeof (responder_t));
    return ptr;
}

void responder_free(responder_t* ptr)
{
    free(ptr);
}

responder_t *responder_create_socket(responder_t *ptr, const char* servername, int serverport)
{
    if(!ptr)
    {
        return  NULL;
    }

    strncpy(ptr->server_name, servername, 32);
    ptr->server_port = serverport;

    ptr->server_address.sin_family = AF_INET;
    ptr->server_address.sin_port = htons(serverport);

    u_long nRemoteAddr;

    char ipbuffer[32]={0};
    strncpy(ipbuffer, servername, 31);

    bool ip = is_ip4_address(ipbuffer);

    if(!ip)
    {
        struct hostent* pHE = gethostbyname(ptr->server_name);
        if (pHE == 0)
        {
            nRemoteAddr = INADDR_NONE;
            free(ptr);
            return NULL;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
        ptr->server_address.sin_addr.s_addr = nRemoteAddr;
    }
    else
    {
         inet_pton (AF_INET, ptr->server_name, &ptr->server_address.sin_addr);
    }

    ptr->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(ptr->socket == INVALID_SOCKET)
    {
        free(ptr);
        return NULL;
    }

    return ptr;
}

responder_t *responder_assign_socket(responder_t *ptr, int inSocket)
{
    if(!ptr)
    {
        return  NULL;
    }

    ptr->socket = inSocket;
    ptr->connected = true;
    return ptr;
}

bool responder_connect_socket(responder_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    if(ptr->connected == true)
	{
		return true;
	}

    int returncode = -1;

    returncode = connect(ptr->socket,(struct sockaddr*)&ptr->server_address, sizeof(struct sockaddr_in));

    if(returncode == SOCKET_ERROR)
	{
        ptr->error_code = errno;
        shutdown(ptr->socket, 2);
        closesocket(ptr->socket);
        ptr->connected = false;
		return false;
	}

    ptr->connected = true;
	return true;
}

bool responder_close_socket(responder_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    if(ptr->connected == false)
    {
        return true;
    }

    shutdown(ptr->socket, 2);
    closesocket(ptr->socket);

    ptr->connected = false;

	return false;
}

bool responder_receive_buffer(responder_t* ptr, char** iobuffer, size_t len, bool alloc_buffer)
{
    if(!ptr)
    {
        return  false;
    }

    size_t	bufferpos = 0;
    size_t	bytesleft = len;

    // If there are pre-fetched bytes left, we have to copy that first and release memory

    if(alloc_buffer)
    {
        *iobuffer = (char*)calloc(1, len + 1);
    }

    if(ptr->prefetched_buffer_size > 0)
    {
        memcpy(*iobuffer, ptr->prefetched_buffer, ptr->prefetched_buffer_size);
        bytesleft = len - ptr->prefetched_buffer_size;
        bufferpos = ptr->prefetched_buffer_size;
        ptr->prefetched_buffer_size = 0;
        free(ptr->prefetched_buffer);
        ptr->prefetched_buffer = NULL;

        if(bytesleft < 1)
        {
            return true;
        }
    }

    while(true)
    {
        char*	buffer = 0;
        ssize_t	bytesread = 0;
        buffer = (char*)calloc(1, bytesleft + 1);

        if (buffer)
        {
            bytesread = (ssize_t)recv(ptr->socket, buffer, (int)bytesleft, 0);
        }

        // Error or link down
        if(bytesread < 1 || buffer == NULL)
        {
            ptr->error_code = SOCKET_ERROR;

            if (buffer)
            {
                free(buffer);
            }

            if(alloc_buffer)
            {
                free(*iobuffer);
            }
            else
            {
                memset(*iobuffer, 0, len);
            }

            len	= 0;
            ptr->connected = false;
            return false;
        }

        memcpy(*iobuffer+bufferpos, buffer, (size_t)bytesread);
        free(buffer);

        bufferpos = bufferpos + (size_t)bytesread;

        bytesleft = bytesleft - (size_t)bytesread;

        if(bufferpos >= len)
        {
            return true;
        }
    }
}

bool responder_receive_string(responder_t* ptr, char** iostr, const char* delimeter)
{
    if(!ptr)
    {
        return  false;
    }

    char*	data = NULL;
    char*   current_line = NULL;
    char*   next_line = NULL;

    if(ptr->prefetched_buffer_size > 0)
	{
        if(strstr((char*)ptr->prefetched_buffer, delimeter) !=0 )
		{
            strsplitkeyvaluesubstr((const char*)ptr->prefetched_buffer, delimeter, &current_line, &next_line);

            ptr->prefetched_buffer = NULL;
            free(ptr->prefetched_buffer);
            ptr->prefetched_buffer_size = 0;

            if(current_line != NULL)
            {
                *iostr = (char*)calloc(1, strlen(current_line));
                strcpy(*iostr, current_line);
                free(current_line);
            }

            if(next_line != NULL)
            {
                ptr->prefetched_buffer_size = strlen(next_line);
                ptr->prefetched_buffer = (unsigned char*)calloc(1, (sizeof (unsigned char)*ptr->prefetched_buffer_size) + 1);
                strcpy((char*)ptr->prefetched_buffer, next_line);
                free(next_line);
            }

			return true;
		}

        if(ptr->prefetched_buffer_size > 0)
        {
            data = (char*)calloc(1, ptr->prefetched_buffer_size + 1);
            strcpy(data, (char*)ptr->prefetched_buffer);
            ptr->prefetched_buffer_size = 0;
            free(ptr->prefetched_buffer);
            ptr->prefetched_buffer = NULL;
        }
	}

	while(true)
	{
        char* buffer = NULL;

        if(!responder_receive_buffer(ptr, &buffer, 1024, true))
        {
            if(*iostr)
            {
                free(*iostr);
            }

            ptr->connected = false;
            ptr->error_code = SOCKET_ERROR;
            return false;
        }

        data = (char*)realloc(data, strlen(data) + 1024);
        strcat(data, buffer);
        free(buffer);

        if(strstr(data, delimeter) != 0)
		{
            strsplitkeyvaluesubstr((const char*)ptr->prefetched_buffer, delimeter, &current_line, &next_line);

            if(next_line != NULL)
            {
                ptr->prefetched_buffer_size = strlen(next_line);
            }
            
            if(ptr->prefetched_buffer_size > 0)
            {
                ptr->prefetched_buffer = (unsigned char*)calloc(1, sizeof (unsigned char));
                memcpy(ptr->prefetched_buffer, next_line, ptr->prefetched_buffer_size);
                free(next_line);
            }

            *iostr = (char*)realloc(*iostr, strlen(*iostr) + strlen(current_line));
            strcat(*iostr, current_line);
            free(current_line);
            free(data);

            return true;
		}
	}
	return true;
}

bool responder_send_buffer(responder_t* ptr, const char* data, size_t len)
{
    if(!ptr)
    {
        return  false;
    }

	long sentsize =0;

    sentsize = send(ptr->socket, data, (int)len, (int)0);

    if(sentsize == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool responder_send_string(responder_t* ptr, const char* str)
{
    size_t len = strlen(str);

    return responder_send_buffer(ptr, str, len);
}

size_t responder_read_size(responder_t *ptr)
{
    if(!ptr)
    {
        return  false;
    }

    return ptr->prefetched_buffer_size;
}

bool responder_is_connected(responder_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    return ptr->connected;
}

socket_t responder_get_socket(responder_t *ptr)
{
    if(!ptr)
    {
        return  false;
    }

    if(ptr->connected)
    {
        return ptr->socket;
    }

    return -1;
}

int  responder_get_error_code(responder_t* ptr)
{
    return 0;
}
