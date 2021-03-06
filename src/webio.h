/* webio.h
 *
 * Part of the Webio Open Source lightweight web server.
 *
 * Copyright (c) 2007 by John Bartas
 * All rights reserved.
 *
 * Use license: Modified from standard BSD license.
 * 
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation, advertising 
 * materials, Web server pages, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by John Bartas. The name "John Bartas" may not be used to 
 * endorse or promote products derived from this software without 
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef _WEBIO_H_
#define _WEBIO_H_    1

struct wi_sess_s;    /* predecl */

/* Port number on which to listen. May be changed prior to calling wi_init */
extern   int   httpport;

typedef enum httpcmd {
   H_INITIAL = 0,
   H_GET = 0x47455420,
   H_POST = 0x504F5354,
   H_PUT = 0x50555420,
   H_DONE = -1,
} httpcmds;


/* Data to send is held in a list of txbuf structures */
typedef struct txbuf_s {
   struct txbuf_s * tb_next;           /* list link */
   struct   wi_sess_s * tb_session;    /* backpointer to session */
   int      tb_total;                  /* Size of data in tb_data */
   int      tb_done;                   /* amount of tb_data already sent */
   char     tb_data[WI_TXBUFSIZE];     /* Data buffer for this segment */
} txbuf;


/* A list of filectl objects is keptfor every session that is reading 
 * any kinf of file, The "Active" file (the one currently being read) 
 * is at the head of the list. Thus if "index.html" contains an SSI 
 * named (for example) "header.html", then while header.html is being 
 * read into txbufs, the filectl for header.htmlis at the front of the 
 * list, and it's wf_next link points to the filectl for index.html.
 */

struct wi_file_s;    /* predecl */

typedef long   wi_sec;     /* A number of seconds, for timeouts */

/* States of a sesison */
typedef enum wistates { 
   WI_HEADER,        /* Getting HTTP header form socket */
   WI_POSTRX,        /* waiting for POST name value pairs */
   WI_CONTENT,       /* reading file from disk or script */
   WI_SENDDATA,      /* Sending file/data into socket */
   WI_PUSHING,       /* session is owned by a server push routine */
   WI_ENDING         /* Sessions done,cleaning up for deletion */
} wistate;


typedef struct wi_sess_s {
   struct   wi_sess_s * ws_next;    /* queue link */
   socktype ws_socket;
   wistate  ws_state;

   char     ws_rxbuf[WI_RXBUFSIZE]; /* input from browser */
   int      ws_rxsize;              /* size of valid data in rxbuf */
   int      ws_contentLength;       /* size of current sess data */
   char *   ws_data;                /* start of contetnt */

   txbuf *  ws_txbufs;              /* list of output buffers ready to send */
   txbuf *  ws_txtail;              /* last entry in ws_txbufs list */

   const char * ws_uri;             /* URI from request (often inside rxbuf) */
   const char * ws_referer;         /* Referrer Information */
   const char * ws_auth;
   const char * ws_host;

   struct wi_form_s * ws_formlist;  /* attached forms (once parsed) */
   struct wi_file_s * ws_filelist;  /* local files associated with session */

   httpcmds     ws_cmd;             /* GET, POST, etc. */
   int          ws_flags;
   const char * ws_ftype;           /* Mime type (best guess) */
   wi_sec       ws_last;            /* timetick of last activity */
} wi_sess;   


typedef struct wi_pair_s {
   char * name;
   char * value;
} wi_pair;

typedef struct wi_form_s {
   struct wi_form_s * next;
   int      paircount;
#ifdef MAX_FORM_PARAMS
   wi_pair  pairs[MAX_FORM_PARAMS];
#else
   wi_pair  pairs[1];   /* Size actually will be paircount */
#endif
} wi_form;

extern   wi_sess * wi_sessions;

#define WF_READINGCMDS     0x0001      /* Still reading socket for commands from browser */
#define WF_SSL             0x0004      /* Socket is SSL socket */
#define WF_HEADERSENT      0x0008      /* Header sent for current write */
#define WF_BINARY          0x0010      /* current file is binary (no SSIs) */
#define WF_PERSIST         0x0020      /* connection is persistent */
#define WF_SVRPUSH         0x0040      /* current file is custom server push */


#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

/* Error codes from Webio calls - MUST be negative */

#define WI_E_ERRORBASE   -1000
#ifndef WI_E_SOCKET
#define WI_E_SOCKET   WI_E_ERRORBASE - 1    /* error from sockets api */
#endif
#ifndef WI_E_MEMORY
#define WI_E_MEMORY   WI_E_ERRORBASE - 2    /* out of heap */
#endif
#ifndef WI_E_CLIENT
#define WI_E_CLIENT   WI_E_ERRORBASE - 3    /* browser error */
#endif
#ifndef WI_E_NOFILE
#define WI_E_NOFILE   WI_E_ERRORBASE - 4    /* file not found */
#endif
#ifndef WI_E_BADFILE
#define WI_E_BADFILE  WI_E_ERRORBASE - 5    /* file IO error */
#endif
#ifndef WI_E_BADPARM
#define WI_E_BADPARM  WI_E_ERRORBASE - 6    /* Invalid parameter */
#endif
#ifndef WI_E_FORMAT
#define WI_E_FORMAT  WI_E_ERRORBASE - 7     /* Bad http/html format */
#endif
#ifndef WI_E_PERMIT
#define WI_E_PERMIT   WI_E_ERRORBASE - 8     /* wrong user permissions */
#endif


#define HDRBUFSIZE   1000
extern char hdrbuf[HDRBUFSIZE];   /* For building HTTP headers */

extern   char * wi_servername;

extern   int         wi_init(void);
extern   int         wi_poll(void);

#ifdef WI_USE_MALLOC
extern   char *      wi_alloc(int bufsize);
extern   void        wi_free(void *);
#endif

extern   txbuf *     wi_txalloc( wi_sess *);
extern   void        wi_txfree( txbuf *);

extern   wi_sess *   wi_newsess(void);
extern   void        wi_delsess( wi_sess *);

extern   void        wi_printf(wi_sess * sess, char * fmt, ...);
extern   int         wi_readfile(struct wi_sess_s * sess);
extern   int         wi_sockwrite(struct wi_sess_s * sess);
extern   int         wi_sockaccept(void);
extern   int         wi_parseheader( wi_sess * sess );
extern   int         wi_putfile( wi_sess * sess);
extern   int         wi_senderr(wi_sess * sess, int htmlcode );
extern   char *      wi_getline( char * linetype, char * httphdr );
extern   char *      wi_nextarg( char * argbuf );
extern   int         wi_argncpy(char * buf, char * arg, int size);
extern   int         wi_buildform(wi_sess * sess, char * cp);
extern   char *      wi_argterm( char * arg );
extern   int         wi_setftype(wi_sess * sess);
extern   char *      wi_getdate(wi_sess * sess);
extern   int         wi_replyhdr(wi_sess * sess, int contentLen);
extern   int         wi_txdone(wi_sess * sess);
extern   int         wi_ssi(wi_sess * sess);
extern   int         wi_exec(wi_sess * sess);
extern   int         wi_putlong(wi_sess * sess, u_long value);
extern   int         wi_putstring(wi_sess * sess, char * string);
extern   int         wi_cvariables(wi_sess * sess, int token);
extern   int         wi_redirect(wi_sess * sess, const char * filename);
extern   int         wi_redirect_get(wi_sess * sess, char * filename);
extern   void        wi_decode_auth(wi_sess * sess, char * name, int name_len, char * pass, int pass_len);
extern   char *      wi_formipaddr( wi_sess * sess, char * ipname, u_long * ipaddr);
extern   char *      wi_formvalue( wi_sess * sess, char * ctlname );
extern   int         wi_formint(wi_sess * sess, char * name, long * return_int );
extern   int         wi_formbool(wi_sess * sess, char * name);

extern   int         wi_step();

#ifdef WI_USE_THREADS
/* Entry point for main (or only) thread in demo */
extern   int         wi_thread(void);
#endif /* WI_USE_THREADS */

/* Optional "exec" routine */
extern   int         (*wi_execfunc)(wi_sess * sess, char * args);

#endif   /* _WEBIO_H_ */

