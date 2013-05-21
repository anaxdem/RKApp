/*
 * by anaxdem
 * anaxdema@mail.ru
 *
 */

#include <unistd.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <jni.h>
#include <android/log.h>
#include <libusb.h>


#define MODULE "RkApp"
#define LOG(a) __android_log_print(ANDROID_LOG_DEBUG, MODULE, a)

//TODO Hotplug connection

jboolean init(JNIEnv*  env, jobject  thiz)
{

	return true;
}

void cleanup(JNIEnv*  env)
{
    __android_log_print(ANDROID_LOG_DEBUG, MODULE, "framework cleanup");


}

jboolean keyEvent(JNIEnv* env, jobject thiz, jint action, jint unicode)
{
    static jclass KeyCode_class = env->FindClass("android/view/KeyEvent");
    static jfieldID ACTION_UP_id = env->GetStaticFieldID(KeyCode_class, "ACTION_UP", "I");
    static int ACTION_UP = env->GetStaticIntField(KeyCode_class, ACTION_UP_id);

    if (action != ACTION_UP)
        return JNI_TRUE;

   // LOG( "action: %d, unicode: %c", action, unicode);

    switch (unicode)
    {
        case 'p':
        	LOG( "KEYCODE_P");
            //pause_demo = !pause_demo;
            break;
    }

    return JNI_TRUE;
}




jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    LOG("JNI_OnLoad called");
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOG("Failed to get the environment using GetEnv()");
		return -1;
	}


    JNINativeMethod methods[] =
       {
           {
               "init",
               "()Z",
               (void *) init
           },
           {
               "keyEvent",
               "(IILandroid/view/KeyEvent;)Z",
               (void *) keyEvent
           },
           {
               "cleanup",
               "()V",
               (void *) cleanup
           }
       };
    jclass cl;
        cl = (env)->FindClass ("com/rpapp/RKAppActivity");
        (env)->RegisterNatives(cl, methods, 3);

    return JNI_VERSION_1_4;
}
