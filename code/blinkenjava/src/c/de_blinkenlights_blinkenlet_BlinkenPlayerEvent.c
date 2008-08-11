/********************************************/
/* machine generated file. do not edit!     */
/*                                          */
/********************************************/

#include "de_blinkenlights_blinkenlet_BlinkenPlayerEvent.h"

jclass Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent;


jmethodID CID_de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new;

jobject de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new( JNIEnv* env , jint p0, jint p1)
 {
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_BlinkenPlayerEvent);
    return (*env)->NewObject( env, Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent, CID_de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new , p0, p1 );
}


jboolean IsInstanceOf_de_blinkenlights_blinkenlet_BlinkenPlayerEvent(JNIEnv* env, jobject object)
{
   INIT_CHECK1(env, de_blinkenlights_blinkenlet_BlinkenPlayerEvent);
   return (*env)->IsInstanceOf(env, object, Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent);
}


int Init_de_blinkenlights_blinkenlet_BlinkenPlayerEvent(JNIEnv* env)
{
    if( Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent != NULL ) return JOXY_TRUE;
    Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent = (*env)->FindClass(env, "de/blinkenlights/blinkenlet/BlinkenPlayerEvent" );
    JNU_GLOBALIZE( Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent );
    RETURN_VAL_ON_EXCEPTION(env, JOXY_FALSE);

    CID_de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent, "<init>", "(II)V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );

    return JOXY_TRUE;
}


