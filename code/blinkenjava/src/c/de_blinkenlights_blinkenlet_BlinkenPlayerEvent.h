
#ifndef DE_BLINKENLIGHTS_BLINKENLET_BLINKENPLAYEREVENT_H
#define DE_BLINKENLIGHTS_BLINKENLET_BLINKENPLAYEREVENT_H
#include <jni.h>
#include "joxy-utils.h"
extern jclass Class_de_blinkenlights_blinkenlet_BlinkenPlayerEvent;
jboolean IsInstanceOf_de_blinkenlights_blinkenlet_BlinkenPlayerEvent(JNIEnv* env, jobject object);
int Init_de_blinkenlights_blinkenlet_BlinkenPlayerEvent(JNIEnv* env);


extern jmethodID CID_de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new;

/* void <init>(int deviceId, int key) as "new" */
jobject de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new( JNIEnv* env , jint p0, jint p1)
;


#endif
