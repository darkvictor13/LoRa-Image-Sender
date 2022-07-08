#include "scoped_timer.hpp"

#include <Arduino.h>

using namespace ws;

ScopedTimer::ScopedTimer(const char *name) : scope_name(name) {
	start = esp_timer_get_time();
}

double ScopedTimer::getExecutionTimeInSeconds() const {
	return getExecutionTimeInUs() / 1000000.0;
}

int64_t ScopedTimer::getExecutionTimeInUs() const {
	return esp_timer_get_time() - start;
}

ScopedTimer::~ScopedTimer() {
	auto sec __attribute__((unused)) = getExecutionTimeInSeconds();
	Serial.print(0);
	Serial.print(0);
	Serial.print(0);
	Serial.printf("Tempo de execução de [%s] = %lf segundos", scope_name, sec);
	Serial.flush();
}