#include "dgrep.h"

int gidx = 0;
node_t *nodes[256];
static 
node_t *dgrep_search_trie( dgrep_make_trie_t *trie, uchar_t * key, uchar_t * rest, search_status_t *status, int substring ){

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

static node_t *dgrep_match_trie( dgrep_make_trie_t *trie, uchar_t * key, uchar_t *rest, trie_status_t *status ){

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

node_t *dgrep_make_node( ){

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

int dgrep_make_ynext_array( dgrep_make_trie_t *trie, node_t *node ){

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
		dgrep_make_ynext_array( trie, node->ynext );

	if( node->xnext != NULL )
		dgrep_make_ynext_array( trie, node->xnext );


	return 1;
}

int dgrep_numbering( dgrep_make_trie_t *trie, node_t *node, int depth ){

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
		dgrep_numbering( trie, node->xnext,depth+1 );
	if( node->ynext != NULL )
		dgrep_numbering( trie, node->ynext, depth );

	return 1;
}


void dgrep_make_index_info( dgrep_make_trie_t *trie ){

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





int dgrep_optimization( dgrep_make_trie_t *trie, node_t * node, int first ){

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
		dgrep_optimization( trie, node->xnext, 0 );
	if( node->ynext != NULL )
		dgrep_optimization( trie, node->ynext, 0 );

	return 1;
}

int dgrep_insert_trie( dgrep_make_trie_t *trie, char * key, char *val ){

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
	cnode     = dgrep_match_trie( trie,(uchar_t*) key, rest, &status );
	rest_size = strlen( (char*)rest );
	pRest     = &rest[0];

	if( rest_size >= 1 ){
		nnode       = dgrep_make_node( );
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
		nnode        = dgrep_make_node(  );
		nnode->code  = *( pRest++ );
		cnode->xnext = nnode;
		cnode        = nnode;
		cnode->val   = NULL;
	}
	cnode->val = strdup( val );
	return 1;
}

int dgrep_build_trie( dgrep_make_trie_t* trie ){

	dgrep_optimization ( trie, trie->root,1  );
	dgrep_make_index_info ( trie );
	dgrep_numbering( trie, trie->root, 0 );
	dgrep_make_ynext_array( trie, trie->root );
	return 0;
}

dgrep_make_trie_t *dgrep_make_trie( ){

	dgrep_make_trie_t *trie;
	trie = malloc( sizeof( dgrep_make_trie_t ) );
	trie->root = NULL;
	trie->node_num = 1;
	return trie;
}



dgrep_t *dgrep_read_input_group_file( dgrep_t *dgrep, char * pattern_fn, char *group_name ){

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

	dgrep->trie = NULL;
	/* ************************
	 * read pattern file 
	 * ************************/

	if( pattern_fn != NULL && *pattern_fn != '\0' ){
		if( pattern_fn[0] == '\0' ) return dgrep;
		if((fp = fopen( pattern_fn, "r" )) == NULL ){
				fprintf(stderr," file open error : %s\n", pattern_fn);
				return dgrep;
		}
		dgrep->trie = dgrep_make_trie();
		while( fgets(buffer,KGREP_BUF_LEN,fp) != NULL){
				if(( p = strchr( buffer, '\n' )) != NULL ) *p = '\0';

				if(( p = strchr( buffer, '\t' )) != NULL ) { *p = '\0'; p++; }
				else p = buffer;

				strcpy((char*)key, (char*)buffer );
		strcpy((char*)val, (char*)p );
		if(!dgrep_insert_trie( dgrep->trie, (char*)key, (char*)val ))
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
		if(dgrep->trie == NULL ) dgrep->trie = dgrep_make_trie();
		if( plist != NULL ){
			for( ; *plist != '\0'; plist++ ){
				key[0] = *plist;
				key[1] = '\0';
				strcpy( val, key );
				if(!dgrep_insert_trie( dgrep->trie, (char*)key, (char*)val ))
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
					if(!dgrep_insert_trie( dgrep->trie, (char*)key, (char*)val ))
						fprintf( stderr,"cannt insert trie : %s\n", key );
				}
		}
	}

	if( dgrep->trie != NULL )
	dgrep_build_trie( dgrep->trie );
	return dgrep;
}


node_t *dgrep_free_node( node_t *node ){

	if( node->xnext != NULL )
		node->xnext = dgrep_free_node( node->xnext );

	if( node->ynext != NULL )
		node->ynext = dgrep_free_node( node->ynext );

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

void dgrep_free_trie( dgrep_make_trie_t *trie ){

	if( trie == NULL ) return;
	if( trie->Index_Seek != NULL )
		free( trie->Index_Seek );
	if( trie->Index_List != NULL )
		free( trie->Index_List );
	if( trie->root != NULL )
		dgrep_free_node( trie->root );
	if( trie != NULL )
		free( trie );
	return;
}

void dgrep_free( dgrep_t * dgrep ){
	dgrep_free_trie( dgrep->trie );
	free( dgrep );
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

void print_in_color_all( dgrep_t *dgrep, char *line, int *len, int *pos, int index ){

	int i, j=0, x, s = 0;
	int findex = 0;
	int field_num = dgrep->field_num;
	char *s_string, *e_string;
	char *delimiter = dgrep->delimiter;


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

void dgrep_print_result_all( dgrep_t *dgrep,  op_type_t op_type, char *line, int* pos, int* len, int index ){

	if( dgrep->op_type & COLOR ){
		print_in_color_all( dgrep, line, len, pos, index );
	}else{
		if( isatty( 1 ))
			print_in_color_all( dgrep, line, len, pos, index );
		else
			printf("%s\n", line );
	}
}


void dgrep_print_result( op_type_t op_type,  char *value, char *line, int pos ){

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

int dgrep_prefix_match( dgrep_t * dgrep, char * string, char *line ){

	int found = 0;
	node_t *cnode;
	char rest[KGREP_BUF_LEN];
	search_status_t status;
	if((cnode = dgrep_search_trie( dgrep->trie, (uchar_t*) string, (uchar_t*)rest, &status, 0 )) != NULL ){
		if( cnode->val != NULL && status != NO_MATCH ){
			found++;
			if( !(dgrep->op_type & INVERSE) ){
				dgrep_print_result( dgrep->op_type,  cnode->val, line, 0 );
			}
		}
	}

	if( !found && dgrep->op_type & INVERSE )
		printf("%s\n", line );
	return 0;
}

int dgrep_suffix_match( dgrep_t * dgrep, char * string, char *line ){

	int found = 0, clen;
	node_t  *cnode;
	char rest[KGREP_BUF_LEN], *pstring;
	search_status_t status;
	for( pstring = string; *pstring != '\0'; pstring += clen ){
		if((cnode = dgrep_search_trie( dgrep->trie, (uchar_t*) pstring, (uchar_t*)rest, &status, 0 )) != NULL ){
			if( cnode->val != NULL && status != NO_MATCH && rest[0] == '\0' ){
				found++;
				if( !(dgrep->op_type & INVERSE) ){
					dgrep_print_result( dgrep->op_type,  cnode->val, line, pstring-string );
				}
				if( found >= 1024 ) break;
			}
		}
		clen = c_utflen( *pstring );
	}

	if( !found && dgrep->op_type & INVERSE )
		printf("%s\n", line );
	return 0;
}

int dgrep_substring_match( dgrep_t * dgrep, char * string, char *line ){

	int len[1024];
	int pos[1024], clen;
	int found = 0, i;
	char rest[KGREP_BUF_LEN], *pstring;
	search_status_t status;
	for( pstring = string; *pstring != '\0'; pstring += clen ){
		dgrep_search_trie( dgrep->trie, (uchar_t*) pstring, (uchar_t*)rest, &status, 1 );
		if( gidx > 0 ){
			if( !(dgrep->op_type & INVERSE) ){
				if( (dgrep->op_type & MATCH_PATTERN) ){
					dgrep_print_result( dgrep->op_type,  nodes[gidx-1]->val, line, pstring-string );
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
	if( !(dgrep->op_type & MATCH_PATTERN) ){
		if( dgrep->op_type & INVERSE ){
			if( found == 0 )
				printf("%s\n", line );
		} else {
			if( found > 0 )
				dgrep_print_result_all( dgrep,  dgrep->op_type, line, pos, len, found );
		}
	}

	return 0;
}

int dgrep_exact_match( dgrep_t * dgrep, char * string, char *line ){

	node_t    *cnode;
	int found = 0;
	char rest[KGREP_BUF_LEN];
	search_status_t status;
	printf("%s\n", __FUNCTION__ );
	if((cnode = dgrep_search_trie( dgrep->trie, (uchar_t*) string, (uchar_t*)rest, &status, 0 )) != NULL ){
		if( cnode->val != NULL && status == FULL_MATCH ){
			found++;
			if( !(dgrep->op_type & INVERSE) ){
				dgrep_print_result( dgrep->op_type,  cnode->val, line, 0 );
			}
		}
	}

	if( !found && dgrep->op_type & INVERSE )
		printf("%s\n", line );
	return 0;
}

int dgrep_match_in_line( dgrep_t *dgrep, char* buffer ){

	int findex = 1;
	char line[KGREP_BUF_LEN], *s_string, *e_string;
	strcpy( line, buffer );
	s_string = e_string = buffer;

	for( e_string = strstr( s_string, dgrep->delimiter ), findex=1; \
		e_string != NULL; \
		e_string = strstr( s_string, dgrep->delimiter ), findex++){ 
		e_string[0] = '\0';
		if( findex == dgrep->field_num ){
			if( (dgrep->op_type &  PREFIX ) == PREFIX ) dgrep_prefix_match( dgrep, s_string, line );
			else if( (dgrep->op_type & SUFFIX ) == SUFFIX ) dgrep_suffix_match( dgrep, s_string, line ); 
			else if( (dgrep->op_type & SUBSTRING ) == SUBSTRING ) dgrep_substring_match( dgrep, s_string, line );
			else dgrep_exact_match( dgrep, s_string, line );
		}
		e_string += strlen( dgrep->delimiter );
		s_string = e_string;
	}///end for


	if( findex == dgrep->field_num ){
		if( (dgrep->op_type &  PREFIX ) == PREFIX ) dgrep_prefix_match( dgrep, s_string, line );
		else if( (dgrep->op_type & SUFFIX ) == SUFFIX ) dgrep_suffix_match( dgrep, s_string, line ); 
		else if( (dgrep->op_type & SUBSTRING ) == SUBSTRING ) dgrep_substring_match( dgrep, s_string, line );
		else dgrep_exact_match( dgrep, s_string, line );
	}
	return 0;
}


int dgrep_matching( dgrep_t * dgrep ){

	FILE *fp = dgrep->fp;
	char buffer[KGREP_BUF_LEN], *p;

	while( fgets( buffer, KGREP_BUF_LEN, fp ) != NULL ){
		if(( p = strchr( buffer, '\n' )) != NULL ) *p = '\0';
		dgrep_match_in_line( dgrep, buffer );
	}


	return 0;
}

dgrep_t *dgrep_load_group( dgrep_t *dgrep, char* pattern, char* group ){

	if( ( dgrep->op_type & PATTERN_FILE ) && ( dgrep->op_type & EXPAND )){
		return dgrep_read_input_group_file( dgrep, pattern, group );
	} else if( ( dgrep->op_type & PATTERN_FILE )){
		return dgrep_read_input_group_file( dgrep, pattern, NULL );
	} else if( ( dgrep->op_type & EXPAND )){
		return dgrep_read_input_group_file( dgrep, NULL, group );
	}
	return dgrep;
}

void set_opt( dgrep_t *dgrep, int opt ){
	dgrep->op_type |= opt;
}

dgrep_t *dgrep_make_dgrep( ){
	dgrep_t *dgrep;
	dgrep = malloc(sizeof(dgrep_t));
	memset( dgrep, 0, sizeof(dgrep_t));
	dgrep->fp = stdin;
	dgrep->op_type = NONE;
	dgrep->delimiter[0] = '\n';
	dgrep->field_num = 1;
	dgrep->trie = NULL;

	return dgrep;
}


void set_delimiter( dgrep_t *dgrep, char* delimiter ){
	strcpy( dgrep->delimiter, delimiter );
}


void set_field( dgrep_t *dgrep, char *field ){
	dgrep->field_num = atoi( field );
}

dgrep_t *read_dgrep_opt( dgrep_t *dgrep, int argc, char *argv[] ){

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

	dgrep->field_num = 1;
	strcpy( delimiter, "\t" );
	while( (op = getopt_long( argc, argv, "hf:e:d:k:smv", long_options, &option_index )) != -1 ){
		switch( op ) {
			case 0 :
				if(      !strcmp( "prefix", long_options[option_index].name  )){ set_opt( dgrep, PREFIX);
				}else if( !strcmp( "suffix", long_options[option_index].name )){ set_opt( dgrep, SUFFIX); } break;
			case 'f' : sprintf( pattern, "%s", optarg); set_opt( dgrep, PATTERN_FILE ); break;
			case 'd' : sprintf( delimiter, "%s", optarg); break;
			case 'k' : sprintf( field, "%s", optarg); dgrep->field_num = atoi(field); break;
			case 'e' : sprintf( group, "%s", optarg ); set_opt( dgrep, EXPAND);  break;
			case 's' : set_opt( dgrep, SUBSTRING); break;
			case 'v' : set_opt( dgrep, INVERSE);   break;
			case 'm' : set_opt( dgrep, MATCH_PATTERN); break;
			case 'h' : help( argv[0] ); return dgrep; break;
			default  : break; return dgrep;
		}
	}
	set_delimiter( dgrep, delimiter );
	set_field( dgrep, field );
	dgrep = dgrep_load_group( dgrep, pattern, group );
	return dgrep;
}

int main(int argc, char *argv[]){

	int file_count = 0;
	dgrep_t *dgrep;

	if( argc == 1 ){ return help( argv[0] );  }

	dgrep = dgrep_make_dgrep( );
	if( (dgrep = read_dgrep_opt( dgrep, argc, argv )) == NULL) return 0;

	if( optind < argc ) {
		while( optind < argc ){
			// 패턴 목록 파일을 읽지 못한 경우에는 첫번째 인자를 패턴으로 인식한다
			if( dgrep->trie  ){
				if((dgrep->fp = fopen( argv[optind++], "r" )) == NULL ){
					fprintf( stderr, "[ERROR] : cannot open file : %s\n", argv[optind-1] );
				}else{
					file_count++;
					dgrep_matching( dgrep ); 
					fclose( dgrep->fp );
				}
			}else{
				dgrep->trie = dgrep_make_trie( );
				dgrep_insert_trie( dgrep->trie, argv[optind], argv[optind] );
				dgrep_build_trie( dgrep->trie );
				optind++;
			}
		}
	}

	if( !file_count ) dgrep_matching( dgrep ); 
	dgrep_free( dgrep );
	return 0;
}
