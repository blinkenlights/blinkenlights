/********************************************/
/* machine generated file. do not edit!     */
/*                                          */
/* generated from: java_lang_Throwable.joxy  */
/********************************************/

#include "java_lang_Throwable.h"

jclass Class_java_lang_Throwable;


jmethodID MID_java_lang_Throwable_printStackTrace;

/* public void printStackTrace()  */
void java_lang_Throwable_printStackTrace( JNIEnv* env , jobject this )
{
    
    INIT_CHECK1(env, java_lang_Throwable);
    (*env)->CallVoidMethod( env, this, MID_java_lang_Throwable_printStackTrace);
    EXCEPTION_CHECK(env);
    return ;
}


jboolean IsInstanceOf_java_lang_Throwable(JNIEnv* env, jobject object)
{
   INIT_CHECK1(env, java_lang_Throwable);
   return (*env)->IsInstanceOf(env, object, Class_java_lang_Throwable);
}


int Init_java_lang_Throwable(JNIEnv* env)
{
    if( Class_java_lang_Throwable != NULL ) return JOXY_TRUE;
    Class_java_lang_Throwable = (*env)->FindClass(env, "java/lang/Throwable" );
    JNU_GLOBALIZE( Class_java_lang_Throwable );
    RETURN_VAL_ON_EXCEPTION(env, JOXY_FALSE);

    MID_java_lang_Throwable_printStackTrace = (*env)->GetMethodID(env, Class_java_lang_Throwable, "printStackTrace", "()V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );

    return JOXY_TRUE;
}


