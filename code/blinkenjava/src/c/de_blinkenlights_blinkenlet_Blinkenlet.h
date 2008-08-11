/********************************************/
/* machine generated file. do not edit!     */
/*                                          */
/* generated from: de_blinkenlights_blinkenlet_Blinkenlet.joxy  */
/********************************************/

#ifndef DE_BLINKENLIGHTS_BLINKENLET_BLINKENLET_H
#define DE_BLINKENLIGHTS_BLINKENLET_BLINKENLET_H
#include <jni.h>
#include "joxy-utils.h"
extern jclass Class_de_blinkenlights_blinkenlet_Blinkenlet;
jboolean IsInstanceOf_de_blinkenlights_blinkenlet_Blinkenlet(JNIEnv* env, jobject object);
int Init_de_blinkenlights_blinkenlet_Blinkenlet(JNIEnv* env);


extern jmethodID CID_de_blinkenlights_blinkenlet_Blinkenlet_new;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_getTitle;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_prepare;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_startInternal;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_stopInternal;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_getDescription;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_getAuthor;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal;
extern jmethodID MID_de_blinkenlights_blinkenlet_Blinkenlet_tickInternal;

/* public void <init>(String title, String author, String description) as "new" */
jobject de_blinkenlights_blinkenlet_Blinkenlet_new( JNIEnv* env , jstring p0, jstring p1, jstring p2)
;
/* boolean prepareInternal(int width, int height, int channels, int maxVal)  */
jboolean de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal( JNIEnv* env , jobject this , jint p0, jint p1, jint p2, jint p3);
/* protected String getTitle()  */
jstring de_blinkenlights_blinkenlet_Blinkenlet_getTitle( JNIEnv* env , jobject this );
/* public abstract boolean prepare(int arg1, int arg2, int arg3, int arg4)  */
jboolean de_blinkenlights_blinkenlet_Blinkenlet_prepare( JNIEnv* env , jobject this , jint p0, jint p1, jint p2, jint p3);
/* void startInternal()  */
void de_blinkenlights_blinkenlet_Blinkenlet_startInternal( JNIEnv* env , jobject this );
/* void stopInternal()  */
void de_blinkenlights_blinkenlet_Blinkenlet_stopInternal( JNIEnv* env , jobject this );
/* protected String getDescription()  */
jstring de_blinkenlights_blinkenlet_Blinkenlet_getDescription( JNIEnv* env , jobject this );
/* protected String getAuthor()  */
jstring de_blinkenlights_blinkenlet_Blinkenlet_getAuthor( JNIEnv* env , jobject this );
/* void handleEventInternal(de.blinkenlights.blinkenlet.BlinkenEvent event)  */
void de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal( JNIEnv* env , jobject this , jobject p0);
/* int tickInternal()  */
jint de_blinkenlights_blinkenlet_Blinkenlet_tickInternal( JNIEnv* env , jobject this );


#endif
