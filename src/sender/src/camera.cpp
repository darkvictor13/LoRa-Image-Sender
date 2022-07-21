#include "camera.hpp"

#include <Arduino.h>
#include "hardware.h"

Camera::Camera() : is_initialized(false) {
	setDefaultConfig();
}

void Camera::setDefaultConfig() {
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;

	config.frame_size = FRAMESIZE_HVGA;
	config.jpeg_quality = 50;  //0-63 lower number means higher quality
	config.fb_count = 1;
}

void Camera::init() {
	const auto err = esp_camera_init(&config);
	is_initialized = (err == ESP_OK);
	ESP_ERROR_CHECK(err);
	sensor_t *sensor = esp_camera_sensor_get();
	//sensor->set_whitebal(sensor, 0);
	sensor->set_special_effect(sensor, 2); //grayscale
}

camera_fb_t* Camera::takePicture() {
	if (!is_initialized) {
		Serial.println("Camera nao foi inicializada");
		delay(1000);
		ESP.restart();
	}
    auto p = esp_camera_fb_get();
	if (p == NULL) {
		Serial.printf("Picture is NULL\n");
		delay(1000);
		ESP.restart();
	}
	return p;
}

void Camera::freePicture(camera_fb_t* picture) {
	esp_camera_fb_return(picture);
}

Camera::~Camera() {
    
}
