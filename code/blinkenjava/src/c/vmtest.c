#include <jni.h>

JavaVM *jvm;
JavaVMInitArgs vm_args;
JNIEnv*        *env;

int main() {
  memset(&vm_args, 0, sizeof(vm_args));
  vm_args.version = 0x00010002;
  if( JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args) != 0 ) {
      jvm = NULL;
      fprintf(stderr, "creation of JVM failed.\n");
      return 1;
  } else {
      fprintf(stdout, "successfully created java VM\n");
      return 0;
  }
}
