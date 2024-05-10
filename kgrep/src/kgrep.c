#include "kgrep.h"

int gidx = 0;
node_t *nodes[256];
static 
node_t *kgrep_search_trie( kgrep_make_trie_t *trie, uchar_t * key, uchar_t * rest, search_status_t *status, int substring ){

	node_t *cnode;
	node_t *tnode;
	uchar_t *sp;
	int middle = 1;
	if( trie == NULL ) return NULL;

	cnode = trie->root;
	tnode = NULL;
	sp = key;
	*status = NO_MATCH;
	if( substring ) gidx =  0;

	if( cnode == NULL ){
		strcpy((char*)rest, (char*)key );
		*status = NO_MATCH;
		return NULL;
	}

	for( ; ; ){
		if( *sp == '\0' ){
			rest[0] = '\0';
			*status = FULL_MATCH;
			return tnode;
		}


		if( strncmp( (char*)sp, (char*)cnode->str, strlen( (char*)cnode->str )) == 0 ){
			if( substring && cnode != NULL && cnode->val != NULL && gidx < 256  ) nodes[gidx++] = cnode;
			sp += strlen( (char*)cnode->str );
			if( *sp == '\0' ){
				rest[0] = '\0';
				*status = FULL_MATCH; // input word full match
				return cnode;
			}else{

				tnode = cnode;
				cnode = cnode->xnext;

				if( cnode == NULL ){
					strcpy((char*)rest,(char*)sp );
					*status = PARTIAL_MATCH; // input word partial match
					return tnode;
				}else{
					middle = 1;
					continue;
				}
			}

		}


		if( middle  == 1 && ( *sp < *cnode->str ) ){
			strcpy((char*)rest,(char*)sp );
			*status = NO_MATCH;
			return tnode;
		}

		if( cnode->ynext == NULL ){
			strcpy((char*)rest,(char*)sp );
			*status = NO_MATCH;
			return cnode;
		}

		if( ( cnode->ynext != NULL ) && ( *sp < *cnode->ynext->str ) ){
			strcpy((char*)rest,(char*)sp );
			*status = NO_MATCH;
			return cnode;
		}
		tnode  = cnode;
		cnode  = cnode->ynext;
		middle = 0;
	}
}

static node_t *kgrep_match_trie( kgrep_make_trie_t *trie, uchar_t * key, uchar_t *rest, trie_status_t *status ){

	node_t *cnode;
	node_t *tnode;
	uchar_t *sp;
	int	middle = 1;


	cnode = trie->root;
	tnode = NULL;
	sp = key;
	*status = YNEXT;

	if( cnode == NULL ){
		strcpy((char*)rest, (char*)key );
		*status = EMPTY;
		return NULL;
	}

	for( ; ; ){
		if( *sp == '\0' ){
			rest[0] = '\0';
			*status = COMPLETE;
			return tnode;
		}

		if( cnode->code == *sp ){

			sp++;
			if( *sp == '\0' ){
				rest[0] = '\0';
				*status = COMPLETE;
				return cnode;
			}else{

				tnode = cnode;
				cnode = cnode->xnext;

				if( cnode == NULL ){
					strcpy((char*)rest,(char*)sp );
					*status = XNEXT;
					return tnode;
				}else{
					middle = 1;
					continue;
				}
			}
		}

		if( middle  == 1 && ( *sp < cnode->code ) ){

			strcpy((char*)rest,(char*)sp );
			*status = MIDDLE;
			return tnode;
		}

		if( cnode->ynext == NULL ){
			strcpy((char*)rest,(char*)sp );
			*status = YNEXT;
			return cnode;
		}

		if( ( cnode->ynext != NULL ) && ( *sp < cnode->ynext->code ) ){
			strcpy((char*)rest,(char*)sp );
			*status = YNEXT;
			return cnode;
		}
		tnode  = cnode;
		cnode  = cnode->ynext;
		middle = 0;
	}

}

node_t *kgrep_make_node( ){

	node_t * node;

	if((node = ( node_t* )malloc( sizeof( node_t ) )) == NULL){
		fprintf( stderr,"%s:%d\tnode count=%d\tNO MORE SPACE !!! EXIT\n",__FUNCTION__,__LINE__,nodecount );
		exit( 0 );
	}

	if((node->str = malloc( sizeof( uchar_t ) * MAXSTRINGSIZE )) == NULL){
		fprintf( stderr,"%s:%d\tnode count=%d\tNO MORE SPACE !!! EXIT\n",__FUNCTION__,__LINE__,nodecount );
		exit( 0 );
	}


	node->alloc  = 1;
	node->lock   = 0;
	node->str[0] = 0;
	node->size   = 0;
	node->code   = 0;
	node->index  = 0;
	node->ycount = 0;
	node->xnext  = NULL;
	node->ynext  = NULL;
	node->val    = NULL;
	nodecount++;
	return node;
}

int kgrep_make_ynext_array( kgrep_make_trie_t *trie, node_t *node ){

	int i;
	node_t *tnode;
	if( node == NULL ) return -1;
	if( node->ycount > 0 ){

		node->ynexts = malloc( sizeof( make_ynexts_t ) * node->ycount );
		tnode = node;
		for( i = 0; i < node->ycount; i++ ){
			memset( &node->ynexts[i],0,sizeof(make_ynexts_t));
			node->ynexts[i].code    = tnode->code;
			node->ynexts[i].address = trie->Index_Seek[tnode->index];
			tnode = tnode->ynext;

		}
	}
	if( node->ynext != NULL )
		kgrep_make_ynext_array( trie, node->ynext );

	if( node->xnext != NULL )
		kgrep_make_ynext_array( trie, node->xnext );


	return 1;
}

int kgrep_numbering( kgrep_make_trie_t *trie, node_t *node, int depth ){

	if( node == NULL ) return -1;

	if( trie->node_num > nodecount ){
	fprintf( stderr,"INDEX SIZE OVERFLOW :%d\n", nodecount );
	exit( 0 );
	}
	trie->Index_List[trie->node_num] = node;


	if( node->ycount == 0 )
	(  trie->Index_Seek )[ trie->node_num + 1] =
		trie->Index_Seek [ trie->node_num ] + 
		1 + 1 +                                     //ynexts + ycount 
		strlen(  (char*)node->str  ) + 1 +          //size + null
		sizeof(int);                                //data pointer

	else
	(  trie->Index_Seek )[ trie->node_num + 1] =
		trie->Index_Seek [ trie->node_num ]   + 
		1 + sizeof(  make_ynexts_t  ) * node->ycount + //ynexts + ycount 
		strlen(  (char*)node->str  ) + 1 +             //size + null
		sizeof(int);                                   //data pointer


	//fprintf( stderr,"INDEX_SEEK[%d] = %d( %d )\n",trie->node_num+1,( trie->Index_Seek )[trie->node_num+1] ,( trie->Index_Seek )[trie->node_num+1]+ 1024 );

	node->index = trie->node_num;

	trie->node_num++;

	if( node->xnext != NULL )
		kgrep_numbering( trie, node->xnext,depth+1 );
	if( node->ynext != NULL )
		kgrep_numbering( trie, node->ynext, depth );

	return 1;
}


void kgrep_make_index_info( kgrep_make_trie_t *trie ){

	int i;
	if((trie->Index_Seek = (  uint_t *  ) malloc (  sizeof(  uint_t  ) * ( nodecount + 5 ) )) == NULL){
		fprintf( stderr,"%s:%d\tnode count=%d\tNO MORE SPACE !!! EXIT\n",__FUNCTION__,__LINE__,nodecount );
		exit( 0 );
	}


	if((trie->Index_List = (  node_t**  ) malloc (  sizeof(  node_t  ) * ( nodecount + 5 ) )) == NULL){
		fprintf( stderr,"%s:%d\tnode count=%d\tNO MORE SPACE !!! EXIT\n",__FUNCTION__,__LINE__,nodecount );
		exit( 0 );
	}

	for( i = 0; i < nodecount + 5; i++ ) trie->Index_Seek[i] = 0;
}





int kgrep_optimization( kgrep_make_trie_t *trie, node_t * node, int first ){

	node_t *tnode;

	if( node == NULL ) return -1;

	node->size = 0;
	node->str[node->size++] = node->code;

	while( 1 ){
		if( node->xnext != NULL && node->xnext->ynext == NULL && node->val == 0 ){
			if( node->size >= ( ( node->alloc * MAXSTRINGSIZE ) )-1 ){
				node->alloc++;
				if( ( node->str = realloc( node->str,sizeof( uchar_t )*( MAXSTRINGSIZE * node->alloc ) ) ) == NULL ){
					fprintf( stderr," cannot memory allocation : %ld\n", sizeof( uchar_t )*( MAXSTRINGSIZE * node->alloc )  );;
					exit( 0 );
				}
			}
			node->str[node->size++]   = node->xnext->code;
			node->val                 = node->xnext->val;
			tnode                     = node->xnext;
			node->xnext               = node->xnext->xnext;//A->B->C?? A->C??
			tnode->str[tnode->size+1] = '\0';

			free( tnode->str );
			free( tnode );
			nodecount--;
		}
		else{ break; }
	}
	if( node->size >= ( ( node->alloc * MAXSTRINGSIZE ) )-1 ){
		node->alloc++;
		if( node->size >= ( uchar_t )255 ){
			fprintf( stderr," string too long : 255\n" );
			exit( 0 );
		}
		node->str[node->size] = '\0';

		if( ( node->str = realloc( node->str,sizeof( uchar_t )*( MAXSTRINGSIZE * node->alloc ) ) ) == NULL ){
			fprintf( stderr," cannot memory allocation : %d\n", ( MAXSTRINGSIZE * node->alloc ) );;
			exit( 0 );
		}
	}


	node->str[node->size] = '\0';

	if( node->ynext != NULL ){
		tnode = node;
		while( tnode != NULL && tnode->lock != 1 ){
			tnode->lock = 1;
			if( first == 0 )
				node->ycount++;
			tnode = tnode->ynext;
		}
	}

	if( node->xnext != NULL )
		kgrep_optimization( trie, node->xnext, 0 );
	if( node->ynext != NULL )
		kgrep_optimization( trie, node->ynext, 0 );

	return 1;
}

int kgrep_insert_trie( kgrep_make_trie_t *trie, char * key, char *val ){

	uchar_t* pRest;
	uchar_t rest[KGREP_BUF_LEN];
	trie_status_t status;

	node_t *nnode;
	node_t *cnode;

	int i         = 0;
	int rest_size = 0;

	if( *key == '\0' ){
		fprintf(stderr,"[%s:%d]\tkey is empty string\n", __FUNCTION__,__LINE__);
		return 0;
	}

	rest[0]   = '\0';
	cnode     = kgrep_match_trie( trie,(uchar_t*) key, rest, &status );
	rest_size = strlen( (char*)rest );
	pRest     = &rest[0];

	if( rest_size >= 1 ){
		nnode       = kgrep_make_node( );
		nnode->code = *pRest;
		pRest++;


		if( cnode == NULL ){
				nnode->ynext = trie->root;
				trie->root   = nnode;
		}else{
			switch( status ){
					case XNEXT :
						cnode->xnext  = nnode;          break;
					case YNEXT :
							nnode->ynext = cnode->ynext;
							cnode->ynext = nnode;           break;
					case MIDDLE :
							nnode->ynext = cnode->xnext;
							cnode->xnext = nnode;  break;
					case COMPLETE:
					case EMPTY:              break;

			}
		}
		cnode = nnode;
	}

	for( i = 1; i < rest_size; i++ ){
		nnode        = kgrep_make_node(  );
		nnode->code  = *( pRest++ );
		cnode->xnext = nnode;
		cnode        = nnode;
		cnode->val   = NULL;
	}
	cnode->val = strdup( val );
	return 1;
}

int kgrep_build_trie( kgrep_make_trie_t* trie ){

	kgrep_optimization ( trie, trie->root,1  );
	kgrep_make_index_info ( trie );
	kgrep_numbering( trie, trie->root, 0 );
	kgrep_make_ynext_array( trie, trie->root );
	return 0;
}

kgrep_make_trie_t *kgrep_make_trie( ){

	kgrep_make_trie_t *trie;
	trie = malloc( sizeof( kgrep_make_trie_t ) );
	trie->root = NULL;
	trie->node_num = 1;
	return trie;
}



kgrep_t *kgrep_read_input_group_file( kgrep_t *kgrep, char * pattern_fn, char *group_name ){

	FILE *fp;
	char buffer[KGREP_BUF_LEN]= {0};
	char *p = NULL, *token, *bp, *plist = NULL;
	char key[KGREP_BUF_LEN]   = {0};
	char val[KGREP_BUF_LEN]   = {0};

	char DIGIT[] = {"0123456789"};
	char LOWER[] = {"abcdefghijklmnopqrstuvwxyz"};
	char UPPER[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
	char ALPHA[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
	//char ALNUM[] = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
	char JAUM[] = {"ㄱ ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ"};
	char MOUM[] = {"ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ"};
	char JAMO[] = {"ㄱ ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ"};

	kgrep->trie = NULL;
	/* ************************
	 * read pattern file 
	 * ************************/

	if( pattern_fn != NULL && *pattern_fn != '\0' ){
		if( pattern_fn[0] == '\0' ) return kgrep;
		if((fp = fopen( pattern_fn, "r" )) == NULL ){
				fprintf(stderr," file open error : %s\n", pattern_fn);
				return kgrep;
		}
		kgrep->trie = kgrep_make_trie();
		while( fgets(buffer,KGREP_BUF_LEN,fp) != NULL){
				if(( p = strchr( buffer, '\n' )) != NULL ) *p = '\0';

				if(( p = strchr( buffer, '\t' )) != NULL ) { *p = '\0'; p++; }
				else p = buffer;

				strcpy((char*)key, (char*)buffer );
		strcpy((char*)val, (char*)p );
		if(!kgrep_insert_trie( kgrep->trie, (char*)key, (char*)val ))
			fprintf( stderr,"cannt insert trie : %s\n", key );
		}
		fclose(fp);
	}


	/* ************************
	 * read group file 
	 * ************************/
	if( group_name != NULL && *group_name != '\0' ){
		if( strcasecmp( group_name, "DIGIT" ) == 0 ) plist = DIGIT; 
		else if( strcasecmp( group_name, "LOWER" ) == 0 ) plist = LOWER; 
		else if( strcasecmp( group_name, "UPPER" ) == 0 ) plist = UPPER; 
		else if( strcasecmp( group_name, "ALPHA" ) == 0 ) plist = ALPHA; 
		if(kgrep->trie == NULL ) kgrep->trie = kgrep_make_trie();
		if( plist != NULL ){
			for( ; *plist != '\0'; plist++ ){
				key[0] = *plist;
				key[1] = '\0';
				strcpy( val, key );
				if(!kgrep_insert_trie( kgrep->trie, (char*)key, (char*)val ))
					fprintf( stderr,"cannt insert trie : %s\n", key );
			}
		} else {
			if( strcasecmp( group_name, "JAUM" ) == 0 ) plist = JAUM; 
			else if( strcasecmp( group_name, "MOUM" ) == 0 ) plist = MOUM; 
			else if( strcasecmp( group_name, "JAMO" ) == 0 ) plist = JAMO; 

			if( plist != NULL )
				for( token = strtok_r( plist, " ", &bp ); token != NULL; token = strtok_r( NULL, " ", &bp )){
					strcpy( key, token );
					strcpy( val, key );
					if(!kgrep_insert_trie( kgrep->trie, (char*)key, (char*)val ))
						fprintf( stderr,"cannt insert trie : %s\n", key );
				}
		}
	}

	if( kgrep->trie != NULL )
	kgrep_build_trie( kgrep->trie );
	return kgrep;
}


node_t *kgrep_free_node( node_t *node ){

	if( node->xnext != NULL )
		node->xnext = kgrep_free_node( node->xnext );

	if( node->ynext != NULL )
		node->ynext = kgrep_free_node( node->ynext );

	if(node->ycount > 0)
		free( node->ynexts );

	if(node->str != NULL )
		free( node->str );

	if( node->val != NULL )
		free( node->val );
	free( node );
	nodecount--;
	return NULL;
}

void kgrep_free_trie( kgrep_make_trie_t *trie ){

	if( trie == NULL ) return;
	if( trie->Index_Seek != NULL )
		free( trie->Index_Seek );
	if( trie->Index_List != NULL )
		free( trie->Index_List );
	if( trie->root != NULL )
		kgrep_free_node( trie->root );
	if( trie != NULL )
		free( trie );
	return;
}

void kgrep_free( kgrep_t * kgrep ){
	kgrep_free_trie( kgrep->trie );
	free( kgrep );
}


/*********************************************************
 *                        TRIE                            *
 *********************************************************/



int help( char * program ){

	fprintf(stderr,"%s version 1.0.14\n" , program );
	fprintf(stderr,"%s [option] PATTERN [FILE]\n" , program );
	fprintf(stderr,"    -h, --help          : 도움말\n" );
	fprintf(stderr,"    -f, --file          : 찾을 패턴이 있는 파일명(value)\n");
	fprintf(stderr,"                          패턴파일명\n");
	fprintf(stderr,"    -e, --expand        : 기정의된 패턴(value)\n");
	fprintf(stderr,"                          패턴타입(DIGIT,LOWER,UPPER,ALPHA,JAUM,MOUM,JAMO)\n");
	fprintf(stderr,"    -d, --delimiter     : 구분자(value)\n");
	fprintf(stderr,"                          멀티바이트가능\n");
	fprintf(stderr,"    -k, --field         : 매칭 대상이 되는 필드 번호(value)\n");
	fprintf(stderr,"                          필드 번호\n");
	fprintf(stderr,"    -m, --match-pattern : 매칭된 패턴을 맨 앞에 출력\n");
	fprintf(stderr,"    -v, --inverse       : 매칭이 안된 라인을 출력\n");
	fprintf(stderr,"    --prefix            : 전방일치 매칭\n");
	fprintf(stderr,"    --suffix            : 후방일치 매칭\n");
	fprintf(stderr,"    -s, --substring     : 부분 문자열( 해당 옵션이 off일때는 exact matching )\n");
	return 0;
}


void print_in_color( char *line, int vsize, int pos ){

	int i, color=0, v=0;
	for( i = 0; line[i] != '\0'; i++ ){
		if( i == pos ){ 
			printf("%s", GREEN); color=1; v = vsize;
		}
		printf("%c", line[i] );
		if( color ) v--;
		if( color && !v ) { 
			printf("%s", RESET ); color = 0; 
		}
	}
	printf("\n");
}


int is_bold_position( int i, int* pos, int index ){

	int j;
	for( j = 0; j < index; j++ ){
		if( i == pos[j] ){ 
			return j;
		}
		if( i < pos[j] ) break;
	}
	return -1;
}

void print_in_color_all( kgrep_t *kgrep, char *line, int *len, int *pos, int index ){

	int i, j=0, x, s = 0;
	int findex = 0;
	int field_num = kgrep->field_num;
	char *s_string, *e_string;
	char *delimiter = kgrep->delimiter;


	s_string = e_string = line;

	for( e_string = strstr( s_string, delimiter ), findex=1; ; e_string = strstr( s_string, delimiter ), findex++){
		if( e_string!=NULL) e_string[0] = '\0';
		if( findex > 1 ) printf("%s", delimiter );
		if( findex == field_num ){
			for( i = 0; s_string[i] != '\0'; i++ ){
				if( (j = is_bold_position( i, &pos[s], index-s )) >= 0 ){
					printf( "%s", GREEN );
					for( x = 0; x < len[j+s]; x++ )   printf("%c", s_string[i+x] );
					i += (x-1);
					s = j+1;
					printf("%s", RESET );
				} else {
					printf("%c", s_string[i] );
				}
			}

		} else {
			printf("%s", s_string );
		}
		if( e_string == NULL ) break;
		e_string += strlen( delimiter );
		s_string = e_string;
	}///end for
	printf("\n");
}

void kgrep_print_result_all( kgrep_t *kgrep,  op_type_t op_type, char *line, int* pos, int* len, int index ){

	if( kgrep->op_type & COLOR ){
		print_in_color_all( kgrep, line, len, pos, index );
	}else{
		if( isatty( 1 ))
			print_in_color_all( kgrep, line, len, pos, index );
		else
			printf("%s\n", line );
	}
}


void kgrep_print_result( op_type_t op_type,  char *value, char *line, int pos ){

	if( isatty( 1 ))
		printf("%s%s%s\t%s\n", RED, value, RESET, line );
	else
		printf("%s\t%s\n", value, line );
}

int c_utflen( char c ){
	/*11000000      0XC0
	  11100000        0XE0
	  11110000        0XF0
	  11111000        0XF8
	  11111100        0XFC*/
	if ( c == '\0' )     return 1;
	else if ( (  c & 0x80) == 0x00 )     return 1;
	else if( (c & 0xE0) == 0xC0 )   return 2;
	else if( (c & 0xF0) == 0xE0 )   return 3;
	else if( (c & 0xF8) == 0xF0 )   return 4;
	else if( (c & 0xFC) == 0xF8 )   return 5;
	else if( (c & 0xFE) == 0xFC )   return 6;

	return 1;
}

int kgrep_prefix_match( kgrep_t * kgrep, char * string, char *line ){

	int found = 0;
	node_t *cnode;
	char rest[KGREP_BUF_LEN];
	search_status_t status;
	if((cnode = kgrep_search_trie( kgrep->trie, (uchar_t*) string, (uchar_t*)rest, &status, 0 )) != NULL ){
		if( cnode->val != NULL && status != NO_MATCH ){
			found++;
			if( !(kgrep->op_type & INVERSE) ){
				kgrep_print_result( kgrep->op_type,  cnode->val, line, 0 );
			}
		}
	}

	if( !found && kgrep->op_type & INVERSE )
		printf("%s\n", line );
	return 0;
}

int kgrep_suffix_match( kgrep_t * kgrep, char * string, char *line ){

	int found = 0, clen;
	node_t  *cnode;
	char rest[KGREP_BUF_LEN], *pstring;
	search_status_t status;
	for( pstring = string; *pstring != '\0'; pstring += clen ){
		if((cnode = kgrep_search_trie( kgrep->trie, (uchar_t*) pstring, (uchar_t*)rest, &status, 0 )) != NULL ){
			if( cnode->val != NULL && status != NO_MATCH && rest[0] == '\0' ){
				found++;
				if( !(kgrep->op_type & INVERSE) ){
					kgrep_print_result( kgrep->op_type,  cnode->val, line, pstring-string );
				}
				if( found >= 1024 ) break;
			}
		}
		clen = c_utflen( *pstring );
	}

	if( !found && kgrep->op_type & INVERSE )
		printf("%s\n", line );
	return 0;
}

int kgrep_substring_match( kgrep_t * kgrep, char * string, char *line ){

	int len[1024];
	int pos[1024], clen;
	int found = 0, i;
	char rest[KGREP_BUF_LEN], *pstring;
	search_status_t status;
	for( pstring = string; *pstring != '\0'; pstring += clen ){
		kgrep_search_trie( kgrep->trie, (uchar_t*) pstring, (uchar_t*)rest, &status, 1 );
		if( gidx > 0 ){
			if( !(kgrep->op_type & INVERSE) ){
				if( (kgrep->op_type & MATCH_PATTERN) ){
					kgrep_print_result( kgrep->op_type,  nodes[gidx-1]->val, line, pstring-string );
				} else {
					for( i = 0; i < gidx; i++ ){
						pos[found] = pstring-string;
						if( nodes[i]->val != NULL )
							len[found] = strlen(nodes[i]->val);
					}
				}
			}
			found++;
			if( found >= 1024 ) break;
		}
		clen = c_utflen( *pstring );
	}
	if( !(kgrep->op_type & MATCH_PATTERN) ){
		if( kgrep->op_type & INVERSE ){
			if( found == 0 )
				printf("%s\n", line );
		} else {
			if( found > 0 )
				kgrep_print_result_all( kgrep,  kgrep->op_type, line, pos, len, found );
		}
	}

	return 0;
}

int kgrep_exact_match( kgrep_t * kgrep, char * string, char *line ){

	node_t    *cnode;
	int found = 0;
	char rest[KGREP_BUF_LEN];
	search_status_t status;
	printf("%s\n", __FUNCTION__ );
	if((cnode = kgrep_search_trie( kgrep->trie, (uchar_t*) string, (uchar_t*)rest, &status, 0 )) != NULL ){
		if( cnode->val != NULL && status == FULL_MATCH ){
			found++;
			if( !(kgrep->op_type & INVERSE) ){
				kgrep_print_result( kgrep->op_type,  cnode->val, line, 0 );
			}
		}
	}

	if( !found && kgrep->op_type & INVERSE )
		printf("%s\n", line );
	return 0;
}

int kgrep_match_in_line( kgrep_t *kgrep, char* buffer ){

	int findex = 1;
	char line[KGREP_BUF_LEN], *s_string, *e_string;
	strcpy( line, buffer );
	s_string = e_string = buffer;

	for( e_string = strstr( s_string, kgrep->delimiter ), findex=1; \
		e_string != NULL; \
		e_string = strstr( s_string, kgrep->delimiter ), findex++){ 
		e_string[0] = '\0';
		if( findex == kgrep->field_num ){
			if( (kgrep->op_type &  PREFIX ) == PREFIX ) kgrep_prefix_match( kgrep, s_string, line );
			else if( (kgrep->op_type & SUFFIX ) == SUFFIX ) kgrep_suffix_match( kgrep, s_string, line ); 
			else if( (kgrep->op_type & SUBSTRING ) == SUBSTRING ) kgrep_substring_match( kgrep, s_string, line );
			else kgrep_exact_match( kgrep, s_string, line );
		}
		e_string += strlen( kgrep->delimiter );
		s_string = e_string;
	}///end for


	if( findex == kgrep->field_num ){
		if( (kgrep->op_type &  PREFIX ) == PREFIX ) kgrep_prefix_match( kgrep, s_string, line );
		else if( (kgrep->op_type & SUFFIX ) == SUFFIX ) kgrep_suffix_match( kgrep, s_string, line ); 
		else if( (kgrep->op_type & SUBSTRING ) == SUBSTRING ) kgrep_substring_match( kgrep, s_string, line );
		else kgrep_exact_match( kgrep, s_string, line );
	}
	return 0;
}


int kgrep_matching( kgrep_t * kgrep ){

	FILE *fp = kgrep->fp;
	char buffer[KGREP_BUF_LEN], *p;

	while( fgets( buffer, KGREP_BUF_LEN, fp ) != NULL ){
		if(( p = strchr( buffer, '\n' )) != NULL ) *p = '\0';
		kgrep_match_in_line( kgrep, buffer );
	}


	return 0;
}

kgrep_t *kgrep_load_group( kgrep_t *kgrep, char* pattern, char* group ){

	if( ( kgrep->op_type & PATTERN_FILE ) && ( kgrep->op_type & EXPAND )){
		return kgrep_read_input_group_file( kgrep, pattern, group );
	} else if( ( kgrep->op_type & PATTERN_FILE )){
		return kgrep_read_input_group_file( kgrep, pattern, NULL );
	} else if( ( kgrep->op_type & EXPAND )){
		return kgrep_read_input_group_file( kgrep, NULL, group );
	}
	return kgrep;
}

void set_opt( kgrep_t *kgrep, int opt ){
	kgrep->op_type |= opt;
}

kgrep_t *kgrep_make_kgrep( ){
	kgrep_t *kgrep;
	kgrep = malloc(sizeof(kgrep_t));
	memset( kgrep, 0, sizeof(kgrep_t));
	kgrep->fp = stdin;
	kgrep->op_type = NONE;
	kgrep->delimiter[0] = '\n';
	kgrep->field_num = 1;
	kgrep->trie = NULL;

	return kgrep;
}


void set_delimiter( kgrep_t *kgrep, char* delimiter ){
	strcpy( kgrep->delimiter, delimiter );
}


void set_field( kgrep_t *kgrep, char *field ){
	kgrep->field_num = atoi( field );
}

kgrep_t *read_kgrep_opt( kgrep_t *kgrep, int argc, char *argv[] ){

	int op;
	int option_index = 0;
	char delimiter[8];
	char field[KGREP_BUF_LEN]={0};
	char group[KGREP_BUF_LEN]={0};
	char pattern[KGREP_BUF_LEN]={0};
	static struct option long_options[11] =  {
	{"help",         no_argument, 0, 'h'},
	{"file",         no_argument, 0, 'f'},
	{"expand",       no_argument, 0, 'e'},
	{"delimiter",    no_argument, 0, 'd'},
	{"field",        no_argument, 0, 'k'},
	{"prefix",       no_argument, 0,  0 },
	{"suffix",       no_argument, 0,  0 },
	{"substring",    no_argument, 0, 's'},
	{"inverse",      no_argument, 0, 'v'},
	{"match",        no_argument, 0, 'm'},
	{0, 0, 0, 0} };

	kgrep->field_num = 1;
	strcpy( delimiter, "\t" );
	while( (op = getopt_long( argc, argv, "hf:e:d:k:smv", long_options, &option_index )) != -1 ){
		switch( op ) {
			case 0 :
				if(      !strcmp( "prefix", long_options[option_index].name  )){ set_opt( kgrep, PREFIX);
				}else if( !strcmp( "suffix", long_options[option_index].name )){ set_opt( kgrep, SUFFIX); } break;
			case 'f' : sprintf( pattern, "%s", optarg); set_opt( kgrep, PATTERN_FILE ); break;
			case 'd' : sprintf( delimiter, "%s", optarg); break;
			case 'k' : sprintf( field, "%s", optarg); kgrep->field_num = atoi(field); break;
			case 'e' : sprintf( group, "%s", optarg ); set_opt( kgrep, EXPAND);  break;
			case 's' : set_opt( kgrep, SUBSTRING); break;
			case 'v' : set_opt( kgrep, INVERSE);   break;
			case 'm' : set_opt( kgrep, MATCH_PATTERN); break;
			case 'h' : help( argv[0] ); return kgrep; break;
			default  : break; return kgrep;
		}
	}
	set_delimiter( kgrep, delimiter );
	set_field( kgrep, field );
	kgrep = kgrep_load_group( kgrep, pattern, group );
	return kgrep;
}

int main(int argc, char *argv[]){

	int file_count = 0;
	kgrep_t *kgrep;

	if( argc == 1 ){ return help( argv[0] );  }

	kgrep = kgrep_make_kgrep( );
	if( (kgrep = read_kgrep_opt( kgrep, argc, argv )) == NULL) return 0;

	if( optind < argc ) {
		while( optind < argc ){
			// 패턴 목록 파일을 읽지 못한 경우에는 첫번째 인자를 패턴으로 인식한다
			if( kgrep->trie  ){
				if((kgrep->fp = fopen( argv[optind++], "r" )) == NULL ){
					fprintf( stderr, "[ERROR] : cannot open file : %s\n", argv[optind-1] );
				}else{
					file_count++;
					kgrep_matching( kgrep ); 
					fclose( kgrep->fp );
				}
			}else{
				kgrep->trie = kgrep_make_trie( );
				kgrep_insert_trie( kgrep->trie, argv[optind], argv[optind] );
				kgrep_build_trie( kgrep->trie );
				optind++;
			}
		}
	}

	if( !file_count ) kgrep_matching( kgrep ); 
	kgrep_free( kgrep );
	return 0;
}
