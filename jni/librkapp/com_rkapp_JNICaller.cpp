
#include <com_rkapp_JNICaller.h>
#include <rkapp_main.h>

JNIEXPORT jboolean  Java_com_rkapp_JNICaller_init  (JNIEnv *, jobject){
	return init();
}

JNIEXPORT jboolean Java_com_rkapp_JNICaller_keyEvent
  (JNIEnv *, jobject, jint, jint, jint, jobject){
	return 0;
}

JNIEXPORT void Java_com_rkapp_JNICaller_cleanup
  (JNIEnv *, jobject){
	cleanup();
}

JNIEXPORT jboolean Java_com_rkapp_JNICaller_openSync
  (JNIEnv *, jobject){
	return openSync();
}

JNIEXPORT jboolean Java_com_rkapp_JNICaller_turnLedRed
  (JNIEnv *, jobject){
	return turnLedRed();
}

JNIEXPORT jboolean Java_com_rkapp_JNICaller_turnLedGreen
  (JNIEnv *, jobject){
	return turnLedGreen();
}



