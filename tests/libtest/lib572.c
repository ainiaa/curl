
/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 */

#include "test.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <curl/mprintf.h>

#include "memdebug.h"

/* build request url */
static char *suburl(const char *base, int i)
{
  return curl_maprintf("%s%.4d", base, i);
}

/*
 * Test GET_PARAMETER: PUT, HEARTBEAT, and POST
 */
int test(char *URL)
{
  int res;
  CURL *curl;
  int params;
  FILE *paramsf = NULL;
  struct_stat file_info;
  char *stream_uri = NULL;
  int request=1;
  struct curl_slist *custom_headers=NULL;

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  if ((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }


  test_setopt(curl, CURLOPT_HEADERDATA, stdout);
  test_setopt(curl, CURLOPT_WRITEDATA, stdout);
  test_setopt(curl, CURLOPT_VERBOSE, 1L);

  test_setopt(curl, CURLOPT_URL, URL);

  /* SETUP */
  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  test_setopt(curl, CURLOPT_RTSP_TRANSPORT, "Planes/Trains/Automobiles");
  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_SETUP);
  fprintf(stderr, "CPC: %s:%d\n", __FILE__, __LINE__);
  res = curl_easy_perform(curl);
  fprintf(stderr, "CPC: %s:%d\n", __FILE__, __LINE__);
  if(res)
    goto test_cleanup;

  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;
  fprintf(stderr, "CPC: %s:%d\n", __FILE__, __LINE__);

  /* PUT style GET_PARAMETERS */
  params = open("log/file572.txt", O_RDONLY);
  fstat(params, &file_info);
  close(params);

  paramsf = fopen("log/file572.txt", "rb");
  if(paramsf == NULL) {
    fprintf(stderr, "can't open log/file572.txt\n");
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_GET_PARAMETER);

  test_setopt(curl, CURLOPT_READDATA, paramsf);
  test_setopt(curl, CURLOPT_UPLOAD, 1L);
  test_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) file_info.st_size);

  fprintf(stderr, "CPC: %s:%d\n", __FILE__, __LINE__);
  res = curl_easy_perform(curl);
  if(res)
    goto test_cleanup;

  test_setopt(curl, CURLOPT_UPLOAD, 0L);
  fclose(paramsf);
  paramsf = NULL;
  fprintf(stderr, "CPC: %s:%d\n", __FILE__, __LINE__);

  /* Heartbeat GET_PARAMETERS */
  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  fprintf(stderr, "CPC: %s:%d\n", __FILE__, __LINE__);
  res = curl_easy_perform(curl);
  if(res)
    goto test_cleanup;

  /* POST GET_PARAMETERS */

  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_GET_PARAMETER);
  test_setopt(curl, CURLOPT_POSTFIELDS, "packets_received\njitter\n");

  res = curl_easy_perform(curl);
  if(res)
    goto test_cleanup;

  test_setopt(curl, CURLOPT_POSTFIELDS, NULL);

  /* Make sure we can do a normal request now */
  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_OPTIONS);
  res = curl_easy_perform(curl);

test_cleanup:

  if(paramsf)
    fclose(paramsf);

  if(stream_uri)
    free(stream_uri);

  if(custom_headers)
    curl_slist_free_all(custom_headers);

  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return res;
}
