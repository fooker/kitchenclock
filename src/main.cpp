#include "Arduino.h"

#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/TomThumb.h>

#include <ESP8266WiFi.h>

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Time.h>
#include <Timezone.h>

#include "../settings.h"

#define PIN 4

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(24, 8, PIN,
	NEO_MATRIX_TOP +
	NEO_MATRIX_RIGHT +
	NEO_MATRIX_COLUMNS +
	NEO_MATRIX_PROGRESSIVE,
	NEO_GRB +
	NEO_KHZ800);

WiFiUDP udp;
NTPClient ntp(udp, "de.pool.ntp.org", 0, 60*1000);

void setup() {
	Serial.begin(115200);
	Serial.println();

	matrix.begin();
	matrix.setTextWrap(false);
	matrix.setBrightness(40);
	matrix.setFont(&TomThumb);

	WiFi.begin(WIFI_SSID, WIFI_PASS);

	ntp.begin(false);
}

static const char NUMS[] = "0123456789";

uint16_t calc_color(const uint8_t h, const uint8_t m) {
	uint16_t x = h * 60 + m;
	if (x > 12*60) {
		x = 12*60 - (x - 12*60);
	}
	
	return matrix.Color(
		      (x * 255 / (12*60)),
		      (x * 255 / (12*60)),
		255 - (x * 255 / (12*60))
	);
}

void loop() {
	matrix.clear();

	ntp.update();

	static TimeChangeRule dst = {"CEST", Last, Sun, Mar, 2, 120};
	static TimeChangeRule std = {"CET",  Last, Sun, Oct, 3,  60};
	static Timezone tz(dst, std);

	time_t local = tz.toLocal(ntp.getEpochTime());

	if (WiFi.status() != WL_CONNECTED) {
		matrix.setTextColor(matrix.Color(255, 128, 128));
		matrix.setCursor(1, 6);
		matrix.print("WiFi...");

	} else {
		if (ntp.getEpochTime() % 2 == 0) {
			matrix.setTextColor(matrix.Color(255, 255, 255));
			matrix.setCursor(11, 6);
			matrix.print(":");
		}

		const int8_t h = hour(local);
		const int8_t m = minute(local);
		const int8_t s = second(local);

		const uint16_t color = calc_color(h, m);

		matrix.setTextColor(color);
		matrix.setCursor(3, 6);
		matrix.print(NUMS[h/10]);
		matrix.setCursor(7, 6);
		matrix.print(NUMS[h%10]);

		matrix.setCursor(13, 6);
		matrix.print(NUMS[m/10]);
		matrix.setCursor(17, 6);
		matrix.print(NUMS[m%10]);

		if (m % 2 == 0) {
			if (s != 0) {
				matrix.writeFastHLine(0, 7, s*24/60, matrix.Color(100, 100, 130));
			}
		} else {
			if (s != 59) {
				matrix.writeFastHLine(s*24/60, 7, 24, matrix.Color(100, 100, 130));
			}
		}
	}

	matrix.show();
	delay(500);
}
