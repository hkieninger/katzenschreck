/*
 * cgi.h
 *
 *  Created on: Oct 29, 2017
 *      Author: hans
 */

#ifndef CGI_H
#define CGI_H

/* returns the a buffer containing the cgi data independent of the method (GET or POST)
 * the buffer returned must be freed with free
 *  @return: the cgi data or NULL if an error occured */
char *cgi_data(void);

/* some special characters returned by cgi_data() are encoded in hex and must be parsed, this can be done with this function
 *  @str: the cgi_data to be parsed */
void cgi_data2string(char *str);

#endif
