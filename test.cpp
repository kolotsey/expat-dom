/*
 * test.c
 *
 *  Created on: 17/11/2013
 *      Author: Sergey Kolotsey
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <linux/types.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <CppUTest/CommandLineTestRunner.h>
extern "C" {
#include "expat-dom.h"
}

#ifndef MAX
	#define MAX(a,b) ((a)>(b)? (a):(b))
#endif
#ifndef MIN
	#define MIN(a,b) ((a)<(b)? (a):(b))
#endif


TEST_GROUP(g_is_escaped)
{
};
TEST(g_is_escaped, t_is_escaped){
	CHECK_TRUE( is_escaped('<'));
	CHECK_TRUE( is_escaped('>'));
	CHECK_TRUE( is_escaped('\''));
	CHECK_TRUE( is_escaped('"'));
	CHECK_TRUE( is_escaped('&'));
	CHECK_FALSE( is_escaped(' '));
	CHECK_FALSE( is_escaped('a'));
	CHECK_FALSE( is_escaped('1'));
	CHECK_FALSE( is_escaped('-'));
	CHECK_FALSE( is_escaped('\n'));
	CHECK_FALSE( is_escaped( 0));
	CHECK_FALSE( is_escaped( -1));
	CHECK_FALSE( is_escaped( 1024));
}


TEST_GROUP(g_escape_xml)
{
};
TEST( g_escape_xml, t_escape_xml){
	STRCMP_EQUAL( "", escape_xml(""));
	STRCMP_EQUAL( "Hello\n\r\t World!", escape_xml("Hello\n\r\t World!"));
	STRCMP_EQUAL( "&lt;test/&gt;", escape_xml("<test/>"));
	STRCMP_EQUAL( "test&amp;this", escape_xml("test&this"));
	STRCMP_EQUAL( "test&amp;amp;this", escape_xml("test&amp;this"));
	STRCMP_EQUAL( "&lt;&gt;&apos;&quot;&amp;", escape_xml("<>'\"&"));
}


TEST_GROUP(g_escape_xml_r)
{
};
TEST( g_escape_xml_r, t_escape_xml_r){
	LONGS_EQUAL(0, escape_xml_r("", 0, NULL, 0));
	LONGS_EQUAL(1, escape_xml_r(" ", 1, NULL, 0));
	LONGS_EQUAL(4, escape_xml_r("<", 1, NULL, 0));
	LONGS_EQUAL(strlen("&lt;&gt;&apos;&quot;&amp;"), escape_xml_r("<>'\"&", strlen("<>'\"&"), NULL, 0));

	LONGS_EQUAL(0, escaped_length("", 0));
	LONGS_EQUAL(1, escaped_length(" ", 1));
	LONGS_EQUAL(4, escaped_length("<", 1));
	LONGS_EQUAL(strlen("&lt;&gt;&apos;&quot;&amp;"), escaped_length("<>'\"&", strlen("<>'\"&")));

	char out[10];

	LONGS_EQUAL(strlen("&lt;&gt;&apos;&quot;&amp;"), escape_xml_r("<>'\"&", strlen("<>'\"&"), out, sizeof(out)));
	out[9]=0;
	STRCMP_EQUAL("&lt;&gt;&", out);

	LONGS_EQUAL(strlen("Hello World!"), escape_xml_r("Hello World!", strlen("Hello World!"), out, sizeof(out)));
	out[9]=0;
	STRCMP_EQUAL("Hello Wor", out);
}


TEST_GROUP(g_unescape_xml)
{
};
TEST( g_unescape_xml, t_unescape_xml){
	STRCMP_EQUAL( "", unescape_xml(""));
	STRCMP_EQUAL( "Hello\n\r\t World!", unescape_xml("Hello\n\r\t World!"));
	STRCMP_EQUAL( "<test/>", unescape_xml("&lt;test/&gt;"));
	STRCMP_EQUAL( "test&this", unescape_xml("test&amp;this"));
	STRCMP_EQUAL( "test&amp;this", unescape_xml("test&amp;amp;this"));
	STRCMP_EQUAL( "<>'\"&", unescape_xml("&lt;&gt;&apos;&quot;&amp;"));
}


TEST_GROUP(g_unescape_xml_r)
{
};
TEST( g_unescape_xml_r, t_unescape_xml_r){
	char out[1024];

	LONGS_EQUAL(0, unescape_xml_r("", 0, NULL));
	LONGS_EQUAL(1, unescape_xml_r(" ", 1, out));
	out[1]=0;
	STRCMP_EQUAL(" ", out);
	LONGS_EQUAL(1, unescape_xml_r("&lt;", 4, out));
	out[1]=0;
	STRCMP_EQUAL("<", out);

	LONGS_EQUAL(strlen("<>'\"&"), unescape_xml_r("&lt;&gt;&apos;&quot;&amp;", strlen("&lt;&gt;&apos;&quot;&amp;"), out));
	out[strlen("<>'\"&")]=0;
	STRCMP_EQUAL("<>'\"&", out);

	LONGS_EQUAL(strlen("<>&ap"), unescape_xml_r("&lt;&gt;&apos;", 11, out));
	out[5]=0;
	STRCMP_EQUAL("<>&ap", out);

	LONGS_EQUAL(strlen("Hello World!"), unescape_xml_r("Hello World!", strlen("Hello World!"), out));
	out[strlen("Hello World!")]=0;
	STRCMP_EQUAL("Hello World!", out);
}


TEST_GROUP(g_dom)
{
#define XML "\
<movies>\
<movie title=\"The Shawshank Redemption\" year=\"1994\" length=\"142\" rating=\"9.2\">\
<characters>\
<character><name>Andy Dufresne</name><actor>Tim Robbins</actor></character>\
<character><name>Ellis Boyd Redding</name><actor>Morgan Freeman</actor></character>\
</characters>\
<plot>Two imprisoned men bond over a number of years, finding solace and eventual redemption through acts of common decency.</plot>\
<quotes><quote>District Attorney: &quot;Mr. Dufresne, describe the confrontation you had with your wife the night that she was murdered&quot;.</quote></quotes>\
</movie>\
</movies>"

	typedef struct {
		int fd[2];
		char  buffer[2048];
		int buffer_ready;
		pthread_mutex_t mutex;
	}shared_s;

	shared_s shared;
	pthread_t tid;

	static void *read_thread( void *arg){
		shared_s *shared=(shared_s *) arg;
		FILE *f=fdopen( shared->fd[0], "r");
		int buffer_len=0;
		int r;

		do{
			r=fread( shared->buffer+buffer_len, 1, sizeof( shared->buffer)-buffer_len, f);
			buffer_len+=r;
		}while( !feof(f));

		fclose(f);
		close(shared->fd[0]);
		shared->buffer[ MIN(buffer_len, sizeof( shared->buffer)-1)]=0;

		pthread_mutex_lock( &shared->mutex);
		shared->buffer_ready=1;
		pthread_mutex_unlock( &shared->mutex);

		pthread_exit( (void *)0);
		return NULL;
	}

	void setup(){
		pthread_attr_t attr;
		pthread_mutexattr_t mattr;
		if( pipe( shared.fd) == -1 ) {
			FAIL("Pipe failed");
			return;
		}
		pthread_mutexattr_init( &mattr);
		pthread_mutex_init( &shared.mutex, &mattr);
		// create reading thread
		pthread_attr_init( &attr );
		shared.buffer_ready=0;
		pthread_create( &tid, &attr, &read_thread, &shared);
	}

	void teardown(){
		char *result;
		pthread_join( tid, (void **)&result);
	}

};
TEST( g_dom, t_dom){
	dom_t *dom;
	dom_t *node;
	char buffer[1024];

	dom=dom_parse_buffer( XML, strlen(XML));
	CHECK_TRUE(dom);
	FILE* o=fdopen( shared.fd[1], "w");
	dom_print( o, dom, 0);
	fclose(o);
	close(shared.fd[1]);

	while( 1){
		pthread_mutex_lock( &shared.mutex);
		if(shared.buffer_ready){
			break;
		}
		pthread_mutex_unlock( &shared.mutex);
		usleep(10000);
	}

	STRCMP_EQUAL(XML, shared.buffer);

	node=dom_find_node( dom, "nam");
	CHECK_FALSE(node);

	node=dom_find_node( dom, "name");
	CHECK_TRUE(node);
	STRCMP_EQUAL( "name", node->name);
	LONGS_EQUAL( strlen( "Andy Dufresne"), node->data_len);
	strncpy( buffer, node->data, node->data_len);
	buffer[ node->data_len]=0;
	STRCMP_EQUAL( "Andy Dufresne", buffer);
	LONGS_EQUAL( strlen( "Andy Dufresne"), node->user_data_len);
	strncpy( buffer, node->user_data, node->user_data_len);
	buffer[ node->user_data_len]=0;
	STRCMP_EQUAL( "Andy Dufresne", buffer);
	CHECK_TRUE(node->next);
	STRCMP_EQUAL( "actor", node->next->name);
	LONGS_EQUAL( strlen( "Tim Robbins"), node->next->data_len);
	CHECK_FALSE(node->next->next);

	node=dom_find_node( dom, "movie");
	CHECK_TRUE(node);
	char *attr;
	attr=dom_find_attr( node->attr, "year");
	CHECK_TRUE(attr);
	STRCMP_EQUAL("1994", attr);

	dom=dom_free( dom);
	CHECK_FALSE(dom);

	void *parser=NULL;
	int ret;
	const char *str=XML;
	ret=dom_parse_chunked_data( &parser, &dom, str, 10, 0);
	CHECK_FALSE(ret);
	ret=dom_parse_chunked_data( &parser, &dom, str+10, strlen(str)-10, 1);
	CHECK_FALSE(ret);
	node=dom_find_node( dom, "movie");
	CHECK_TRUE(node);
	attr=dom_find_attr( node->attr, "year");
	CHECK_TRUE(attr);
	STRCMP_EQUAL("1994", attr);
	dom_free( parser);
	dom=dom_free( dom);
}

int main(int ac, char *av[]){
	return CommandLineTestRunner::RunAllTests(ac, av);
}
