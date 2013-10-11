/*
 * Copyright (c) 2011 Sergey Kolotsey.
 * This file if part of expat-dom library.
 * See the file COPYING for copying permission.
 *
 * Common expat-dom API functions.
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <expat.h>
#include "expat-dom.h"


//#define DOM_DEBUG(fmt,...) fprintf ( stderr, "%s (%lu): " fmt "\n", __func__, (long unsigned int)pthread_self(), ##__VA_ARGS__)
#ifdef DOM_DEBUG
#include <pthread.h>
#endif


#define DOM_SPACE(a) ((a)==' ' || (a)=='\t' || (a)=='\r' || (a)=='\n')
#define DOM_BUFFER_LEN 2048

/*
 * Please see file expat-dom.h for information about functions
 */

dom_attr_t *dom_attr_free(dom_attr_t *attr){
	dom_attr_t *temp;
	while(attr){
		if(attr->var)
			free(attr->var);
		if(attr->val)
			free(attr->val);
		temp=attr;
		attr=attr->next;
		free(temp);
	}
	return NULL;
}

dom_t *dom_free(dom_t *dom){
	dom_t *temp;
	while(dom){
		if(dom->name)
			free(dom->name);
		if(dom->data)
			free(dom->data);
		dom_attr_free(dom->attr);
		dom_free(dom->child);
		temp=dom;
		dom=dom->next;
		free(temp);
	}
	return NULL;
}



static dom_attr_t *dom_attributes(const char **atts){
	dom_attr_t *attr=NULL;
	dom_attr_t *temp;

	while(atts[0]){
		if((temp=calloc(1, sizeof(dom_attr_t)))) {
			temp->var=strdup(atts[0]);
			if(atts[1]){
				temp->val=strdup(atts[1]);
			}else{
				temp->val=strdup("");
			}
			temp->next=attr;
			attr=temp;
			atts+=2;
		}else{
			break;
		}
	}
	return attr;
}

static void XMLCALL start_element(void *user_data, const char *name, const char **atts){
	dom_t *dom;
	dom_t *temp;

	dom=*((dom_t **)user_data);
	if(dom){
		if((temp=calloc(1, sizeof(dom_t)))) {
			temp->closed=0;
			temp->user_data=NULL;
			temp->user_data_len=0;
			temp->name=strdup(name);
			temp->data=NULL;
			temp->data_len=0;
			temp->attr=dom_attributes(atts);
			*((dom_t **)user_data)=temp;
			temp->next=NULL;
			temp->child=NULL;
			if(dom->closed){
				dom->next=temp;
				temp->parent=dom->parent;
			}else{
				temp->parent=dom;
				dom->child=temp;
			}
		}
	}else{
		if((dom=calloc(1, sizeof(dom_t)))) {
			*((dom_t **)user_data)=dom;
			dom->name=strdup(name);
			dom->data=NULL;
			dom->user_data=NULL;
			dom->user_data_len=0;
			dom->data_len=0;
			dom->attr=dom_attributes(atts);
			dom->closed=0;
			dom->next=NULL;
			dom->child=NULL;
			dom->parent=NULL;
		}
	}
}

static void XMLCALL end_element(void *user_data, const char *name){
	dom_t *dom;

	dom=*((dom_t **)user_data);
	if(dom){
		if(dom->closed){
			//close parent tag
			if(dom->parent){
				dom=dom->parent;
				*((dom_t **)user_data)=dom;
				end_element(user_data, name);
			}
#ifdef DOM_DEBUG
			else{
				DOM_DEBUG("close closed root tag %s?", name);
			}
#endif
		}else{
#ifdef DOM_DEBUG
			if( !dom->name){
				DOM_DEBUG("close tag with no name, need to close %s", name);
			}else if(0 !=strcmp(name, dom->name)){
				DOM_DEBUG("close tag %s, need to close %s", dom->name, name);
			}
#endif
			dom->closed=1;
			if(dom->data && dom->data_len){
				if( !dom->user_data){
					dom->user_data=dom->data;
					dom->user_data_len=dom->data_len;
					while(dom->user_data_len && *dom->user_data && DOM_SPACE(dom->user_data[0])){
						dom->user_data++;
						dom->user_data_len--;
					}
					while(dom->user_data_len && DOM_SPACE(dom->user_data[dom->user_data_len-1])){
						dom->user_data_len--;
					}
				}
			}
		}
	}
#ifdef DOM_DEBUG
	else{
		DOM_DEBUG("close unopened tag %s", name);
	}
#endif
}

static void XMLCALL start_cdata(void *user_data){
	dom_t *dom;
	dom=*((dom_t **)user_data);

	if(dom){
		if(dom->closed){
			if(dom->parent && ( !dom->parent->closed)){
				start_cdata(&dom->parent);
			}
#ifdef DOM_DEBUG
			else{
				DOM_DEBUG("user_data outside of root tag?");
			}
#endif
		}else if(dom->data){
			dom->user_data=dom->data+dom->data_len;
			dom->user_data_len=0;
		}
#ifdef DOM_DEBUG
		else{
			DOM_DEBUG("user_data without data?");
		}
#endif
	}
#ifdef DOM_DEBUG
	else{
		DOM_DEBUG("user_data in empty tag");
	}
#endif
}

static void XMLCALL end_cdata(void *user_data){
	dom_t *dom;
	dom=*((dom_t **)user_data);

	if(dom){
		if(dom->closed){
#ifdef DOM_DEBUG
			DOM_DEBUG("user_data close outside tag");
#endif
		}else if(dom->user_data){
			dom->user_data_len=dom->data+dom->data_len-dom->user_data;
		}
#ifdef DOM_DEBUG
		else{
			DOM_DEBUG("user_data close without open");
		}
#endif
	}
#ifdef DOM_DEBUG
	else{
		DOM_DEBUG("close user_data in empty tag");
	}
#endif
}

static void XMLCALL element_data(void *user_data, const char *buffer, int buffer_len){
	dom_t *dom;

	dom=*((dom_t **)user_data);
	if(dom){
		if((dom->data=realloc(dom->data, buffer_len+dom->data_len))) {
			memcpy(dom->data+dom->data_len, buffer, buffer_len);
			dom->data_len+=buffer_len;
		}
	}
#ifdef DOM_DEBUG
	else{
		DOM_DEBUG("user data in empty tag");
	}
#endif
}


//функция для поиска нужного параметра в массиве атрибутов
char *dom_find_attr(dom_attr_t *attr, char *var){
	while(attr){
		if(0==strcasecmp(var, attr->var)){
			return attr->val;
		}
		attr=attr->next;
	}
	return NULL;
}

//функция для поиска первого нужного узла в массиве узлов
dom_t *dom_find_node(dom_t *root, char *node){
	dom_t *ret;
	while(root){
		if(0==strcasecmp(root->name, node)){
			return root;
		}else if(root->child){
			if((ret=dom_find_node(root->child, node))){
				return ret;
			}
		}
		root=root->next;
	}
	return NULL;
}

static void dom_print_attr(FILE *output, dom_attr_t *attr){
	while(attr){
		if(attr->var && attr->val){
			fprintf(output, " %s=\"%s\"", attr->var, escape_xml(attr->val));
		}
		attr=attr->next;
	}
}

void dom_print(FILE *output, dom_t *dom, int use_new_line){
	while(dom){
		fprintf(output, "<%s", dom->name);
		dom_print_attr(output, dom->attr);
		if(dom->child || (dom->user_data && dom->user_data_len)){
			fprintf( output, ">");
			if(use_new_line) fprintf( output, "\n");
			if(dom->user_data && dom->user_data_len){
				char *buf;
				int len=escaped_length( dom->user_data, dom->user_data_len);
				if(( buf=malloc( len+1))){
					escape_xml_r( dom->user_data, dom->user_data_len, buf, len);
					fwrite( buf, len, 1, output);
					free( buf);
				}
				if(use_new_line) fprintf( output, "\n");
			}
			if(dom->child){
				dom_print(output, dom->child, use_new_line);
			}
			fprintf(output, "</%s>", dom->name);
			if(use_new_line) fprintf( output, "\n");
		}else{
			fprintf(output, "/>");
			if(use_new_line) fprintf( output, "\n");
		}
		dom=dom->next;
	}
}


dom_t *dom_parse_file(int fd){
	dom_t *dom=NULL;
	int size_read;
	char buffer[DOM_BUFFER_LEN];
	XML_Parser parser;
	int done;

	if(NULL==(parser=XML_ParserCreate(NULL))){
#ifdef DOM_DEBUG
		DOM_DEBUG("xml parser create error");
#endif
		errno=ENOMEM;
		return NULL;
	}
	done=0;
	XML_SetUserData(parser, &dom);
	XML_SetElementHandler(parser, start_element, end_element);
	XML_SetCdataSectionHandler(parser, start_cdata, end_cdata);
	XML_SetCharacterDataHandler(parser, element_data);


	while( !done){
#ifdef DOM_DEBUG
		DOM_DEBUG("Reading");
#endif
		if(-1==(size_read=read(fd, buffer, DOM_BUFFER_LEN))){
#ifdef DOM_DEBUG
			DOM_DEBUG("file read error: %s", strerror(errno));
#endif
			if( dom) while( dom->parent) dom=dom->parent;
			dom=dom_free(dom);
			//errno is set in "read"
			break;
		}
		done=(size_read==0);
#ifdef DOM_DEBUG
			DOM_DEBUG("file read: %d bytes", size_read);
#endif
		if (XML_Parse(parser, buffer, size_read, done) == XML_STATUS_ERROR) {
#ifdef DOM_DEBUG
			DOM_DEBUG("parse error: %s", XML_ErrorString(XML_GetErrorCode(parser)));
#endif
			if( dom) while( dom->parent) dom=dom->parent;
			dom=dom_free(dom);
			errno=EINVAL;
			break;
		}
	}
	XML_ParserFree(parser);
	return dom;
}

dom_t *dom_parse_file_name(char *name){
	int fd;
	dom_t *dom=NULL;


	if(-1 !=(fd=open(name, O_RDONLY))){
		dom=dom_parse_file(fd);
		close(fd);
		//dom_print(stdout, dom);
		//dom=dom_free(dom);

	}
#ifdef DOM_DEBUG
	else{
		DOM_DEBUG("file %s open error: %s", name, strerror(errno));
		//errno is set in "open"
	}
#endif
	return dom;
}

dom_t *dom_parse_buffer(char *buffer, int buffer_len){
	dom_t *dom=NULL;
	XML_Parser parser;

	if(NULL==(parser=XML_ParserCreate(NULL))){
#ifdef DOM_DEBUG
		DOM_DEBUG("xml parser create error");
#endif
		errno=ENOMEM;
		return NULL;
	}
	XML_SetUserData(parser, &dom);
	XML_SetElementHandler(parser, start_element, end_element);
	XML_SetCdataSectionHandler(parser, start_cdata, end_cdata);
	XML_SetCharacterDataHandler(parser, element_data);



	if (XML_Parse(parser, buffer, buffer_len, 1) == XML_STATUS_ERROR) {
#ifdef DOM_DEBUG
		DOM_DEBUG("parse error: %s", XML_ErrorString(XML_GetErrorCode(parser)));
#endif
		if( dom) while( dom->parent) dom=dom->parent;
		dom=dom_free(dom);
		XML_ParserFree(parser);
		errno=EINVAL;
		return NULL;
	}
	XML_ParserFree(parser);
	return dom;
}


int dom_parse_chunked_data( void **parser, dom_t **dom, char *buffer, int buffer_len, int isFinal){
	XML_Parser p=*parser;

	if( NULL==p){
		if(NULL==(p=XML_ParserCreate(NULL))){
#ifdef DOM_DEBUG
			DOM_DEBUG("xml parser create error");
#endif
			return ENOMEM;
		}
		XML_SetUserData(p, dom);
		XML_SetElementHandler(p, start_element, end_element);
		XML_SetCdataSectionHandler(p, start_cdata, end_cdata);
		XML_SetCharacterDataHandler(p, element_data);
		*parser=p;
	}

	if (XML_Parse(p, buffer, buffer_len, isFinal) == XML_STATUS_ERROR) {
#ifdef DOM_DEBUG
		DOM_DEBUG("parse error: %s", XML_ErrorString(XML_GetErrorCode(p)));
#endif
		if( *dom) while( (*dom)->parent) *dom=(*dom)->parent;
		dom_free(*dom);
		*dom=NULL;
		XML_ParserFree(p);
		*parser=NULL;
		return EINVAL;
	}

	if(isFinal){
		XML_ParserFree(p);
		*parser=NULL;
	}
	return 0;
}
