#ifndef __RKAPP_MAIN_H__
#define __RKAPP_MAIN_H__

#include <jni.h>


#define TRUE 1
#define FALSE 0

jboolean initializeWorker(JNIEnv *env, jobject *javaFront, JavaVM *mGlobalJavaVM);

void finalizeWorker(JNIEnv *env, jobject *javaFront);

jboolean openSync();

jboolean turnLedRed();

jboolean turnLedGreen();

void setPathToFile(JNIEnv *env, jstring path);

void makeGlobalRef(JNIEnv *env, jobject *obj);
void deleteGlobalRef(JNIEnv *env, jobject *obj);

#endif
