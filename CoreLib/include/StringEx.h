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

#ifndef STRING_EX_C
#define STRING_EX_C

#include "Defines.h"
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

extern LIBRARY_EXPORT wchar_t* strtowstr(const char* str);
extern LIBRARY_EXPORT char* strfromwstr(const wchar_t* wstr);

extern LIBRARY_EXPORT char* strfromint(long num);
extern LIBRARY_EXPORT char* strfromdouble(double num);

extern LIBRARY_EXPORT char* strreverse(char* ptr);
extern LIBRARY_EXPORT char* strsegmentreverse(char* str, long start, long term);

extern LIBRARY_EXPORT long strindexofsubstr(const char *str, const char* substr);
extern LIBRARY_EXPORT long strindexofchar(const char *str, const char ch);

extern LIBRARY_EXPORT long strcountsubstr(const char *str, const char* substr);
extern LIBRARY_EXPORT long strcountchar(char* str, const char ch);

extern LIBRARY_EXPORT char* strtolower(char* str);
extern LIBRARY_EXPORT char* strtoupper(char* str);

extern LIBRARY_EXPORT char* strlefttrim(char* str);
extern LIBRARY_EXPORT char* strrighttrim(char* str);
extern LIBRARY_EXPORT char* stralltrim(char* str);

extern LIBRARY_EXPORT char* strremsubstrfirst(char* str, const char* substr);
extern LIBRARY_EXPORT char* strremsubstrall(char* str, const char* substr);
extern LIBRARY_EXPORT char* strremsubstrat(char* str, long pos, long len);

extern LIBRARY_EXPORT char* strremcharfirst(char* str, const char oldchar);
extern LIBRARY_EXPORT char* strremcharall(char* str, const char oldchar);
extern LIBRARY_EXPORT char* strremcharat(char* str, long pos);

extern LIBRARY_EXPORT char* strrepsubstrfirst(char* str, const char* oldsubstr, const char* newsubstr);
extern LIBRARY_EXPORT char* strrepsubstrall(char* str, const char* oldsubstr, const char* newsubstr);

extern LIBRARY_EXPORT char* strrepcharfirst(char* str, const char oldchar, const char newchar);
extern LIBRARY_EXPORT char* strrepcharall(char* str, const char oldchar, const char newchar);
extern LIBRARY_EXPORT char* strrepcharat(char* str, const char newchar, long pos);

extern LIBRARY_EXPORT void strsplitkeyvaluechar(const char* str, const char delimiter, char **key, char **value);
extern LIBRARY_EXPORT void strsplitkeyvaluesubstr(const char* str, const char* delimiter, char **key, char **value);
extern LIBRARY_EXPORT char** strsplitsubstr(const char* str, const char* delimiter);
extern LIBRARY_EXPORT char** strsplitchar(const char* str, const char delimiter);
extern LIBRARY_EXPORT char* strjoinlistwithsubstr(const char **strlist, const char* delimiter);
extern LIBRARY_EXPORT char* strjoinlistwithchar(const char** strlist, const char delimiter);
extern LIBRARY_EXPORT char* strmergelistwithsubstr(const char **strlist, const char* delimiter);
extern LIBRARY_EXPORT char* strmergelistwithchar(const char** strlist, const char delimiter);
extern LIBRARY_EXPORT void  strsortlist(char** strlist);
extern LIBRARY_EXPORT void  strfreelist(char** strlist);

#ifdef __cplusplus
}
#endif

#endif
