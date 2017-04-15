#ifndef BF_PARSE_H
#define BF_PARSE_H

#define CR  '\r'
#define LF  '\n'
#define CRLFCRLF  '\r\n\r\n'

int bf_parse_request_line(bf_request_t *r);
int bf_parse_request_body(bf_request_t *r);

#endif
