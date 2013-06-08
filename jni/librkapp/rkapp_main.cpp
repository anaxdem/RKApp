/*
 * by anaxdem
 * anaxdema@mail.ru
 *
 */


#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jni.h>
#include <time.h>



#include <pthread.h>
#include <math.h>

#include <assert.h>

#include <android/log.h>
#include <libusb.h>
#include <libfreenect.h>		//The application needs to include the driver header file

#include <freenect_internal.h>

#define LOG(a) __android_log_print(ANDROID_LOG_DEBUG, TAG, a)

//TODO Hotplug connection

pthread_t freenect_thread;
volatile int die = 0;
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

static int got_new_frame=0;

int new_frame(){
	pthread_mutex_lock(&gl_backbuf_mutex);
	return got_new_frame;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}
void getDepthData(uint8_t *rgb){
	//lock this and make copy of the memory
	pthread_mutex_lock(&gl_backbuf_mutex);
	if(got_new_frame){
		memcpy(rgb, depth_mid, 640*480*4*sizeof(uint8_t));
		got_new_frame=0;
	}
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}
void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
	int i;
	uint16_t *depth = (uint16_t*)v_depth;

	pthread_mutex_lock(&gl_backbuf_mutex);
	got_new_frame=1;
	unsigned char* depth_mid_ptr=depth_mid;
	for (i=0; i<640*480; i++) {
		int pval = t_gamma[depth[i]];
		int lb = pval & 0xff;
		switch (pval>>8) {
			case 0:
				//should be more efficient then the old way way. (optimized for mobile!)
				*depth_mid_ptr=255;
				*(depth_mid_ptr+1)=255-lb;
				*(depth_mid_ptr+2)=255-lb;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 255;
//				depth_mid[3*i+1] = 255-lb;
//				depth_mid[3*i+2] = 255-lb;
				break;
			case 1:
				*depth_mid_ptr=255;
				*(depth_mid_ptr+1)=lb;
				*(depth_mid_ptr+2)=0;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 255;
//				depth_mid[3*i+1] = lb;
//				depth_mid[3*i+2] = 0;
				break;
			case 2:
				*depth_mid_ptr=255-lb;
				*(depth_mid_ptr+1)=255;
				*(depth_mid_ptr+2)=0;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 255-lb;
//				depth_mid[3*i+1] = 255;
//				depth_mid[3*i+2] = 0;
				break;
			case 3:
				*depth_mid_ptr=0;
				*(depth_mid_ptr+1)=255;
				*(depth_mid_ptr+2)=lb;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 255;
//				depth_mid[3*i+2] = lb;
				break;
			case 4:
				*depth_mid_ptr=0;
				*(depth_mid_ptr+1)=255-lb;
				*(depth_mid_ptr+2)=255;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 255-lb;
//				depth_mid[3*i+2] = 255;
				break;
			case 5:
				*depth_mid_ptr=0;
				*(depth_mid_ptr+1)=0;
				*(depth_mid_ptr+2)=255-lb;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 0;
//				depth_mid[3*i+2] = 255-lb;
				break;
			default:
				*depth_mid_ptr=0;
				*(depth_mid_ptr+1)=0;
				*(depth_mid_ptr+2)=0;
				*(depth_mid_ptr+3)=255;
				depth_mid_ptr+=4;
//				depth_mid[3*i+0] = 0;
//				depth_mid[3*i+1] = 0;
//				depth_mid[3*i+2] = 0;
				break;
		}
	}
	//got_depth++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	pthread_mutex_lock(&gl_backbuf_mutex);

	// swap buffers
	assert (rgb_back == rgb);
	rgb_back = rgb_mid;
	freenect_set_video_buffer(dev, rgb_back);
	rgb_mid = (uint8_t*)rgb;

	//got_rgb++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void *freenect_threadfunc(void *arg)
{
	int accelCount = 0;

	freenect_set_tilt_degs(f_dev,freenect_angle);
	freenect_set_led(f_dev,current_led_option);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_callback(f_dev, rgb_cb);
	freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, current_format));
	freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(f_dev, rgb_back);

	freenect_start_depth(f_dev);
	freenect_start_video(f_dev);

	//printf("'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");
	//char buf[512];
	LOG("Threaded...\n");

	while (!die && freenect_process_events(f_ctx) >= 0) {
		//Throttle the text output
		if (accelCount++ >= 2000)
		{
			freenect_set_led(f_dev,current_led_option);
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(f_dev);
			state = freenect_get_tilt_state(f_dev);
			double dx,dy,dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
			printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
			fflush(stdout);
		}

		if (requested_format != current_format) {
			freenect_stop_video(f_dev);
			freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, requested_format));
			freenect_start_video(f_dev);
			current_format = requested_format;
		}
	}

	LOG("\nshutting down streams...\n");

	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);

	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	return NULL;
}

/**
 * Here we setting up permission for open Kinect connection
 * result is result of setting up
 */
int sysSetUP(){

	return chmod( "/dev/bus/usb/001/", 0777);
}

jboolean openSync()
{
		LOG("Start Sync");

		int res;
//iinitialize variables
		depth_mid = (uint8_t*)malloc(640*480*4);
		depth_front = (uint8_t*)malloc(640*480*3);
		rgb_back = (uint8_t*)malloc(640*480*3);
		rgb_mid = (uint8_t*)malloc(640*480*3);
		rgb_front = (uint8_t*)malloc(640*480*3);

		LOG("Kinect camera test\n");

		int i;
		for (i=0; i<2048; i++) {
			float v = i/2048.0;
			v = powf(v, 3)* 6;
			t_gamma[i] = v*6*256;
		}
//init  kinect context
		if (freenect_init(&f_ctx, NULL) < 0) {
			LOG("Kinect open sync failed\n");
			return false;
		}
//settup
		freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
		freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
//get num dev
		int nr_devices = freenect_num_devices (f_ctx);

		sprintf(my_log, "Number Devices found %d\n", nr_devices);
		LOG(my_log);

		int user_device_number = 0;

		if (nr_devices < 1)
			return false;

		if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
			LOG("Cannot Open device\n");
			return false;
		}
//create thread
		res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
		if (res) {
			LOG("pthread_create failed...\n");
			return false;
		}

	return true;
}



jboolean init()
{
	LOG( "init was ");
	return true;
}

void cleanup()
{
	free(f_dev);

	free(depth_mid);
	free(depth_front);
	free(rgb_back);
	free(rgb_mid);
	free(rgb_front);

	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	printf("-- done!\n");
    LOG("framework cleanup");
}

jboolean turnLedRed ()
{

	LOG( "turnLedRed");
	current_led_option = LED_RED;
	return true;
}

jboolean turnLedGreen ()
{

	current_led_option = LED_GREEN;
	return true;
}

