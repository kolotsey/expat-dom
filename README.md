
EXPAT-DOM, THE EXPAT WRAPPER
============================

Overview
--------

  This library is a wrapper for Expat C library (http://expat.sourceforge.net).
  This library makes XML easy to use in C by providing a very simple DOM
  interface. You do not need to register handlers. The library parses the whole
  file or buffer and returns a pointer to the DOM structure that you can 
  navigate by calling the special functions, such as dom_find_node(). You also
  can navigate the structure, using its internal fields, e.g. child, parent and
  next.

  All function in the library are thread-safe. Please, refer to the example on
  how to use this library below.

Requirements
------------

  This library requires Expat library in order to compile successfully. If you
  use a Linux distribution with packging system, try installing libexpat-dev
  package.

Installation Instructions
-------------------------

  The simplest way to compile this package is:

  1. 'cd' to the directory containing the package's source code and type
     `./configure` to configure the package for your system. While running, 
     'configure' prints some messages telling which features it is 
     checking for. There are many options which you may provide to configure 
     (which you can discover by running configure with the --help option).

  2. Type `make` to compile the package.

  3. Type `make install` to install the programs, libraries and any data files
     and documentation.

Documentation
-------------

  The library is distributed with detailed documentation. The documentation
  in HTML format is stored in sub-folder named 'doc'.

Example
-------

  This is a simple example of library usage

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
		                    strncpy( author_buffer, author->data, 
		                    	author->data_len<1023? author->data_len : 1023);
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


  See file 'example.c'. To build this file, run
    `gcc -lexpat -lexpat-dom -o example example.c`
  in console.

Copyright notice
----------------

  Expat-dom is free software.  You may copy, distribute, and modify it under
  the terms of the License contained in the file COPYING distributed with this
  package. This license is the same as the MIT/X Consortium license.
