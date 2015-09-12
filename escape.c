/*
 * Copyright (c) 2011 Sergey Kolotsey.
 * This file if part of expat-dom library.
 * See the file COPYING for copying permission.
 *
 * This file defines functions that help converting any strings into
 * XML-friendly strings.
 */

#include "expat-config.h"

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif



#define LT "&lt;"
#define LT_LEN 4
#define GT "&gt;"
#define GT_LEN 4
#define APOS "&apos;"
#define APOS_LEN 6
#define QUOT "&quot;"
#define QUOT_LEN 6
#define AMP "&amp;"
#define AMP_LEN 5

#define IS_LT(s, s_len) (((s_len)>=LT_LEN) && 0==strncmp((s), LT, LT_LEN))
#define IS_GT(s, s_len) (((s_len)>=GT_LEN) && 0==strncmp((s), GT, GT_LEN))
#define IS_APOS(s, s_len) (((s_len)>=APOS_LEN) && 0==strncmp((s), APOS, APOS_LEN))
#define IS_QUOT(s, s_len) (((s_len)>=QUOT_LEN) && 0==strncmp((s), QUOT, QUOT_LEN))
#define IS_AMP(s, s_len) (((s_len)>=AMP_LEN) && 0==strncmp((s), AMP, AMP_LEN))



/*
 * Unescape XML string
 *
 * input and output may be the same buffers
 * return len of output
 *
 * "&lt;"   -> "<"
 * "&gt;"   -> "<"
 * "&apos;" -> "'"
 * "&quot;" -> "\""
 * "&amp;"  -> "&"
 */
int unescape_xml_r( const char *input, int input_len, char *output){
	int output_len=0;

	while(input_len){
		if(*input=='&'){
			if(IS_LT(input, input_len)){
				*output='<';
				input+=LT_LEN;
				input_len-=LT_LEN;
			}else if(IS_GT(input, input_len)){
				*output='>';
				input+=GT_LEN;
				input_len-=GT_LEN;
			}else if(IS_APOS(input, input_len)){
				*output='\'';
				input+=APOS_LEN;
				input_len-=APOS_LEN;
			}else if(IS_QUOT(input, input_len)){
				*output='"';
				input+=QUOT_LEN;
				input_len-=QUOT_LEN;
			}else if(IS_AMP(input, input_len)){
				*output='&';
				input+=AMP_LEN;
				input_len-=AMP_LEN;
			}else{
				*output='&';
				input++;
				input_len--;
			}
		}else{
			*output=*input++;
			input_len--;
		}
		output_len++;
		output++;
	}
	return output_len;
}

#define MIN(a,b) ((a)<(b)?(a):(b))
/*
 * input and output must not be the same
 */
int escape_xml_r( const char *input, int input_len, char *output, int output_max_len){
	int output_len=0;
	int l;

	while(input_len){
		if(*input=='<'){
			if(output_max_len>0){
				l=MIN(LT_LEN, output_max_len);
				memcpy(output, LT, l);
				output+=l;
				output_max_len-=l;
			}

			output_len+=LT_LEN;

		}else if(*input=='>'){
			if(output_max_len>0){
				l=MIN(GT_LEN, output_max_len);
				memcpy(output, GT, l);
				output+=l;
				output_max_len-=l;
			}
			output_len+=GT_LEN;

		}else if(*input=='\''){
			if(output_max_len>0){
				l=MIN(APOS_LEN, output_max_len);
				memcpy(output, APOS, l);
				output+=l;
				output_max_len-=l;
			}
			output_len+=APOS_LEN;

		}else if(*input=='"'){
			if(output_max_len>0){
				l=MIN(QUOT_LEN, output_max_len);
				memcpy(output, QUOT, l);
				output+=l;
				output_max_len-=l;
			}
			output_len+=QUOT_LEN;

		}else if(*input=='&'){
			if(output_max_len>0){
				l=MIN(AMP_LEN, output_max_len);
				memcpy(output, AMP, l);
				output+=l;
				output_max_len-=l;
			}
			output_len+=AMP_LEN;

		}else{
			if(output_max_len>=1){
				*output++=*input;
				output_max_len--;
			}
			output_len++;
		}
		input_len--;
		input++;
	}
	return output_len;
}


#define ESCAPE_TEMP_LEN 1024
static int temp_len=0;
static char *temp=NULL;
static char *empty="";
char *unescape_xml( const char *input){
	int len=strlen(input);
	while(len+1>temp_len){
		if((temp=realloc(temp, temp_len+ESCAPE_TEMP_LEN))){
			temp_len+=ESCAPE_TEMP_LEN;
		}else{
			break;
			temp_len=0;
		}
	}
	if(temp){
		len=unescape_xml_r(input, len, temp);
		temp[len>temp_len-1? temp_len-1 : len]=0;
		return temp;
	}else{
		return NULL;
	}
}

char *escape_xml( const char *input){
	int in_len;
	int out_len;

	in_len=strlen(input);

	out_len=escape_xml_r( input, in_len, temp, temp_len);
	if( out_len+1<=temp_len){
		temp[out_len]=0;
		return temp;
	}

	int new_len=(out_len+1)/1024*1024+ESCAPE_TEMP_LEN;
	if((temp=realloc(temp, new_len))){
		temp_len=new_len;
		out_len=escape_xml_r( input, in_len, temp, temp_len);
		temp[out_len]=0;
		return temp;
	}else{
		temp_len=0;
		return empty;
	}
}
