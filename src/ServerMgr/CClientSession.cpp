/*
This is an example of how to hook up evhttp with bufferevent_ssl

It just GETs an https URL given on the command-line and prints the response
body to stdout.

Actually, it also accepts plain http URLs to make it easy to compare http vs
https code paths.

Loosely based on le-proxy.c.
*/
#include "CClientSession.h"
// Get rid of OSX 10.7 and greater deprecation warnings.


static struct event_base *base;
static int ignore_cert = 0;

static void
http_request_done(struct evhttp_request *req, void *ctx)
{
	CClientSession* pSession = (CClientSession*)ctx;
	if (req == NULL || pSession == NULL) {
		LOG_WARNING << "http failed";
		return;
	}
	LOG_INFO << "recv http resp code =" << evhttp_request_get_response_code(req) << "body = " << evhttp_request_get_response_code_line(req);
	pSession->OnResult(req);

}

static void
syntax(void)
{
	fputs("Syntax:\n", stderr);
	fputs("   https-client -url <https-url> [-data data-file.bin] [-ignore-cert] [-retries num] [-timeout sec] [-crt crt]\n", stderr);
	fputs("Example:\n", stderr);
	fputs("   https-client -url https://ip.appspot.com/\n", stderr);
}

static void
err(const char *msg)
{
	fputs(msg, stderr);
}


CClientSession::CClientSession()
:m_retries(0)
, m_timeout(0)
, m_bev(nullptr)
{
	memset(m_retBody, 0, BODYLEN);
}


CClientSession::~CClientSession()
{

}

int
CClientSession::http_client_short_link(const char* url, const char* buf, int iBodyLen, string& retBody)
{
	int r;

	struct evhttp_uri *http_uri = NULL;
	const char *crt = NULL;
	const char *scheme, *host, *path, *query;
	char uri[256];
	int port;

	struct evhttp_connection *evcon = NULL;
	struct evhttp_request *req;
	struct evkeyvalq *output_headers;
	struct evbuffer *output_buffer;
	int ret = 0;
	http_uri = evhttp_uri_parse(url);
	if (http_uri == NULL) {
		err("malformed url");
		goto error;
	}

	scheme = evhttp_uri_get_scheme(http_uri);
	if (scheme == NULL || (strcasecmp(scheme, "https") != 0 &&
		strcasecmp(scheme, "http") != 0)) {
		err("url must be http or https");
		goto error;
	}

	host = evhttp_uri_get_host(http_uri);
	if (host == NULL) {
		err("url must have a host");
		goto error;
	}

	port = evhttp_uri_get_port(http_uri);
	if (port == -1) {
		port = (strcasecmp(scheme, "http") == 0) ? 80 : 443;
	}

	path = evhttp_uri_get_path(http_uri);
	if (strlen(path) == 0) {
		path = "/";
	}
	query = evhttp_uri_get_query(http_uri);
	if (query == NULL) {
		snprintf(uri, sizeof(uri)-1, "%s", path);
	}
	else {
		snprintf(uri, sizeof(uri)-1, "%s?%s", path, query);
	}
	uri[sizeof(uri)-1] = '\0';


	/* This isn't strictly necessary... OpenSSL performs RAND_poll
	* automatically on first use of random number generator. */

	/* Create a new OpenSSL context */


	(void)crt;
	base = event_base_new();
	if (!base) {
		perror("event_base_new()");
		goto error;
	}

	if (strcasecmp(scheme, "http") == 0) {
		m_bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	}

	if (m_bev == NULL) {
		fprintf(stderr, "bufferevent_openssl_socket_new() failed\n");
		goto error;
	}

	// For simplicity, we let DNS resolution block. Everything else should be
	// asynchronous though.
	evcon = evhttp_connection_base_bufferevent_new(base, NULL, m_bev,
		host, port);
	if (evcon == NULL) {
		fprintf(stderr, "evhttp_connection_base_bufferevent_new() failed\n");
		goto error;
	}

	if (m_retries > 0) {
		evhttp_connection_set_retries(evcon, m_retries);
	}
	if (m_timeout >= 0) {
		evhttp_connection_set_timeout(evcon, m_timeout);
	}

	// Fire off the request
	req = evhttp_request_new(http_request_done, this);
	if (req == NULL) {
		fprintf(stderr, "evhttp_request_new() failed\n");
		goto error;
	}

	output_headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_headers, "Host", host);
	evhttp_add_header(output_headers, "Connection", "close");

	if (buf) {
		/* NOTE: In production code, you'd probably want to use
		* evbuffer_add_file() or evbuffer_add_file_segment(), to
		* avoid needless copying. */
		size_t bytes = 0;

		output_buffer = evhttp_request_get_output_buffer(req);
		evbuffer_add(output_buffer, buf, iBodyLen);
		char Content[16] = { 0 };
		evutil_snprintf(Content, sizeof(buf)-1, "%lu", iBodyLen);
		evhttp_add_header(output_headers, "Content-Length", Content);
	}

	r = evhttp_make_request(evcon, req, buf ? EVHTTP_REQ_POST : EVHTTP_REQ_GET, uri);
	if (r != 0) {
		fprintf(stderr, "evhttp_make_request() failed\n");
		goto error;
	}

	event_base_dispatch(base);
	retBody = m_retBody;
	goto cleanup;

error:
	ret = 1;
cleanup:
	if (evcon)
		evhttp_connection_free(evcon);
	if (http_uri)
		evhttp_uri_free(http_uri);
	event_base_free(base);

#ifdef _WIN32
	WSACleanup();
#endif

	return ret;
}

void CClientSession::OnResult(evhttp_request* req)
{
	if (evbuffer_remove(evhttp_request_get_input_buffer(req), m_retBody, BODYLEN) > 0)
	{
		LOG_WARNING << "cant deal with too long msg";
		return;
	}
}
