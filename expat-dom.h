/*
 * Copyright (c) 2011 Sergey Kolotsey.
 * This file if part of expat-dom library.
 * See the file COPYING for copying permission.
 */

#ifndef __EXPAT_DOM_INCLUDED
#define __EXPAT_DOM_INCLUDED

#include <stdio.h>


/**
 * @file expat-dom.h
 * Expat-dom library for creating DOM struct
 * @ingroup expat-dom
 * @defgroup expat-dom-library Expat-dom library
 * @{
 */

/**
 * @mainpage Expat-dom library
 *
 * @section Overview
 *
 * This library is a wrapper for <a href='http://expat.sourceforge.net'>Expat
 * C library</a>.
 *
 * This library makes XML easy to use in C by providing a very simple DOM
 * interface. You do not need to register handlers. The library parses the whole
 * file or buffer and returns a pointer to the DOM tree (structure in memory) that you can
 * navigate by calling the special functions, such as dom_find_node(). You also
 * can navigate the structure, using its internal fields, e.g. @c child, @c parent and
 * @c next. All functions in the library are thread-safe, except escape_xml() and
 * unescape_xml(). Please, refer to the example on how to use this library below.
 *
 * @section Requirements
 * This library requires <a href='http://expat.sourceforge.net'>Expat
 * library</a> in order to compile successfully. If you use a Linux distribution
 * with packging system, try installing @c libexpat-dev package.
 *
 * @section Example
 *
 * @code
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "expat-dom.h"

int main( int argc, char *argv[]){
	dom_t *dom;
	char *xml="<hello><world/></hello>";

	if( NULL !=(dom=dom_parse_buffer( xml, strlen( xml)))){
		fprintf( stdout, "Root node name: %s\n", dom->name);
		fprintf( stdout, "Root node has children: %s\n", dom->child==NULL? "False" : "True");
		dom_free( dom);
		return 0;
	}else{
		fprintf( stderr, "Parse XML error: %s\n", strerror( errno));
		return 1;
	}
}
 * @endcode
 *
 * To build this example, copy it into a file and rename the file to example.c.
 * After renaming the file, run in console the following command:
 * @verbatim gcc -lexpat -lexpat-dom -o example example.c @endverbatim
 *
 * See also example.c.
 *
 * @section Copiright Copyright notice
 *
 * Expat-dom is free software.  You may copy, distribute, and modify it under the
 * terms of the License contained in the file COPYING distributed with this
 * package. This license is the same as the MIT/X Consortium license.
 *
 */



/**
 * @brief This structure contains information about attributes of a node.
 *
 * The structure contains information about attributes of a node including its
 * name and value. All attributes of the node are represented as a linked list
 * of @c dom_attr_t structures.
 * Every instance of @c dom_t structure has a field attr that points to the
 * @c dom_attr_t structure containing list of attributes.
 *
 * @see dom_find_attr().
 */
typedef struct dom_attr_s dom_attr_t;
struct dom_attr_s{
	/**
	 * @brief Attribute name: pointer to a NULL-terminated string.
	 */
	char *var;
	/**
	 * @brief Attribute value: pointer to a NULL-terminated string.
	 */
	char *val;
	/**
	 * @brief Pointer to the next attribute.
	 */
	dom_attr_t *next;
};

/**
 * @brief This structure contains information about an XML node.
 *
 * The expat-dom parcer creates a DOM tree in memory. Each node of the DOM
 * tree is represented as dom_t structure. Root node of this tree contains
 * reference to its first child branch (node). This node in its turn contains
 * pointer to next node on the same level (sibling) and so on.
 *
 * Every node of the tree has a linked list of attributes. The attributes names
 * and values are stored in @c dom_attr_t structures.
 *
 * @see dom_find_node().
 */
typedef struct dom_s dom_t;
struct dom_s{
	/**
	 * @brief Node name: pointer to a NULL-terminated string.
	 */
	char *name;
	/**
	 * @brief Pointer to a linked list that contains node attributes.
	 */
	dom_attr_t *attr;
	/**
	 * @brief Pointer to binary data of the node.
	 */
	char *data;
	/**
	 * @brief Length of the data in field @c data.
	 */
	int data_len;
	/**
	 * @brief Pointer to user data that is wrapped in CDATA tag.
	 */
	char *user_data;
	/**
	 * @brief Length of the data in field @c user_data.
	 */
	int user_data_len;

	/**
	 * @brief Always 1 for well-formed XML files.
	 */
	int closed;
	/**
	 * @brief Pointer to parent node.
	 */
	dom_t *parent;
	/**
	 * @brief Pointer to the first child node.
	 */
	dom_t *child;
	/**
	 * @brief Pointer to the next sibling.
	 */
	dom_t *next;
};


/**
 * @brief Prints DOM tree specified by @c *dom pointer.
 *
 * The function prints DOM tree structure to the specified output as string.
 * The DOM tree is passed to the function as pointer to a previously created
 * @c dom_t structure.
 *
 * @par Example:
 * @code
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "expat-dom.h"

int main( int argc, char *argv[]){
	dom_t *dom;
	char *xml="<hello><world/></hello>";

	if( NULL !=(dom=dom_parse_buffer( xml, strlen( xml)))){
		dom_print( stdout, dom, 1);
		dom_free( dom);
		return 0;
	}else{
		fprintf( stderr, "Parse XML error: %s\n", strerror( errno));
		return 1;
	}
}
 * @endcode
 *
 * @param output Output stream where the DOM tree should be printed.
 * @param dom Pointer to DOM tree to print.
 * @param use_new_line If set to 1, then new line is printed after each node.
 */
void dom_print(FILE *output, dom_t *dom, int use_new_line);

/**
 * @brief Frees previously created linked list of attributes.
 *
 * @param attr Pointer to dom_attr_t structure to free.
 * @return Returns NULL always.
 */
dom_attr_t *dom_attr_free(dom_attr_t *attr);

/**
 * @brief Frees previously created DOM structure.
 *
 * @param dom Pointer to @c dom_t structure to free.
 * @return Returns NULL always.
 *
 * @par Example:
 * See dom_print() for examples.
 *
 */
dom_t *dom_free(dom_t *dom);

/**
 * @brief Read DOM tree from previously opened XML file.
 *
 * The function reads and parses opened XML file.
 * The file must be opened with @c O_RDONLY or @c O_RDWR permission.
 * The function reads all the data from the file determined by file descriptor
 * and tries to parse it as XML data. If this operation succeeds, the function
 * returns a pointer to the newly created @c dom_t stricture. Otherwise NULL is
 * returned and errno is set.
 *
 * @par Example:
 * See dom_parse_file_name() for examples.
 *
 * @param fd Previously opened file descriptor of the file to be read
 * @return Pointer to newly allocated dom_t structure is returned if no
 * 	error occured. The user must free the allocated data when it is not used by
 * 	means of function dom_free(). If error occurs, the function returns NULL
 * 	and sets errno to the following values:
 * 		@li @c ENOMEM Not enough memory.
 * 		@li @c EINVAL Parse error. The opened file does not contain valid XML file or
 * 		        the XML file is not well-formed.
 */
dom_t *dom_parse_file(int fd);

/**
 * @brief Read DOM tree from a file specified by name.
 *
 * The function opens and parses XML file. If this operation succeeds,
 * the function returns a pointer to the newly created @c dom_t stricture.
 * Otherwise NULL is returned and errno is set.
 *
 * @par Example:
 * @code
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "expat-dom.h"

int main( int argc, char *argv[]){
	dom_t *dom, *node;
	dom_attr_t *attr;

	if( NULL ==(dom=dom_parse_file_name( "movies.xml"))){
		fprintf( stderr, "Parse XML error: %s\n", strerror( errno));
		return 1;

	}else{
		if(( node=dom_find_node( dom, "movie"))){
			attr=node->attr;

			printf("Node attributes:\n");
			while( attr){
				printf(" %s='%s'%c", attr->var, attr->val, attr->next? ',' : '\n');
				attr=attr->next;
			}

			printf("Node children:\n");
			node=node->child;
			while( node){
				printf(" %s%c", node->name, node->next? ',' : '\n');
				node=node->next;
			}

		}else{
			fprintf( stderr, "Could not find root node\n");
		}
		dom_free( dom);
		return 0;
	}
}
 * @endcode
 *
 * @par
 * In order to successfully build and run this code you will have to
 * copy the following code into a file @e movies.xml.
 *
 * @par
 * @verbatim
<?xml version='1.0'?>
<movies>
	<movie title="The Shawshank Redemption" year="1994" length="142" rating="9.2">
		<characters>
			<character>
				<name>Andy Dufresne</name>
				<actor>Tim Robbins</actor>
			</character>
			<character>
				<name>Ellis Boyd Redding</name>
				<actor>Morgan Freeman</actor>
			</character>
		</characters>
		<plot>
			Two imprisoned men bond over a number of years, finding solace and
			eventual redemption through acts of common decency.
		</plot>
		<quotes>
			<quote>
				District Attorney: Mr. Dufresne, describe the confrontation you
				had with your wife the night that she was murdered.
			</quote>
		</quotes>
	</movie>
</movies>
@endverbatim
 *
 *
 * @param name Full or relative path to the XML file.
 * @return Pointer to newly allocated @c dom_t structure is returned if no
 * 	error occured. The user must free the allocated data when it is not used
 * 	with dom_free(). If error occurs, the function returns NULL and sets
 * 	errno to the following values:
 * 		@li @c ENOMEM Not enough memory.
 * 		@li @c EINVAL Parse error. The opened file does not contain valid XML
 * 		        file or the XML file is not well-formed.
 * 		@li Other codes may be returned.
 */
dom_t *dom_parse_file_name(char *name);

/**
 * @brief Read DOM tree from a buffer.
 *
 * The function parses passed buffer as XML. If this operation succeeds, the
 * function returns a pointer to the newly created @c dom_t stricture. Otherwise
 * NULL is returned and errno is set.
 *
 * @par Example:
 * See example.c.
 *
 * @param buffer Pointer to a buffer containing well-formed XML data.
 * @param buffer_len Length of the data stored in buffer.
 * @return Pointer to newly allocated @c dom_t structure is returned if no
 * 	error occured. User must free the allocated data with dom_free() when it is
 * 	not used. If error occurs, the function returns NULL and sets
 * 	errno to the following values:
 * 		@li @c ENOMEM Not enough memory.
 * 		@li @c EINVAL Parse error. The buffer does not contain valid XML data or
 * 		        the XML data is not well-formed.
 */
dom_t *dom_parse_buffer(char *buffer, int buffer_len);

/**
 * @brief Parse XML data in chunks.
 *
 * The function parses part of the XML data.
 * The function should be used in cases when not all XML data is available
 * imediately. The function allows user to parse data partially. If this
 * operation succeeds, the function returns 0 and sets parser and dom variables.
 * Otherwise error is returned.
 *
 * This function is used when the length of the buffer with XML data is not
 * known when the first part of the buffer is available. This function may be
 * used in loops when data is received from network or other source in chunks.
 * See example below.
 *
 * @par Example:
 * @code
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "expat-dom.h"

int main( int argc, char *argv[]){
	int count;
	char buffer[28];
	dom_t *dom;
	void *parser;
	int ret;

	parser=NULL;
	dom=NULL;
	ret=0;

	do{
		count=fread( buffer, 1, sizeof( buffer), stdin);
		ret=dom_parse_chunked_data( &parser, &dom, buffer, count, 0);
		if( 0!=ret){
			break;
		}
	}while( !feof( stdin));

	if(0==ret){
		ret=dom_parse_chunked_data( &parser, &dom, NULL, 0, 1);
	}
	if( 0==ret){
		dom_print( stdout, dom, 1);
	}else{
		fprintf( stderr, "Parse error: %s\n", strerror( ret));
	}
	if( dom){
		dom_free( dom);
	}
	return 0;
}
 * @endcode
 * In this example XML data is read from standard input into buffer in parts.
 * When a part of XML data is read, it is passed to the function where it is
 * parsed.
 *
 * @param parser Pointer to internal structure.
 * @param dom Pointer to DOM structure that is created by the function.
 *    The data pointed to by this pointer should be freed by user with
 *    function dom_free().
 * @param buffer Pointer to a buffer containing next part (chunk) of XML data.
 * @param buffer_len Length of the data stored in @c buffer.
 * @param isFinal This variable must be zero for every chunk except the last
 * 	  one, and it must be 1 for the last chunk passed to the function.
 * @return The function returns 0 when succeeds. If an error occurs, the function
 * 	returns error code:
 * 		@li @c ENOMEM Not enough memory.
 * 		@li @c EINVAL Parse error. The buffer does not contain valid XML data or
 * 		        the data is not well-formed XML data.
 */
int dom_parse_chunked_data( void **parser, dom_t **dom, char *buffer, int buffer_len, int isFinal);

/**
 * @brief Find attribute in a linked list by its name.
 *
 * The function allows to find an attribute in an attribute list.
 * The function seeks the linked list pointed by @c attr for the
 * specified attribute. If the attribute is found, its value is returned as
 * pointer to a buffer with NULL-terminated string. The value in the returned
 * buffer should not be modified. Duplicate the buffer to modify the value.
 *
 * @par Example:
 * See file example.c.
 *
 * @param attr Pointer to a linked list with attributes.
 * @param var Pointer to a buffer containing NULL-terminated string with
 *    the name of the attribute to find.
 * @return Pointer to a buffer containing a NULL-terminated string with
 *    attribute value. If no attribute is found with the specified name,
 * 	  then NUll is returned.
 */
char *dom_find_attr(dom_attr_t *attr, char *var);

/**
 * @brief Find a node in DOM tree by its name.
 *
 * The function scans the passed DOM tree and finds the first found node that
 * has a name specified by @c node. The function seeks the DOM tree for the
 * specified node @c root. If a node is found, the function returns pointer to
 * the @c dom_t structure that has the requested name. The function seeks for the
 * specified name recursively. That is, all the nodes in the passed @c dom_t
 * structure @c root are searched, including child nodes. If the function
 * can not find the specified node, NULL is returned.
 *
 * @par Example:
 * See file example.c.
 *
 * @param root Pointer to @c dom_t structure where the required node should
 *    be searched for
 * @param node Pointer to a buffer containing NULL-terminated string with
 *    the name of the node to find.
 * @return Pointer to the first node with the specified name, if found.
 *    Otherwise NULL is returned.
 */
dom_t *dom_find_node(dom_t *root, char *node);

/**
 * @brief Convert specal XML characters into XML entities.
 *
 * The function retuns a string with special XML characters converted into XML
 * entites. The following characters are converted:
 *    @li @c & (ampersand) becomes @c &amp;amp;
 *    @li @c " (double quote) becomes @c &amp;quot;
 *    @li @c ' (single quote) becomes @c &amp;apos;
 *    @li @c < (less than) becomes @c &amp;lt;
 *    @li @c > (greater than) becomes @c &amp;gt;
 *
 * @par Important:
 * <i>The function is not thread-safe</i>. It retuns a pointer to a static
 * buffer that is reused every time the function is called.
 * If you need a thread-safe version of this function, use escape_xml_r().
 *
 * @param input Pointer to a NULL-terminated buffer that contains string to
 *    convert.
 * @return Pointer to a NULL-terminated buffer containing converted string. The
 *    buffer returned is a static buffer. In a multithreaded application use
 *    escape_xml_r().
 *
 * @see escape_xml_r().
 *
 */
char *escape_xml(char *input);

/**
 * @brief Convert specal XML characters into XML entities.
 *
 * The function retuns a string with special XML characters converted into XML
 * entites. The following characters are converted:
 *    @li @c & (ampersand) becomes @c &amp;amp;
 *    @li @c " (double quote) becomes @c &amp;quot;
 *    @li @c ' (single quote) becomes @c &amp;apos;
 *    @li @c < (less than) becomes @c &amp;lt;
 *    @li @c > (greater than) becomes @c &amp;gt;
 *
 * The difference between escape_xml() and escape_xml_r() is that the function
 * escape_xml_r() is a thread-safe function.
 *
 * Use function escaped_length() to determine required buffer length for result.
 *
 * @param input Pointer to a buffer that contains string to convert.
 * @param input_len Length of the string in @c buffer.
 * @param output Pointer to a buffer where function can store result. The
 *    result string stored in the @c output buffer is not NULL-terminated.
 *    @c output @e MUST not point to the same buffer as @c input.
 *    Use function escaped_length() to determine minimum required length of
 *    this buffer.
 * @param output_max_len Length of the buffer @c output.
 * @return Length of the resulting string wrtten to @c output buffer.
 *
 * @see escape_xml(), escaped_length().
 *
 */
int escape_xml_r(char *input, int input_len, char *output, int output_max_len);

/**
 * @brief Determine length of a string with converted specal XML chars.
 *
 * The function allows to determine the length of the string with converted
 * special XML characters.
 *
 * @param input Pointer to a buffer containing a string wth special XML chars.
 * @param input_length Length of the string in buffer @c input.
 * @return Length of the bufer to store converted string.
 *
 * @see escape_xml_r().
 */
int escaped_length( char *input, int input_length);

/**
 * @brief Convert XML entities into specal XML characters.
 *
 * The function retuns a string with XML entites converted to special XML
 * characters. The following entities are converted:
 *    @li @c &amp;amp; becomes @c &
 *    @li @c &amp;quot; becomes @c "
 *    @li @c &amp;apos; becomes @c '
 *    @li @c &amp;lt; becomes @c <
 *    @li @c &amp;gt; becomes @c >
 *
 * @par Important:
 * <i>The function is not thread-safe</i>. It retuns a pointer to a static
 * buffer that is reused every time the function is called.
 * If you need a thread-safe version of this function, use unescape_xml_r().
 *
 * @param input Pointer to a NULL-terminated buffer that contains string to
 *    convert.
 * @return Pointer to a NULL-terminated buffer containing converted string. The
 *    buffer returned is a static buffer. In a multithreaded application use
 *    unescape_xml_r().
 *
 * @see unescape_xml_r().
 *
 */
char *unescape_xml(char *input);

/**
 * @brief Convert XML entities into specal XML characters.
 *
 * The function retuns a string with XML entites converted to special XML
 * characters. The following entities are converted:
 *    @li @c &amp;amp; becomes @c &
 *    @li @c &amp;quot; becomes @c "
 *    @li @c &amp;apos; becomes @c '
 *    @li @c &amp;lt; becomes @c <
 *    @li @c &amp;gt; becomes @c >
 *
 * This is a thread-safe version of function unescape_xml().
 *
 * @param input Pointer to a buffer that contains string to
 *    convert.
 * @param input_len Length of the string in buffer @c input.
 * @param output Pointer to a buffer where function can store its result. This
 *    buffer may be the same as @c input. The minimum length of this buffer
 *    should be the same as @c input_len.
 * @return Length of the string put into the buffer @c output.
 *
 * @see unescape_xml().
 *
 */
int unescape_xml_r(char *input, int input_len, char *output);


/** @} */

#endif //__EXPAT_DOM_INCLUDED

