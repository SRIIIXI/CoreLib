/*

Copyright (c) 2020, CIMCON Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, is allowed only with prior permission from CIMCON Automation

*/

#include "Dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct key_value_t
{
    void* key;
    void* value;
    struct key_value_t* next;
}key_value_t;

typedef struct hash_bucket_t
{
    void* hash;
    long key_value_count;
    key_value_t* key_value_list;
    struct section_t* next;
}hash_bucket_t;

typedef struct dictionary_t
{
    long hash_count;
    hash_bucket_t* hash_bucket;
}dictionary_t;

//typedef struct _dictionary_ {
//    int             n ;     /** Number of entries in dictionary */
//    ssize_t         size ;  /** Storage size */
//    char        **  val ;   /** List of string values */
//    char        **  key ;   /** List of string keys */
//    unsigned     *  hash ;  /** List of hash values for keys */
//} dictionary ;

///** Maximum value size for integers and doubles. */
//#define MAXVALSZ    1024

///** Minimal allocated number of entries in a dictionary */
//#define DICTMINSZ   128

///** Invalid key token */
//#define DICT_INVALID_KEY    ((char*)-1)

//static char * xstrdup(const char * s)
//{
//    char * t ;
//    size_t len ;
//    if (!s)
//        return NULL ;

//    len = strlen(s) + 1 ;
//    t = (char*) malloc(len) ;
//    if (t) {
//        memcpy(t, s, len) ;
//    }
//    return t ;
//}

//static int dictionary_grow(void * dptr)
//{
//    dictionary * d = (dictionary*)dptr;

//    char        ** new_val ;
//    char        ** new_key ;
//    unsigned     * new_hash ;

//    new_val  = (char**) calloc(d->size * 2, sizeof *d->val);
//    new_key  = (char**) calloc(d->size * 2, sizeof *d->key);
//    new_hash = (unsigned*) calloc(d->size * 2, sizeof *d->hash);
//    if (!new_val || !new_key || !new_hash) {
//        /* An allocation failed, leave the dictionary unchanged */
//        if (new_val)
//            free(new_val);
//        if (new_key)
//            free(new_key);
//        if (new_hash)
//            free(new_hash);
//        return -1 ;
//    }
//    /* Initialize the newly allocated space */
//    memcpy(new_val, d->val, d->size * sizeof(char *));
//    memcpy(new_key, d->key, d->size * sizeof(char *));
//    memcpy(new_hash, d->hash, d->size * sizeof(unsigned));
//    /* Delete previous data */
//    free(d->val);
//    free(d->key);
//    free(d->hash);
//    /* Actually update the dictionary */
//    d->size *= 2 ;
//    d->val = new_val;
//    d->key = new_key;
//    d->hash = new_hash;
//    return 0 ;
//}

//unsigned dictionary_hash(const char * key)
//{
//    size_t      len ;
//    unsigned    hash ;
//    size_t      i ;

//    if (!key)
//        return 0 ;

//    len = strlen(key);
//    for (hash=0, i=0 ; i<len ; i++) {
//        hash += (unsigned)key[i] ;
//        hash += (hash<<10);
//        hash ^= (hash>>6) ;
//    }
//    hash += (hash <<3);
//    hash ^= (hash >>11);
//    hash += (hash <<15);
//    return hash ;
//}

//void * dictionary_new(size_t size)
//{
//    dictionary  *   d ;

//    /* If no size was specified, allocate space for DICTMINSZ */
//    if (size<DICTMINSZ) size=DICTMINSZ ;

//    d = (dictionary*) calloc(1, sizeof *d) ;

//    if (d) {
//        d->size = size ;
//        d->val  = (char**) calloc(size, sizeof *d->val);
//        d->key  = (char**) calloc(size, sizeof *d->key);
//        d->hash = (unsigned*) calloc(size, sizeof *d->hash);
//    }
//    return d ;
//}

//void dictionary_del(void * dptr)
//{
//    dictionary * d = (dictionary*)dptr;

//    ssize_t  i ;

//    if (d==NULL) return ;
//    for (i=0 ; i<d->size ; i++) {
//        if (d->key[i]!=NULL)
//            free(d->key[i]);
//        if (d->val[i]!=NULL)
//            free(d->val[i]);
//    }
//    free(d->val);
//    free(d->key);
//    free(d->hash);
//    free(d);
//    return ;
//}

//const char * dictionary_get(const void * dptr, const char * key, const char * def)
//{
//    dictionary * d = (dictionary*)dptr;

//    unsigned    hash ;
//    ssize_t      i ;

//    hash = dictionary_hash(key);
//    for (i=0 ; i<d->size ; i++) {
//        if (d->key[i]==NULL)
//            continue ;
//        /* Compare hash */
//        if (hash==d->hash[i]) {
//            /* Compare string, to avoid hash collisions */
//            if (!strcmp(key, d->key[i])) {
//                return d->val[i] ;
//            }
//        }
//    }
//    return def ;
//}

//int dictionary_set(void * dptr, const char * key, const char * val)
//{
//    dictionary * d = (dictionary*)dptr;

//    ssize_t         i ;
//    unsigned       hash ;

//    if (d==NULL || key==NULL) return -1 ;

//    /* Compute hash for this key */
//    hash = dictionary_hash(key) ;
//    /* Find if value is already in dictionary */
//    if (d->n>0) {
//        for (i=0 ; i<d->size ; i++) {
//            if (d->key[i]==NULL)
//                continue ;
//            if (hash==d->hash[i]) { /* Same hash value */
//                if (!strcmp(key, d->key[i])) {   /* Same key */
//                    /* Found a value: modify and return */
//                    if (d->val[i]!=NULL)
//                        free(d->val[i]);
//                    d->val[i] = (val ? xstrdup(val) : NULL);
//                    /* Value has been modified: return */
//                    return 0 ;
//                }
//            }
//        }
//    }
//    /* Add a new value */
//    /* See if dictionary needs to grow */
//    if (d->n==d->size) {
//        /* Reached maximum size: reallocate dictionary */
//        if (dictionary_grow(d) != 0)
//            return -1;
//    }

//    /* Insert key in the first empty slot. Start at d->n and wrap at
//       d->size. Because d->n < d->size this will necessarily
//       terminate. */
//    for (i=d->n ; d->key[i] ; ) {
//        if(++i == d->size) i = 0;
//    }
//    /* Copy key */
//    d->key[i]  = xstrdup(key);
//    d->val[i]  = (val ? xstrdup(val) : NULL) ;
//    d->hash[i] = hash;
//    d->n ++ ;
//    return 0 ;
//}

//void dictionary_unset(void * dptr, const char * key)
//{
//    dictionary * d = (dictionary*)dptr;

//    unsigned    hash ;
//    ssize_t      i ;

//    if (key == NULL || d == NULL) {
//        return;
//    }

//    hash = dictionary_hash(key);
//    for (i=0 ; i<d->size ; i++) {
//        if (d->key[i]==NULL)
//            continue ;
//        /* Compare hash */
//        if (hash==d->hash[i]) {
//            /* Compare string, to avoid hash collisions */
//            if (!strcmp(key, d->key[i])) {
//                /* Found key */
//                break ;
//            }
//        }
//    }
//    if (i>=d->size)
//        /* Key not found */
//        return ;

//    free(d->key[i]);
//    d->key[i] = NULL ;
//    if (d->val[i]!=NULL) {
//        free(d->val[i]);
//        d->val[i] = NULL ;
//    }
//    d->hash[i] = 0 ;
//    d->n -- ;
//    return ;
//}

//void dictionary_dump(const void * dptr, FILE * out)
//{
//    dictionary * d = (dictionary*)dptr;

//    ssize_t  i ;

//    if (d==NULL || out==NULL) return ;
//    if (d->n<1) {
//        fprintf(out, "empty dictionary\n");
//        return ;
//    }
//    for (i=0 ; i<d->size ; i++) {
//        if (d->key[i]) {
//            fprintf(out, "%20s\t[%s]\n",
//                    d->key[i],
//                    d->val[i] ? d->val[i] : "UNDEF");
//        }
//    }
//    return ;
//}

dictionary_t* dictionary_allocate()
{
    return NULL;
}

void dictionary_free(dictionary_t* dict_ptr)
{

}

void dictionary_set_value(dictionary_t* dict_ptr, const char* key, const void* value)
{

}

void* dictionary_get_value(dictionary_t* dict_ptr, const char* key)
{
    return NULL;
}