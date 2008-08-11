/* blinkenjava - The Java bindings for BModules
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Enno Brehm <enno@selfish.org>
 *                     Miriam Busch <>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdlib.h>

#include <glib-object.h>
#include <gmodule.h>

#include <blib/blib.h>

#include <jni.h>

#include "de_blinkenlights_blinkenlet_Blinkenlet.h"
#include "de_blinkenlights_blinkenlet_BlinkenKeyEvent.h"
#include "de_blinkenlights_blinkenlet_BlinkenPlayerEvent.h"
#include "java_lang_Throwable.h"


/* the name of the environment variable to check for the
 * name of a blinkenlet class 
 */
#define BLINKENLET_CLASS_VAR     "BLINKENLET_CLASS"
/* if no classname is given, use this class */
#define DEFAULT_BLINKENLET_CLASS "de.blinkenlights.examples.BlinkenLife"

#define TYPE_BLINKENJAVA         (type_blinkenjava)
#define BLINKENJAVA(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_BLINKENJAVA, Blinkenjava))
#define BLINKENJAVA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_BLINKENJAVA, BlinkenjavaClass))
#define IS_BLINKENJAVA(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_BLINKENJAVA))
#define BLINKENJAVA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_BLINKENJAVA, BlinkenjavaClass))

typedef struct _Blinkenjava      Blinkenjava;
typedef struct _BlinkenjavaClass BlinkenjavaClass;

struct _Blinkenjava
{
  BModule  parent_instance;
   
  /* the name of the blinkenlet class */
  GString *jclassname;

  JNIEnv* env;
  jobject the_blinkenlet;

  GString *title;
  GString *author;
  GString *description;
};

struct _BlinkenjavaClass
{
  BModuleClass  parent_class;

  /* the VM */
  JavaVM *jvm;
};

enum 
  {
    PROP_0,
    PROP_CLASS
  };

static GType      blinkenjava_get_type   (GTypeModule      *module);

static void       blinkenjava_class_init (BlinkenjavaClass *klass);
static void       blinkenjava_base_class_init (BlinkenjavaClass *klass);
static void       blinkenjava_base_class_finalize (BlinkenjavaClass *klass);

static void       blinkenjava_init       (Blinkenjava      *test_module);
static void       blinkenjava_finalize   (GObject          *object);

static gboolean   blinkenjava_query      (gint              width,
                                          gint              height,
                                          gint              channels,
                                          gint              maxval);
static gboolean   blinkenjava_prepare    (BModule          *bmodule,
					  GError           **error);
static void       blinkenjava_start      (BModule          *bmodule);
static void       blinkenjava_stop       (BModule          *bmodule);
static void       blinkenjava_event      (BModule          *bmodule,
                                            BModuleEvent     *event);
static gint       blinkenjava_tick       (BModule          *bmodule);
static void
blinkenjava_describe (BModule      *module,
                         const gchar **title,
                         const gchar **description,
		      const gchar **author);
static void blinkenjava_relax(BModule *bmodule);

static gboolean create_blinkenlet (Blinkenjava* blinkenjava, JNIEnv* env);


static BModuleClass * parent_class       = NULL;
static BlinkenjavaClass * this_class       = NULL;
static GType          type_blinkenjava   = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  blinkenjava_get_type (module);

  return TRUE;
}

GType
blinkenjava_get_type (GTypeModule *module)
{
  if (! type_blinkenjava)
    {
      static const GTypeInfo module_info =
      {
        sizeof (BlinkenjavaClass),
        (GBaseInitFunc) blinkenjava_base_class_init,           /* base_init */
        (GBaseFinalizeFunc) blinkenjava_base_class_finalize,           /* base_finalize */
        (GClassInitFunc) blinkenjava_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (Blinkenjava),
        0,              /* n_preallocs */
        (GInstanceInitFunc) blinkenjava_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      type_blinkenjava = g_type_module_register_type (module,
                                                        B_TYPE_MODULE,
                                                        "Blinkenjava",
                                                        &module_info,
                                                        0);
    }

  return type_blinkenjava;
}

static void 
blinkenjava_base_class_init (BlinkenjavaClass *klass) {
  JavaVMInitArgs vm_args;

  JavaVMOption   options[2];
  JNIEnv         *env;
  GString        *cp;
  const char     *cp_env = getenv( "CLASSPATH" );
  GString        *jlp;

  /* create the java vm */

  /* add blinken.jar from std location to classpath. retrieve CLASSPATH and
   * put it before, so we can override it. 
   */
  cp = g_string_new( "-Djava.class.path=" );
  if( cp_env != NULL ) {
    g_string_append( cp, getenv("CLASSPATH") );
    g_string_append( cp, ":" );
  }
  g_string_append( cp, BLINKENJAR );
  fprintf(stdout, "will pass following option to vm:\n");
  fprintf(stdout, "     %s\n", cp->str );

  jlp = g_string_new( "-Djava.library.path=" BLIB_MODULEPATH );
  fprintf(stdout, "     %s\n", jlp->str );
 
  options[0].optionString = cp->str;
  options[1].optionString = jlp->str;

  memset(&vm_args, 0, sizeof(vm_args));
  vm_args.version = 0x00010002;
  vm_args.options = options;
  vm_args.nOptions = 2;

  /* actually create vm */
  if( JNI_CreateJavaVM(&(klass->jvm), (void**)&env, &vm_args) != 0 ) {
      klass->jvm = NULL;
      fprintf(stderr, "\n=======================\nCreation of JVM failed.\n");
  } else {


      fprintf(stdout, "successfully created java VM\n");
      printf("exception check = %d\n", (*env)->ExceptionOccurred(env) );
      printf("init 1\n");
      Init_de_blinkenlights_blinkenlet_Blinkenlet( env );

      printf("init 2\n");

      Init_de_blinkenlights_blinkenlet_BlinkenKeyEvent( env );
      
      printf("exception check = %d\n", (*env)->ExceptionOccurred(env) );
      printf("init done\n");
  }
  g_string_free(cp, TRUE);
  g_string_free(jlp, TRUE);
}

static void 
blinkenjava_base_class_finalize (BlinkenjavaClass *klass) {
  JavaVM *jvm = klass->jvm;

  if( jvm != NULL ) {
    fprintf(stdout, "Shtutting down JVM\n");
    (*jvm)->DestroyJavaVM( jvm );
    klass->jvm = NULL;
  } else {
    fprintf(stdout, "Shutdown: JVM appears not to be running. Doing nothing.\n");
  }
}

static void
blinkenjava_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  Blinkenjava *blinkenjava = BLINKENJAVA (object);

  switch (property_id)
    {
    case PROP_CLASS:
      if( blinkenjava->jclassname ) {
	g_string_free(blinkenjava->jclassname, TRUE);
      }
      g_print( "class: %s\n", g_value_get_string(value) );
      blinkenjava->jclassname = g_string_new( g_value_get_string(value) );
      g_print(__FUNCTION__ ": classname %s\n", blinkenjava->jclassname->str ); 
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
blinkenjava_class_init (BlinkenjavaClass *klass)
{
  GObjectClass   *object_class;
  BModuleClass   *module_class;
  GParamSpec     *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  this_class   = BLINKENJAVA_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = blinkenjava_set_property;
  object_class->finalize = blinkenjava_finalize;

  param_spec = g_param_spec_string ("class", NULL, NULL, NULL,
                                      G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_CLASS, param_spec);

  module_class->query    = blinkenjava_query;
  module_class->prepare  = blinkenjava_prepare;
  module_class->relax    = blinkenjava_relax;
  module_class->start    = blinkenjava_start;
  module_class->stop     = blinkenjava_stop;
  module_class->event    = blinkenjava_event;
  module_class->tick     = blinkenjava_tick;
  module_class->describe = blinkenjava_describe;
}

static
gboolean exception_occurred(JNIEnv* env, gboolean clear, const char *msg) {
  jthrowable ex = (*env)->ExceptionOccurred(env);   
  if( ex != NULL ) {
    if( msg != NULL )  {
      fprintf(stderr, "Exception occurred (%s)\n", msg);
    }
    else {
      fprintf(stderr, "Exception occurred\n");
    }
    (*env)->ExceptionDescribe(env);
    java_lang_Throwable_printStackTrace(env, ex);
    if( clear ) {
      (*env)->ExceptionClear(env);
    }
    return TRUE;
  }
  return FALSE;
}



static void
blinkenjava_init (Blinkenjava *blinkenjava)
{
    JNIEnv* env;
    JavaVM *vm = this_class->jvm;

    printf("blinkenjava_init\n");

//    if( (*vm)->AttachCurrentThread(vm, (void**)&env, 0) != 0 ) {
//        fprintf(stderr, "could not attach thread, shutting down vm\n");
//        DestroyJavaVM(this_class->jvm);
//        this_class->jvm = NULL;
//        return;
//    }

    blinkenjava->env = NULL; /* don't store yet: see blinkenlet_prepare */

    blinkenjava->the_blinkenlet = NULL;
    blinkenjava->title = NULL;
    blinkenjava->author = NULL;
    blinkenjava->description = NULL;
}

static gboolean 
create_blinkenlet (Blinkenjava* blinkenjava, JNIEnv* env) {
    jclass blinkenletClass;
    jobject blinkenlet;

    GString *gsc = blinkenjava->jclassname;
    char *p;
    for( p = gsc->str; *p != 0; p++) {
      if( *p == '.' ) {
	*p = '/';
      }
    }
    
    printf("Loading java class %s\n", gsc->str );
    
    blinkenletClass = (*env)->FindClass(env, gsc->str );

    if( exception_occurred(env, TRUE, "probably class not found") ) {
      return FALSE;
    }
    
    printf("Generating new Blinkenlet instance\n");
    blinkenlet = (*env)->NewObject( env, blinkenletClass, 
				    (*env)->GetMethodID(env, blinkenletClass, 
							"<init>", "()V"));
    if( exception_occurred(env, TRUE, "probably ctor missing/not public") ) {
      return FALSE;
    }
    
    blinkenlet = (*env)->NewGlobalRef(env, blinkenlet); 
    blinkenjava->the_blinkenlet = blinkenlet;
    return TRUE;
}


static gboolean
blinkenjava_prepare (BModule *bmodule, GError** error)
{
  jboolean retval;
  gboolean exceptionOccurred;

  Blinkenjava* blinkenjava = BLINKENJAVA (bmodule);
  BlinkenjavaClass* klass = BLINKENJAVA_GET_CLASS(blinkenjava);
  JavaVM* vm = klass->jvm;
  JNIEnv* env; /* don't use blinkenlet->env yet */
  if( (*vm)->AttachCurrentThread(vm, (void**)&env, 0) != 0 ) {
        fprintf(stderr, "could not attach thread, stopping preparation\n");
        return FALSE;
  }

  g_print ("blinkenjava_prepare w=%d, h=%d\n", bmodule->width, bmodule->height);

  if( ! create_blinkenlet(blinkenjava, env) ) {
    (*vm)->DetachCurrentThread(vm);
    return FALSE;
  }

  retval = de_blinkenlights_blinkenlet_Blinkenlet_prepareInternal(
      env, blinkenjava->the_blinkenlet, 
      bmodule->width, bmodule->height, bmodule->channels, bmodule->maxval );
  if (!retval) {
    g_set_error (error, 0, 0,
		 "Blinkenlet cannot handle the requested cofiguration.");
  }
  if( exception_occurred(env, TRUE, NULL) ) {
    g_set_error (error, 0, 0,
		 "Exception occurred. This Blinkenlet is buggy.");
    retval = FALSE;
  }
  (*vm)->DetachCurrentThread(vm);
  return retval;
}

/* Do the clean-ups corresponding to prepare.
 * No need to delegate to Blinkenlet: Java programmers do not need to free any
 * objects and if they use additional resources, they shall write a finalizer
 * as common in Java.
 */
static void
blinkenjava_relax (BModule *module)
{

  Blinkenjava *blinkenjava = BLINKENJAVA(module);
  JNIEnv* env = blinkenjava->env;
  BlinkenjavaClass* klass = BLINKENJAVA_GET_CLASS(blinkenjava);
  JavaVM* vm = klass->jvm;
  /* TODO: if relaxing is possibly done in different thread, attach new JNIEnv* here */

  if( blinkenjava->the_blinkenlet != NULL ) {
    (*env)->DeleteGlobalRef(env, blinkenjava->the_blinkenlet);
  }
  if( blinkenjava->title != NULL ) {
    g_string_free( blinkenjava->title, TRUE );
  }
  if( blinkenjava->author != NULL ) {
    g_string_free( blinkenjava->author, TRUE );
  }
  if( blinkenjava->description != NULL ) {
    g_string_free( blinkenjava->description, TRUE );
  }
  (*vm)->DetachCurrentThread(vm);
}

static void
blinkenjava_finalize (GObject *object)
{
  /* you MUST upchain here! */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
blinkenjava_query (  gint     width,
                     gint     height,
                     gint     channels,
                     gint     maxval)
{
  /* query is not delegated. blinkenlets decide in prepare if they want to run 
   * Background: query cannot be delegated because it is a class method in bmodule.
   *             We do not have a class property, and therefore there is no
   *             Java class to delegate to.
   */
  return TRUE;

}

/* evil but will work. at least in a well-defined, single-threaded
 * execution model 
 */
static Blinkenjava *the_blinkenjava_module;



static void
blinkenjava_start (BModule *bmodule)
{
  Blinkenjava *blinkenjava;
  JNIEnv* env;
  BlinkenjavaClass* klass = BLINKENJAVA_GET_CLASS(blinkenjava);
  JavaVM* vm = klass->jvm;

  /* we will store the env not before start, because intialization/preparation
   * might be done from different thread and JNIEnv's can not be used across
   * threads. but from here on we know all calls into the module will be done
   * from the same thread, so we save the env.
   */
  if( (*vm)->AttachCurrentThread(vm, (void**)&env, 0) != 0 ) {
        fprintf(stderr, "could not attach thread, requesting stop\n");
        b_module_request_stop(bmodule);
        return;
  }
  blinkenjava->env = env;

  g_print ("blinkenjava_start\n");

  blinkenjava = BLINKENJAVA (bmodule);

  the_blinkenjava_module = blinkenjava;


  de_blinkenlights_blinkenlet_Blinkenlet_startInternal(
      blinkenjava->env, blinkenjava->the_blinkenlet);

  if( exception_occurred(env, TRUE, "Exception occurred in start.") ) { 
		g_print("Will issue stop request due to exception");
		b_module_request_stop(bmodule);
  }
}

static void
blinkenjava_stop (BModule *bmodule)
{
  Blinkenjava *blinkenjava;
  JNIEnv* env;

  g_print ("blinkenjava_stop\n");

  blinkenjava = BLINKENJAVA (bmodule);
  env = blinkenjava->env;

  g_print("going to call Java: stopInternal");
  de_blinkenlights_blinkenlet_Blinkenlet_stopInternal(
      blinkenjava->env, blinkenjava->the_blinkenlet );
  exception_occurred(env, TRUE, "Exception occurred in stop.");
}

static void
blinkenjava_event (BModule      *bmodule,
                     BModuleEvent *event)
{
  Blinkenjava *blinkenjava;
  JNIEnv* env;
  jobject j_event = NULL;

  g_print ("blinkenjava_event\n");
  g_return_if_fail( bmodule != NULL && event != NULL );
  blinkenjava = BLINKENJAVA (bmodule);
  env = blinkenjava->env;

  /* Generate corresponding Java-Event. */
  switch( event->type ) {
    case B_EVENT_TYPE_KEY:
      j_event = de_blinkenlights_blinkenlet_BlinkenKeyEvent_new( blinkenjava->env , event->device_id, event->key);
      break;
    case B_EVENT_TYPE_PLAYER_ENTERED:
      j_event = de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new( blinkenjava->env , 0, event->device_id);
      g_message("player entered %d", event->device_id);
      break;
    case B_EVENT_TYPE_PLAYER_LEFT:
      j_event = de_blinkenlights_blinkenlet_BlinkenPlayerEvent_new( blinkenjava->env , 1, event->device_id);
      g_message("player left %d", event->device_id);
    default:
  }

  /* Delegate to the blinkenlet. */
  if( j_event != NULL ) {
    de_blinkenlights_blinkenlet_Blinkenlet_handleEventInternal( blinkenjava->env,
                  blinkenjava->the_blinkenlet,
                  j_event);
    exception_occurred(env, TRUE, "Exception occurred in handleEvent.");
  } 
  else {
    g_warning("Unknown event type, won't be delegated  to the Blinkenlet.");
  }
}

/* Call start_ticker to turn the ticker on.
   the value you return here is the time until this function is called
   the next time. if you return <= 0 here, the function will never again
   be called by the core.
*/
static gint
blinkenjava_tick (BModule *bmodule)
{
  Blinkenjava *blinkenjava;
  JNIEnv* env;
  gint r;

  blinkenjava = BLINKENJAVA (bmodule);
  env = blinkenjava->env;

  //g_print("Delegating to the blinkenlet...\n");

  r = de_blinkenlights_blinkenlet_Blinkenlet_tickInternal(
      blinkenjava->env, blinkenjava->the_blinkenlet );

  if( exception_occurred(env, TRUE, "Exception occurred in tick. will stop") ) {
		b_module_request_stop(bmodule);
	}
  return r;
}


GString *
g_string_from_jstring(JNIEnv* env, jstring jstr) {
    const gchar* raw;
    GString *gs;
    raw = (const gchar*) ((*env)->GetStringChars(env, jstr, NULL));
    if( exception_occurred(env, TRUE, "failed to retrieve string")) {
      raw = NULL;
    }
    return g_string_new( raw == NULL ? "(null)" : raw );
}



static void
blinkenjava_describe (BModule      *module,
                         const gchar **title,
                         const gchar **description,
                         const gchar **author)
{
  Blinkenjava *blinkenjava = BLINKENJAVA (module);
  jstring jstr;
  JNIEnv* env = blinkenjava->env;
  jobject the_blinkenlet = blinkenjava->the_blinkenlet;

  if( blinkenjava->title == NULL ) {
    jstr = de_blinkenlights_blinkenlet_Blinkenlet_getTitle(env, the_blinkenlet);
    blinkenjava->title = g_string_from_jstring(env, jstr);
  }
  if( blinkenjava->author == NULL ) {
    blinkenjava->author = g_string_from_jstring(env, 
      de_blinkenlights_blinkenlet_Blinkenlet_getAuthor(env, the_blinkenlet));
  }
  if( blinkenjava->description == NULL ) {
    jstr = de_blinkenlights_blinkenlet_Blinkenlet_getDescription(env, the_blinkenlet);
    blinkenjava->description = g_string_from_jstring(env, jstr);
  }
  *title = blinkenjava->title->str;
  *author = blinkenjava->author->str;
  *description = blinkenjava->description->str;
}

JNIEXPORT void JNICALL Java_de_blinkenlights_blinkenlet_BlinkenBuffer_paintNative
  (JNIEnv *env, jobject this, jintArray data) 
{
    BModule* bmodule = B_MODULE(the_blinkenjava_module);
    int w = bmodule->width;
    int h = bmodule->height;
    int n = w * h * bmodule->channels;
    jint* pixel_data;
    int i,j;
    pixel_data = (*env)->GetIntArrayElements(env, data, 0);
    
    for(i=0; i<n; i++) {
        bmodule->buffer[i] = pixel_data[i];
    }

    (*env)->ReleaseIntArrayElements(env, data, pixel_data, JNI_ABORT);
    b_module_paint(bmodule);
}

JNIEXPORT void JNICALL Java_de_blinkenlights_blinkenlet_Blinkenlet_startTicker
(JNIEnv *env, jobject this, jint when) 
{
  BModule* bmodule;

  g_print("start ticker called\n");
  bmodule = B_MODULE(the_blinkenjava_module);
  g_print("delegating to b_module_ticker_start\n");
  b_module_ticker_start(bmodule, (int )when+1);
  g_print("start ticker done\n");
}

JNIEXPORT void JNICALL Java_de_blinkenlights_blinkenlet_Blinkenlet_stopTicker
(JNIEnv *env, jobject this) 
{
  BModule* bmodule;

  g_print("stop ticker called\n");
  bmodule = B_MODULE(the_blinkenjava_module);
  g_print("delegating to b_module_ticker_stop\n");
  b_module_ticker_stop(bmodule);
  g_print("start ticker done\n");
}

JNIEXPORT void JNICALL Java_de_blinkenlights_blinkenlet_Blinkenlet_requestStop
(JNIEnv *env, jobject this) 
{
  BModule* bmodule;

  g_print("requestStop called\n");
  bmodule = B_MODULE(the_blinkenjava_module);
  g_print("Delegating to b_module\n");
  b_module_request_stop(bmodule);
  g_print("requestStop done\n");
}


