#pragma once

#include "esp_camera.h"

class Camera {
private:
	bool is_initialized;
	camera_config_t config;
	void setDefaultConfig();
public:
	Camera();

	void init();
	camera_fb_t* takePicture();

	~Camera();
};