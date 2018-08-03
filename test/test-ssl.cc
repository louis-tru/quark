/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "ngui/base/net.h"
#include "ngui/base/fs.h"
#include <uv.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace ngui;

#define error_report printf
#define info_report printf
#define clear_socket_error()    errno=0
#define SHUTDOWN(fd)    { shutdown((fd),0); close((fd)); }
#define SHUTDOWN2(fd)   { shutdown((fd),2); close((fd)); }

// destination IP address
const char* DEST_IP = "61.135.169.125";

// destination IP port
const int DEST_PORT = 443;

static SSL_CTX *sctx = nullptr;
static X509_STORE* root_cert_store = nullptr;

static void init_root_cert() {
	root_cert_store = X509_STORE_new();
	
	String cacert = Path::resources("cacert.pem");
	
	X509_STORE_load_locations(root_cert_store, Path::fallback_c(cacert), nullptr);
	
}

static void init_ssl() {
	
	static int i = 0;
	
	if ( i++ == 0 ) {
		
		// Initialize ssl libraries and error messages
		SSL_load_error_strings();
		SSL_library_init();
		
		init_root_cert();
		
		sctx = SSL_CTX_new(SSLv23_client_method());
		
		SSL_CTX_set_cert_store(sctx, root_cert_store);
		
		SSL_CTX_set_verify(sctx, SSL_VERIFY_PEER, NULL);
		// SSL_CTX_set_cert_verify_callback(sctx, cert_verify_callback, nullptr);
	}
}

static int bio_write(BIO *b, const char *in, int inl) {

	int ret;
	
	clear_socket_error();
	ret = write(b->num, in, inl);
	BIO_clear_retry_flags(b);
	if (ret <= 0) {
		if (BIO_sock_should_retry(ret))
			BIO_set_retry_write(b);
	}
	return (ret);
}

static int bio_puts(BIO *bp, const char *str) {
	int n, ret;
	
	n = strlen(str);
	ret = bio_write(bp, str, n);
	return (ret);

}

static int bio_read(BIO *b, char *out, int outl) {
	int ret = 0;
	
	//static int i = 100;
	//if ( i > 0 ) {
	//  i--;
	//  LOG(i);
	//  return 0;
	//}
	
	if (out != NULL) {
		clear_socket_error();
		ret = read(b->num, out, outl);
		BIO_clear_retry_flags(b);
		if (ret <= 0) {
			if (BIO_sock_should_retry(ret))
				BIO_set_retry_read(b);
		}
	}
	
	return (ret);
}

static long bio_ctrl(BIO *b, int cmd, long num, void *ptr) {
	long ret = 1;
	int *ip;
	
	switch (cmd) {
		case BIO_C_SET_FD:
			b->num = *((int *)ptr);
			b->shutdown = (int)num;
			b->init = 1;
			break;
		case BIO_C_GET_FD:
			if (b->init) {
				ip = (int *)ptr;
				if (ip != NULL)
					*ip = b->num;
				ret = b->num;
			} else
				ret = -1;
			break;
		case BIO_CTRL_GET_CLOSE:
			ret = b->shutdown;
			break;
		case BIO_CTRL_SET_CLOSE:
			b->shutdown = (int)num;
			break;
		case BIO_CTRL_DUP:
		case BIO_CTRL_FLUSH:
			ret = 1;
			break;
		default:
			ret = 0;
			break;
	}
	return (ret);
}

static BIO_METHOD method = {
	BIO_TYPE_MEM,
	"socket",
	bio_write,
	bio_read,
	bio_puts,
	nullptr,    /* sock_gets, */
	bio_ctrl,
	nullptr,
	nullptr,
	nullptr
};

static void SSLInfoCallback(const SSL* ssl_, int where, int ret) {
	if (where & SSL_CB_HANDSHAKE_START) {
		LOG("----------------start");
	}
	if ( where & SSL_CB_HANDSHAKE_DONE ) {
		LOG("----------------done");
	}
}

static int SSLCertCallback(SSL* s, void* arg) {
	LOG("SSLCertCallback");
	return 1;
}

void test_ssl() {
	
	init_ssl();
	
	// request to send to the destination
	const char* REQUEST =
	"GET / HTTP/1.1\r\n"
	"Connection: close\r\n"
	"Host: www.baidu.com\r\n\r\n";
	
	// create a socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Unable to create socket");
		return;
	}
	
	// destination info
	struct sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET; 	    // host byte order
	dest_addr.sin_port = htons(DEST_PORT);    // short, network port
	dest_addr.sin_addr.s_addr = inet_addr(DEST_IP); // destination address
	memset(&(dest_addr.sin_zero), '\0', 8);         // zero out the rest of the struct
	
	// connect to the server
	int status = connect(sockfd, (struct sockaddr*) &dest_addr, sizeof(struct sockaddr_in));
	if (status == -1) {
		perror("Unable to connect to the server");
		close(sockfd);
		return;
	}
	
	// create SSL connection and attach it to the socket
	SSL *ssl = SSL_new(sctx);
	
	SSL_set_app_data(ssl, nullptr);
	SSL_set_info_callback(ssl, &SSLInfoCallback);
	
	BIO* bio = BIO_new(&method);
	BIO_set_fd(bio, sockfd, BIO_NOCLOSE);
	SSL_set_bio(ssl, bio, bio);
	
	// BIO_new_socket(int sock, int close_flag)
	
	while( SSL_connect(ssl) == 0 ) {
		SimpleThread::sleep_for(1e4);
	}
	
	// send an encrypted message
	ssize_t sendsize = SSL_write(ssl, REQUEST, (int)strlen(REQUEST));
	if ( sendsize == -1 ) {
		perror("Unable to send to the server");
		{ //
			char buf[256];
			u_long err;
			
			while ((err = ERR_get_error()) != 0) {
				ERR_error_string_n(err, buf, sizeof(buf));
				printf("*** %s\n", buf);
			}
		}
		SSL_shutdown(ssl);
		SSL_free(ssl);
		close(sockfd);
		return;
	}
	
	for (;;) {
		
		// receive the response
		const int RESPONSE_SIZE = 512;
		char response[RESPONSE_SIZE];
		ssize_t recsize = SSL_read(ssl, response, RESPONSE_SIZE-1);
		
		if ( recsize == -1 ) {
			perror("Unable to send to the server");
			SSL_shutdown(ssl);
			SSL_free(ssl);
			close(sockfd);
			return;
		}
		
		response[recsize] = '\0';
		
		write(STDOUT_FILENO, response, recsize);
		
		// break out of while if there is an error or no bytes read
		if (recsize <= 0)
			break;
	}
	
	// close ssl connection
	SSL_shutdown(ssl);
	
	SSL_free(ssl);
	
	// close the socket
	close(sockfd);
	
	printf("\n");
	
}
