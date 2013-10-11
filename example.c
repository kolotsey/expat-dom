/*
 * Copyright (c) 2011 Sergey Kolotsey.
 * This file if part of expat-dom library.
 * See the file COPYING for copying permission.
 *
 * This is a simple example of usage.
 */

/**
 * @file example.c
 * Expat-dom library usage example
 * @ingroup expat-dom
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "expat-dom.h"

int main( int argc, char *argv[]){
	dom_t *dom, *node, *project, *author;
	char *xml=
"<projects>\
	<project name=\"Expat\">\
		<author>Clark Cooper</author>\
		<website>http://expat.sourceforge.net/</website>\
	</project>\
	<project name=\"Expat-dom\">\
		<author>Sergey Kolotsey</author>\
		<website>http://example.com</website>\
	</project>\
</projects>";


	if( NULL !=(dom=dom_parse_buffer( xml, strlen( xml)))){
		if(( node=dom_find_node( dom, "projects"))){
			project=node->child;
			while( project){
				if( 0==strcasecmp( project->name, "project")){
					char *name=dom_find_attr( project->attr, "name");
					if( name && ( author=dom_find_node( project, "author"))){
						char author_buffer[1024];
						strncpy( author_buffer, author->data, author->data_len<1023? author->data_len : 1023);
						author_buffer[author->data_len<1023? author->data_len : 1023]=0;
						printf("Project %s, author %s\n", name, author_buffer);
					}
				}
				project=project->next;
			}

		}else{
			fprintf( stderr, "Could not find root node\n");
		}
		dom_free( dom);
		return 0;

	}else{
		fprintf( stderr, "Parse XML error: %s\n", strerror( errno));
		return 1;
	}
}

/** @} */
