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

JNIEXPORT jboolean JNICALL Java_com_rkapp_JNICaller_initializeWorker(
		JNIEnv *env, jobject this) {
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

JNIEXPORT void JNICALL Java_com_rkapp_JNICaller_setFilePath(JNIEnv *env,
		jobject this, jstring path) {

	setPathToFile(env, path);
}

JNIEXPORT void JNICALL Java_com_rkapp_JNICaller_titlDown(JNIEnv* env, jobject this) {
	titlDown();
}

JNIEXPORT void JNICALL Java_com_rkapp_JNICaller_tiltUP(JNIEnv * env, jobject this) {
	tiltUP();
}

JNIEXPORT void JNICALL Java_com_rkapp_JNICaller_setSizeToWrite(JNIEnv* env,
		jobject this, jint size) {
	setSizeToWrite(size);
}

JNIEXPORT void JNICALL Java_com_rkapp_JNICaller_setFileWritable(JNIEnv* env,
		jobject this, jint writable) {
	setFileWritable(writable);
}

