/********************************************/
/* machine generated file. do not edit!     */
/*                                          */
/* generated from: de_blinkenlights_blinkenlet_BlinkenKeyEvent.joxy  */
/********************************************/

#include "de_blinkenlights_blinkenlet_BlinkenKeyEvent.h"

jclass Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent;


jmethodID CID_de_blinkenlights_blinkenlet_BlinkenKeyEvent_new;

jobject de_blinkenlights_blinkenlet_BlinkenKeyEvent_new( JNIEnv* env , jint p0, jint p1)
 {
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_BlinkenKeyEvent);
    return (*env)->NewObject( env, Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent, CID_de_blinkenlights_blinkenlet_BlinkenKeyEvent_new , p0, p1 );
}


jboolean IsInstanceOf_de_blinkenlights_blinkenlet_BlinkenKeyEvent(JNIEnv* env, jobject object)
{
   INIT_CHECK1(env, de_blinkenlights_blinkenlet_BlinkenKeyEvent);
   return (*env)->IsInstanceOf(env, object, Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent);
}


int Init_de_blinkenlights_blinkenlet_BlinkenKeyEvent(JNIEnv* env)
{
    if( Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent != NULL ) return JOXY_TRUE;
    Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent = (*env)->FindClass(env, "de/blinkenlights/blinkenlet/BlinkenKeyEvent" );
    JNU_GLOBALIZE( Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent );
    RETURN_VAL_ON_EXCEPTION(env, JOXY_FALSE);

    CID_de_blinkenlights_blinkenlet_BlinkenKeyEvent_new = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_BlinkenKeyEvent, "<init>", "(II)V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );

    return JOXY_TRUE;
}


