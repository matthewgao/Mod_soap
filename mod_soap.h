/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/* ====================================================================
 * Copyright (c) 2002, The Regents of the University of California.
 *
 * The Regents of the University of California MAKE NO REPRESENTATIONS OR
 * WARRANTIES ABOUT THE SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
 * NON-INFRINGEMENT. The Regents of the University of California SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF
 * USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 * ====================================================================
 */

#ifndef __mod_soap_H__
#define __mod_soap_H__


#include <apr_hooks.h>
#include <apr_hash.h>
#include <apr_dbm.h>

#include <httpd.h>
#include <util_filter.h>
#include <util_xml.h>

#define SOAP_DEBUG 1

#ifdef SOAP_DEBUG
#define DBG0(f)         ap_log_error(APLOG_MARK, \
                                APLOG_ERR, 0, NULL, (f))
#define DBG1(f,a1)      ap_log_error(APLOG_MARK, \
                                APLOG_ERR, 0, NULL, f, a1)
#define DBG2(f,a1,a2)   ap_log_error(APLOG_MARK, \
                                APLOG_ERR, 0, NULL, f, a1, a2)
#define DBG3(f,a1,a2,a3) ap_log_error(APLOG_MARK, \
                               APLOG_ERR, 0, NULL, f, a1, a2, a3)
#define TRACE() DBG0("- TRACE :"  __FUNCTION__ )
#else

#define DBG0(f)
#define DBG1(f,a1)
#define DBG2(f,a1,a2)
#define DBG3(f,a1,a2,a3)
#define TRACE() 
#endif

/* 
 * This is from DAV_DECLARE
 * Create a set of DAV_DECLARE(type), DAV_DECLARE_NONSTD(type) and 
 * DAV_DECLARE_DATA with appropriate export and import tags for the platform
 */
#if !defined(WIN32)
#define SOAP_DECLARE(type)            type
#define SOAP_DECLARE_NONSTD(type)     type
#define SOAP_DECLARE_DATA
#elif defined(SOAP_DECLARE_STATIC)
#define SOAP_DECLARE(type)            type __stdcall
#define SOAP_DECLARE_NONSTD(type)     type
#define SOAP_DECLARE_DATA
#elif defined(SOAP_DECLARE_EXPORT)
#define SOAP_DECLARE(type)            __declspec(dllexport) type __stdcall
#define SOAP_DECLARE_NONSTD(type)     __declspec(dllexport) type
#define SOAP_DECLARE_DATA             __declspec(dllexport)
#else
#define SOAP_DECLARE(type)            __declspec(dllimport) type __stdcall
#define SOAP_DECLARE_NONSTD(type)     __declspec(dllimport) type
#define SOAP_DECLARE_DATA             __declspec(dllimport)
#endif

#define SOAP_XML_HEADER          "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
#define SOAP_XML_CONTENT_TYPE    "text/xml; charset=\"utf-8\""
#define SOAP_CR                  "\n"



/* --------------------------------------------------------------------
**
** ERROR MANAGEMENT From dav_error
*/

/*
** soap_error structure.
**
** In most cases, mod_dav uses a pointer to a dav_error structure. If the
** pointer is NULL, then no error has occurred.
**
** In certain cases, a dav_error structure is directly used. In these cases,
** a status value of 0 means that an error has not occurred.
**
** Note: this implies that status != 0 whenever an error occurs.
**
** The desc field is optional (it may be NULL). When NULL, it typically
** implies that Apache has a proper description for the specified status.
*/
typedef struct soap_error {
    int status;			/* suggested HTTP status (0 for no error) */
    int error_id;		/* DAV-specific error ID */
    const char *desc;		/* DAV:responsedescription and error log */

    int save_errno;		/* copy of errno causing the error */

    const char *namespace;      /* [optional] namespace of error */
    const char *tagname;        /* name of error-tag */

    struct soap_error *prev;	/* previous error (in stack) */

} soap_error;

/*
** Create a new error structure. save_errno will be filled with the current
** errno value.
*/
SOAP_DECLARE(soap_error*) soap_new_error(apr_pool_t *p, int status, 
                                      int error_id, const char *desc);


/*
** Create a new error structure with tagname and (optional) namespace;
** namespace may be NULL, which means "DAV:". save_errno will be
** filled with the current errno value.
*/
SOAP_DECLARE(soap_error*) soap_new_error_tag(apr_pool_t *p, int status, 
                                          int error_id, const char *desc,
                                          const char *namespace,
                                          const char *tagname);

/*
** Push a new error description onto the stack of errors.
**
** This function is used to provide an additional description to an existing
** error.
**
** <status> should contain the caller's view of what the current status is,
** given the underlying error. If it doesn't have a better idea, then the
** caller should pass prev->status.
**
** <error_id> can specify a new error_id since the topmost description has
** changed.
*/
SOAP_DECLARE(soap_error*) soap_push_error(apr_pool_t *p, int status, int error_id,
                                       const char *desc, soap_error *prev);

/*
** XML Utilities
**
**
*/
/* validate that the root element uses a given DAV: tagname (TRUE==valid) */
int soap_validate_root(const ap_xml_doc *doc, const char *tagname);

/* find and return the (unique) child with a given DAV: tagname */
ap_xml_elem *soap_find_elem(const ap_xml_elem *elem, const char *tagname);

/* find and return the (unique) child with a given DAV: tagname */
ap_xml_elem *soap_find_child(const ap_xml_elem *elem, const char *tagname);

/* gather up all the CDATA into a single string */
SOAP_DECLARE(const char *) soap_xml_get_cdata(const ap_xml_elem *elem, apr_pool_t *pool,
					      int strip_white);


/*
** Make hash table with the args
*/
apr_hash_t *soap_mkarg_hash(const ap_xml_elem *start, apr_pool_t *pool);


/*
**
** Structures
**
*/

typedef struct soap_result {
    const char *tag;      /* Result tag */
    const char *value;    /* Result saved as String */
    const char *type_str; /* Result type in String. Ex: "int", "float" */
    ap_text   *text;     /* Write result for compound data */
    int type;             /* Reeult type */
    struct soap_result *next; /* For multi result */
}soap_result;


/* SOAP context */
typedef struct soap_ctx {
    request_rec *r;
    const char *method;  /* Method name */
    apr_hash_t *args;   /* Arguments table */
    soap_result *result; /* The result */
    /* Name space handling */
    int envelope_ns;
    int body_ns;
    int method_ns;
    apr_xml_doc *doc;
}soap_ctx;


typedef struct soap_arg {
    const char *key;
    const char *value;  /* value for simple data */
    const char *type;
    apr_xml_elem *elem; /* XML element for strucrue data */
}soap_arg;

soap_error * run_RPC(soap_ctx *sctx);

#endif /* End of the file */
