#include <com_rkapp_JNICaller.h>
#include <rkapp_worker.h>

JavaVM *mGlobalJavaVM;

jint JNI_OnLoad(JavaVM* vm, void *reserved) {
	mGlobalJavaVM = vm;
	JNIEnv *env;
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return -1;
	}


	return JNI_VERSION_1_4;
}


JNIEXPORT jboolean JNICALL Java_com_rkapp_JNICaller_initializeWorker(JNIEnv *env,
		jobject this) {
	return initializeWorker(env, this, mGlobalJavaVM);
}

JNIEXPORT void JNICALL Java_com_rkapp_JNICaller_finalizeWorker(JNIEnv *env,
		jobject this) {
	finalizeWorker(env, this);
}

JNIEXPORT jboolean JNICALL Java_com_rkapp_JNICaller_openSync(JNIEnv *env,
		jobject this) {
	return openSync(env);
}

JNIEXPORT jboolean JNICALL Java_com_rkapp_JNICaller_turnLedRed(JNIEnv *env,
		jobject this) {
	return turnLedRed();
}

JNIEXPORT jboolean JNICALL Java_com_rkapp_JNICaller_turnLedGreen(JNIEnv *env,
		jobject this) {
	return turnLedGreen();
}

