#ifndef __DGREP_H__
#define __DGREP_H___

//#include <google/profiler.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "color.h"

#define PUBLIC 
#define PRIVATE static

#define DGREP_BUF_LEN 1024000


/*********************************************************
*                        TRIE                            *
*********************************************************/
#define FIRST_NODE_COUNT    256
#define MAXSTRINGSIZE 32
#ifndef uchar_t
#define uchar_t unsigned char
#endif

#ifndef uint_t
#define uint_t unsigned int
#endif

uint_t nodecount = 0;
typedef enum trie_status{
	EMPTY     = 0,
	XNEXT     = 1,
	YNEXT     = 2,
	COMPLETE  = 3,
	MIDDLE    = 4
}trie_status_t;

typedef enum search_status{
	FULL_MATCH    = 1,
	PARTIAL_MATCH = 2,
	NO_MATCH      = 3,
}search_status_t;

typedef enum keyvalue{
	KV_KEY = 1,
	KV_VAL = 2
}keyvalue_t;

typedef struct ynext{
	uchar_t code;
	uint_t address;
}make_ynexts_t;

typedef struct node{
	uint_t  index;
	uchar_t* str;
	uchar_t  code;
	uchar_t  lock;
	uchar_t  size;
	uchar_t  alloc;
	uchar_t  ycount;

	struct node* xnext;
	struct node* ynext;
	make_ynexts_t* ynexts;
	char* val;
}node_t;


typedef struct _kgrep_make_trie_{
	node_t*  root;
	uint_t node_num;
	node_t** Index_List;
	uint_t*  Index_Seek;
}kgrep_make_trie_t;


typedef enum type{
	NONE    = 0,
	PREFIX  = 1,
	SUFFIX  = 2,
	SUBSTRING=4,
	INVERSE = 8,
	MATCH_PATTERN = 16,
	PATTERN_FILE = 32,
	EXPAND = 64,
	COLOR  = 128,
}op_type_t;


typedef struct kgrep{
	FILE *fp;
	op_type_t op_type;
	char delimiter[8];
	int field_num;
	kgrep_make_trie_t *trie;
}kgrep_t;


#endif
