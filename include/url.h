/**
 * @file url.h
 */
#ifndef SPM_SPM_URL_H
#define SPM_SPM_URL_H

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#  include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <curl/curl.h>

enum fcurl_type_e {
  CFTYPE_NONE = 0,
  CFTYPE_FILE = 1,
  CFTYPE_CURL = 2
};

struct fcurl_data
{
  enum fcurl_type_e type;     /* type of handle */
  union {
    CURL *curl;
    FILE *file;
  } handle;                   /* handle */

  char *buffer;               /* buffer to store cached data*/
  size_t buffer_len;          /* currently allocated buffers length */
  size_t buffer_pos;          /* end of data in buffer*/
  int still_running;          /* Is background url fetch still in progress */
  long http_status;           /* HTTP server response code */
};

typedef struct fcurl_data URL_FILE;

/* exported functions */
URL_FILE *url_fopen(const char *url, const char *operation);
int url_fclose(URL_FILE *file);
int url_feof(URL_FILE *file);
size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
char *url_fgets(char *ptr, size_t size, URL_FILE *file);
void url_rewind(URL_FILE *file);
const char *http_response_str(long code);
long get_http_response(CURLM *handle);
#endif // SPM_SPM_URL_H
