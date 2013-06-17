#include <rkapp_worker.h>
//Kinect includes
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <time.h>

#include <pthread.h>
#include <math.h>

#include <assert.h>

#include <libusb.h>
#include <libfreenect.h>		//The application needs to include the driver header file
#include <freenect_internal.h>

#include <stdio.h>
#include <stdint.h>

//Worker variables
JavaVM *gJavaVM;
jobject mJavaFront;
// classes
jclass mClassCaller;
//methods
jmethodID MethodOnCleanupEnd;
jmethodID MethodOnMassage;
jmethodID MethodOnNewFrame;
jmethodID MethodOnMassageByte;

const char* pathToFile;

//Kinect part variables
pthread_t freenect_thread;
volatile int die = 0;
volatile int closed = 0;
freenect_context *f_ctx;
freenect_device *f_dev;

char my_log[512];

pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

int got_rgb = 0;
int got_depth = 0;

int freenect_angle = 0;
int freenect_led;

uint16_t t_gamma[2048];

uint8_t *depth_mid, *depth_front;
uint8_t *rgb_back, *rgb_mid, *rgb_front;

freenect_video_format requested_format = FREENECT_VIDEO_RGB;
freenect_video_format current_format = FREENECT_VIDEO_RGB;

freenect_led_options current_led_option = LED_RED;

static int got_new_frame = 0;

static JNIEnv* getJniEnv() {
	JavaVMAttachArgs attachArgs;
	attachArgs.version = JNI_VERSION_1_6;
	attachArgs.name = ">>>NativeThread__Any";
	attachArgs.group = NULL;

	JNIEnv* env;
	if ((*gJavaVM)->GetEnv(gJavaVM, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
				"Failed to get the environment using GetEnv()");
		return -1;
	}
	if ((*gJavaVM)->AttachCurrentThread(gJavaVM, &env, &attachArgs) != JNI_OK) {
		env = NULL;
	}

	return env;
}

void sendMsgToJava(JNIEnv *env, char* msg) {
	if (env == NULL) {
		JNIEnv *envc = getJniEnv();
		jstring str = (*envc)->NewStringUTF(envc, msg);
		(*envc)->CallVoidMethod(envc, mJavaFront, MethodOnMassage, str);
		(*envc)->DeleteLocalRef(envc, str);
	} else {
		jstring str = (*env)->NewStringUTF(env, msg);
		(*env)->CallVoidMethod(env, mJavaFront, MethodOnMassage, str);
		(*env)->DeleteLocalRef(env, str);
	}
}

void sendByteArrayToJava(JNIEnv *env, void* msg) {
	if (env == NULL) {
		JNIEnv *envc = getJniEnv();
		jcharArray bytearrayBuffer = (*envc)->NewCharArray(envc, 640 * 480); // construct a new byte array
		(*env)->SetCharArrayRegion(env, bytearrayBuffer, 0, 640 * 480,
				(jchar*) msg); // copies the region of length nBytes starting from 0 from the char array to the new byte array

		(*envc)->CallVoidMethod(envc, mJavaFront, MethodOnMassageByte,
				bytearrayBuffer);
		(*envc)->DeleteLocalRef(envc, bytearrayBuffer);
	} else {
		jcharArray bytearrayBuffer = (*env)->NewCharArray(env, 640 * 480); // construct a new byte array
		(*env)->SetCharArrayRegion(env, bytearrayBuffer, 0, 640 * 480,
				(jchar*) msg); // copies the region of length nBytes starting from 0 from the char array to the new byte array

		(*env)->CallVoidMethod(env, mJavaFront, MethodOnMassageByte,
				bytearrayBuffer);
		(*env)->DeleteLocalRef(env, bytearrayBuffer);
	}
}

int new_frame() {
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "new_frame ");
	pthread_mutex_lock(&gl_backbuf_mutex);
	return got_new_frame;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}
void getDepthData(uint8_t *rgb) {
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "getDepthData ");
	//lock this and make copy of the memory
	pthread_mutex_lock(&gl_backbuf_mutex);
	if (got_new_frame) {
		memcpy(rgb, depth_mid, 640 * 480 * 4 * sizeof(uint8_t));
		got_new_frame = 0;
	}
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

char* writeToFile(const char* path, uint8_t* buffer) {

	FILE *p = NULL;
	char buf[64];
	size_t len = 0;

	p = fopen(path, "a+");
	if (p == NULL) {
		return "Error writing file";
	}
	int i = 0;
	len = strlen((char*) buffer);
	fprintf(p, "\n!--- new frame: \n");
	for (i = 0; i < 500 && i < len; i++) {

		fprintf(p, "  %d", buffer[i]);

	}

	//fwrite(buffer, sizeof(uint8_t), len, p);
	fclose(p);
	return "File written successfully!";

}

void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp) {
	int i;

	uint16_t *depth = (uint8_t*) v_depth;

	pthread_mutex_lock(&gl_backbuf_mutex);
	got_new_frame = 1;
	unsigned char* depth_mid_ptr = depth_mid;
	for (i = 0; i < 640 * 480; i++) {
		int pval = t_gamma[depth[i]];
		int lb = pval & 0xff;
		switch (pval >> 8) {
		case 0:
			//should be more efficient then the old way way. (optimized for mobile!)
			*depth_mid_ptr = 255;
			*(depth_mid_ptr + 1) = 255 - lb;
			*(depth_mid_ptr + 2) = 255 - lb;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 255;
//				depth_mid[3*i+1] = 255-lb;
//				depth_mid[3*i+2] = 255-lb;
			break;
		case 1:
			*depth_mid_ptr = 255;
			*(depth_mid_ptr + 1) = lb;
			*(depth_mid_ptr + 2) = 0;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 255;
//				depth_mid[3*i+1] = lb;
//				depth_mid[3*i+2] = 0;
			break;
		case 2:
			*depth_mid_ptr = 255 - lb;
			*(depth_mid_ptr + 1) = 255;
			*(depth_mid_ptr + 2) = 0;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 255-lb;
//				depth_mid[3*i+1] = 255;
//				depth_mid[3*i+2] = 0;
			break;
		case 3:
			*depth_mid_ptr = 0;
			*(depth_mid_ptr + 1) = 255;
			*(depth_mid_ptr + 2) = lb;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 255;
//				depth_mid[3*i+2] = lb;
			break;
		case 4:
			*depth_mid_ptr = 0;
			*(depth_mid_ptr + 1) = 255 - lb;
			*(depth_mid_ptr + 2) = 255;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 255-lb;
//				depth_mid[3*i+2] = 255;
			break;
		case 5:
			*depth_mid_ptr = 0;
			*(depth_mid_ptr + 1) = 0;
			*(depth_mid_ptr + 2) = 255 - lb;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 0;
//				depth_mid[3*i+2] = 255-lb;
			break;
		default:
			*depth_mid_ptr = 0;
			*(depth_mid_ptr + 1) = 0;
			*(depth_mid_ptr + 2) = 0;
			*(depth_mid_ptr + 3) = 255;
			depth_mid_ptr += 4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 0;
//				depth_mid[3*i+2] = 0;
			break;
		}
	}
	//got_depth++;

	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);

//	sendByteArrayToJava(NULL, depth);
//	sendMsgToJava(NULL, writeToFile("/mnt/temp.txt", ""));
}

void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp) {
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "rgb cb ");

	//sendMsgToJava(NULL, "rgb callbeck was");

	sendMsgToJava(NULL, writeToFile(pathToFile, (uint8_t*) rgb));

	pthread_mutex_lock(&gl_backbuf_mutex);

	// swap buffers
	assert(rgb_back == rgb);
	rgb_back = rgb_mid;
	freenect_set_video_buffer(dev, rgb_back);
	rgb_mid = (uint8_t*) rgb;

	//got_rgb++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);

}

void *freenect_threadfunc(void *env) {
	int accelCount = 0;

	int status = (*gJavaVM)->AttachCurrentThread(gJavaVM, &env, NULL);

	freenect_set_tilt_degs(f_dev, freenect_angle);
	freenect_set_led(f_dev, current_led_option);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_callback(f_dev, rgb_cb);
	freenect_set_video_mode(
			f_dev,
			freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM,
					current_format));
	freenect_set_depth_mode(
			f_dev,
			freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM,
					FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(f_dev, rgb_back);

	int depthS = freenect_start_depth(f_dev);
	int videoS = freenect_start_video(f_dev);
	sleep(2);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
			"Depth start = %d Video start = %d ", depthS, videoS);

	//printf("'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");
	//char buf[512];
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Threaded...\n");
	sendMsgToJava(env, "Threaded...\n");

	int r = freenect_process_events(f_ctx);
	sprintf(my_log, "freenect_process_events(f_ctx)= %d  ", r);

	sendMsgToJava(env, my_log);
//	sendMsgToJava(env,freenect_process_events(f_ctx));
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
			"freenect_process_events(f_ctx)= %d   ",
			freenect_process_events(f_ctx));

	while (!die && freenect_process_events(f_ctx) >= 0) {
		//Throttle the text output
		if (accelCount++ >= 2000) {
			freenect_set_led(f_dev, current_led_option);
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(f_dev);
			state = freenect_get_tilt_state(f_dev);
			double dx, dy, dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
//			printf(
//					"\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f",
//					state->accelerometer_x, state->accelerometer_y,
//					state->accelerometer_z, dx, dy, dz);
//			fflush(stdout);
		}

		if (requested_format != current_format) {
			freenect_stop_video(f_dev);
			freenect_set_video_mode(
					f_dev,
					freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM,
							requested_format));
			freenect_start_video(f_dev);
			current_format = requested_format;
		}
	}

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
			"\nshutting down streams...\n");

	freenect_set_led(f_dev, LED_OFF);
	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);

	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	closed - 1;
	return NULL;
}

extern struct libusb_context *usbi_default_context;

jboolean openSync(JNIEnv* env) {
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Start Sync");
	sendMsgToJava(NULL, "Start Sync");

	int r = libusb_init(usbi_default_context);
	sprintf(my_log, "freenect_process_events(f_ctx)= %d  ", r);
	sendMsgToJava(env, my_log);

	int res;
//iinitialize variables
	depth_mid = (uint8_t*) malloc(640 * 480 * 4);
	depth_front = (uint8_t*) malloc(640 * 480 * 3);
	rgb_back = (uint8_t*) malloc(640 * 480 * 3);
	rgb_mid = (uint8_t*) malloc(640 * 480 * 3);
	rgb_front = (uint8_t*) malloc(640 * 480 * 3);

	int i;
	for (i = 0; i < 2048; i++) {
		float v = i / 2048.0;
		v = powf(v, 3) * 6;
		t_gamma[i] = v * 6 * 256;
	}
//init  kinect context
	if (freenect_init(&f_ctx, NULL) < 0) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
				"Kinect open sync failed\n");
		sendMsgToJava(env, "Kinect open sync failed");
		return FALSE;
	}
//settup
	freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
	freenect_select_subdevices(
			f_ctx,
			(freenect_device_flags) (FREENECT_DEVICE_MOTOR
					| FREENECT_DEVICE_CAMERA));
//get num dev
	int nr_devices = freenect_num_devices(f_ctx);

	sprintf(my_log, "Number Devices found %d\n", nr_devices);
	sendMsgToJava(env, my_log);
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, my_log);

	int user_device_number = 0;

	if (nr_devices < 1)
		return FALSE;

	if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Cannot Open device\n");
		sendMsgToJava(env, "Cannot Open device");
		return FALSE;
	}
//create thread
	res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, env);
	if (res) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
				"pthread_create failed...\n");
		sendMsgToJava(env, "pthread_create failed...");
		return FALSE;
	}

	return TRUE;
}

void makeGlobalRef(JNIEnv *env, jobject *obj) {
	if (*obj != NULL) {
		jobject globalRef = (*env)->NewGlobalRef(env, *obj);
		(*env)->DeleteLocalRef(env, *obj);
		*obj = globalRef;
	}

}

void deleteGlobalRef(JNIEnv *env, jobject *obj) {
	if (*obj != NULL) {
		(*env)->DeleteGlobalRef(env, obj);
		*obj = NULL;
	}
}

jboolean initializeWorker(JNIEnv *env, jobject *javaFront, JavaVM* gJava) {
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "init was ");

	gJavaVM = gJava;
	//cache classes
	mClassCaller = (*env)->FindClass(env, "com/rkapp/JNICaller");
	makeGlobalRef(env, &mClassCaller);
	if (mClassCaller == NULL) {
		goto ERROR;
	}

	//cache java methods
	MethodOnCleanupEnd = (*env)->GetMethodID(env, mClassCaller, "onFinalizeEnd",
			"(Z)V");
	if (MethodOnCleanupEnd == NULL) {
		goto ERROR;
	}

	MethodOnMassage = (*env)->GetMethodID(env, mClassCaller, "onMassage",
			"(Ljava/lang/String;)V");
	if (MethodOnCleanupEnd == NULL) {
		goto ERROR;
	}

	MethodOnMassageByte = (*env)->GetMethodID(env, mClassCaller,
			"onMassageByte", "([C)V");
	if (MethodOnCleanupEnd == NULL) {
		goto ERROR;
	}

	MethodOnNewFrame = (*env)->GetMethodID(env, mClassCaller, "onNewFrame",
			"()V");
	if (MethodOnCleanupEnd == NULL) {
		goto ERROR;
	}

	mJavaFront = (*env)->NewGlobalRef(env, javaFront);
	if (mJavaFront == NULL) {
		goto ERROR;
	}
	sendMsgToJava(env, "init was");
	return TRUE;

	ERROR: __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
			"start watcher error.");
	sendMsgToJava(env, "start watcher error.");
	return FALSE;
}

void finalizeWorker(JNIEnv *env, jobject *javaFront) {

	die = 1;
//	deleteGlobalRef(env, &mJavaFront);
//	deleteGlobalRef(env, &mClassCaller);
	//freenect_shutdown(&f_ctx);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "framework cleanup");
}

jboolean turnLedRed() {

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "turnLedRed");
	current_led_option = LED_RED;
	return TRUE;
}

jboolean turnLedGreen() {

	current_led_option = LED_GREEN;
	return TRUE;
}

void setPathToFile(JNIEnv* env, jstring cpath) {
	pathToFile = (*env)->GetStringUTFChars(env, cpath, NULL);
	sprintf(my_log, "JNI get file path =  %c  ", pathToFile);
	sendMsgToJava(env, my_log);
}
