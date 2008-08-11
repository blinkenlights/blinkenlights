/********************************************/
/* machine generated file. do not edit!     */
/*                                          */
/* generated from: de_blinkenlights_blinkenlet_Blinkenlet.joxy  */
/********************************************/

#include "de_blinkenlights_blinkenlet_Blinkenlet.h"

jclass Class_de_blinkenlights_blinkenlet_Blinkenlet;


jmethodID CID_de_blinkenlights_blinkenlet_Blinkenlet_new;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_getTitle;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_prepare;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_startInternal;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_stopInternal;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_getDescription;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_getAuthor;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal;
jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_tickInternal;

jobject de_blinkenlights_blinkenlet_Blinkenlet_new( JNIEnv* env , jstring p0, jstring p1, jstring p2)
 {
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->NewObject( env, Class_de_blinkenlights_blinkenlet_Blinkenlet, CID_de_blinkenlights_blinkenlet_Blinkenlet_new , p0, p1, p2 );
}

/* boolean prepareInternal(int width, int height, int channels, int maxVal)  */
jboolean de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal( JNIEnv* env , jobject this , jint p0, jint p1, jint p2, jint p3)
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->CallBooleanMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal, p0, p1, p2, p3);
}

/* protected String getTitle()  */
jstring de_blinkenlights_blinkenlet_Blinkenlet_getTitle( JNIEnv* env , jobject this )
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->CallObjectMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_getTitle);
}

/* public abstract boolean prepare(int arg1, int arg2, int arg3, int arg4)  */
jboolean de_blinkenlights_blinkenlet_Blinkenlet_prepare( JNIEnv* env , jobject this , jint p0, jint p1, jint p2, jint p3)
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->CallBooleanMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_prepare, p0, p1, p2, p3);
}

/* void startInternal()  */
void de_blinkenlights_blinkenlet_Blinkenlet_startInternal( JNIEnv* env , jobject this )
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    (*env)->CallVoidMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_startInternal);
}

/* void stopInternal()  */
void de_blinkenlights_blinkenlet_Blinkenlet_stopInternal( JNIEnv* env , jobject this )
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    (*env)->CallVoidMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_stopInternal);
}

/* protected String getDescription()  */
jstring de_blinkenlights_blinkenlet_Blinkenlet_getDescription( JNIEnv* env , jobject this )
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->CallObjectMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_getDescription);
}

/* protected String getAuthor()  */
jstring de_blinkenlights_blinkenlet_Blinkenlet_getAuthor( JNIEnv* env , jobject this )
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->CallObjectMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_getAuthor);
}

/* void handleEventInternal(de.blinkenlights.blinkenlet.BlinkenEvent event)  */
void de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal( JNIEnv* env , jobject this , jobject p0)
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    (*env)->CallVoidMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal, p0);
}

/* int tickInternal()  */
jint de_blinkenlights_blinkenlet_Blinkenlet_tickInternal( JNIEnv* env , jobject this )
{
    INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
    return (*env)->CallIntMethod( env, this, MID_de_blinkenlights_blinkenlet_Blinkenlet_tickInternal);
}


jboolean IsInstanceOf_de_blinkenlights_blinkenlet_Blinkenlet(JNIEnv* env, jobject object)
{
   INIT_CHECK1(env, de_blinkenlights_blinkenlet_Blinkenlet);
   return (*env)->IsInstanceOf(env, object, Class_de_blinkenlights_blinkenlet_Blinkenlet);
}


int Init_de_blinkenlights_blinkenlet_Blinkenlet(JNIEnv* env)
{
    if( Class_de_blinkenlights_blinkenlet_Blinkenlet != NULL ) return JOXY_TRUE;
    Class_de_blinkenlights_blinkenlet_Blinkenlet = (*env)->FindClass(env, "de/blinkenlights/blinkenlet/Blinkenlet" );
    JNU_GLOBALIZE( Class_de_blinkenlights_blinkenlet_Blinkenlet );
    RETURN_VAL_ON_EXCEPTION(env, JOXY_FALSE);

    CID_de_blinkenlights_blinkenlet_Blinkenlet_new = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "prepareInternal", "(IIII)Z");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_getTitle = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "getTitle", "()Ljava/lang/String;");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_prepare = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "prepare", "(IIII)Z");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_startInternal = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "startInternal", "()V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_stopInternal = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "stopInternal", "()V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_getDescription = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "getDescription", "()Ljava/lang/String;");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_getAuthor = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "getAuthor", "()Ljava/lang/String;");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "handleEventInternal", "(Lde/blinkenlights/blinkenlet/BlinkenEvent;)V");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );
    MID_de_blinkenlights_blinkenlet_Blinkenlet_tickInternal = (*env)->GetMethodID(env, Class_de_blinkenlights_blinkenlet_Blinkenlet, "tickInternal", "()I");
    RETURN_VAL_ON_EXCEPTION( env, JOXY_FALSE );

    return JOXY_TRUE;
}


