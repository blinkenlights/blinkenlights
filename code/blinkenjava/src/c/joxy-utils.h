#ifndef _JOXY_UTILS_H
#define _JOXY_UTILS_H
#include <stddef.h>
/*
 * This file is automatically created by joxy, it will be included
 * by every class-specific file that is generated
 * You can safely remove it, as it will be regenerated if necessary
 */

#define CHECK_EXCEPTIONS

/* Globalizing a reference. This creates a new global reference from a given reference and assigns it to the
   same variable. don't forget to free it somewhen
*/
#define JNU_GLOBALIZE( ref ) { \
  jobject ___tmp = ref; \
  ref = (*env)->NewGlobalRef(env, ref); \
  (*env)->DeleteLocalRef(env, ___tmp); \
}


#define RETURN_ON_EXCEPTION(env) RETURN_VAL_ON_EXCEPTION( env,   );  // Yes, empty second arg!

#define RETURN_VAL_ON_EXCEPTION(env, val) { \
                                        jthrowable exc = (*env)->ExceptionOccurred(env); \
                                        if( exc != NULL ) { \
  		                           (*env)->ExceptionDescribe(env); \
				           (*env)->DeleteLocalRef(env, exc); \
                                           return val; \
                                        } \
				 }

#define INIT_CHECK1(env, class) if( Class ## _ ## class == 0 ) Init ## _ ## class(env);

#ifdef CHECK_EXCEPTIONS
#define EXCEPTION_CHECK(env) { \
                                 jthrowable exc = (*env)->ExceptionOccurred(env); \
                                 if( exc != NULL ) { \
  		                            (*env)->ExceptionDescribe(env); \
				                    (*env)->DeleteLocalRef(env, exc); \
                                 } \
				             }
#else
#define EXCEPTION_CHECK(env)
#endif

#define JOXY_TRUE 1
#define JOXY_FALSE 0

#endif

