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

const uint8_t Clock4x6Bitmaps[] PROGMEM = {
	0x79, 0x99, 0x9e, // 0
	0x26, 0xa2, 0x2f, // 1
	0x69, 0x16, 0x8f, // 2
	0xe1, 0x61, 0x1e, // 3
	0x99, 0xf1, 0x11, // 4
	0xf8, 0xe1, 0x1e, // 5
	0x78, 0xe9, 0x96, // 6
	0xf1, 0x24, 0x44, // 7
	0x79, 0xf9, 0x9e, // 8
	0x79, 0xf1, 0x1e, // 9
};

const GFXglyph Clock4x6Glypths[] PROGMEM = {
	// Idx, W, H, xAdv, dx, dy
	{  0, 4, 6, 5, 0, 0 }, // 0
	{  3, 4, 6, 5, 0, 0 }, // 1
	{  6, 4, 6, 5, 0, 0 }, // 2
	{  9, 4, 6, 5, 0, 0 }, // 3
	{ 12, 4, 6, 5, 0, 0 }, // 4
	{ 15, 4, 6, 5, 0, 0 }, // 5
	{ 18, 4, 6, 5, 0, 0 }, // 6
	{ 21, 4, 6, 5, 0, 0 }, // 7
	{ 24, 4, 6, 5, 0, 0 }, // 8
	{ 27, 4, 6, 5, 0, 0 }, // 9
};

const GFXfont Clock4x6 PROGMEM = {
	(uint8_t*)  Clock4x6Bitmaps,
	(GFXglyph*) Clock4x6Glypths,
	0x30, 0x39, 7
};

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
	matrix.setBrightness(127);

	WiFi.begin(WIFI_SSID, WIFI_PASS);

	ntp.begin(false);
}

static const char NUMS[] = "0123456789";

uint16_t calc_hue(const uint8_t h, const uint8_t m, const uint8_t s) {
	uint32_t x = (h * 60 + m) * 60 + s;
	return x * (255*255) / (24*60*60);
}

struct {
	uint8_t x;
	uint8_t y;
	uint8_t v;
} drops[32];

void loop() {
	matrix.clear();

	ntp.update();

	static TimeChangeRule dst = {"CEST", Last, Sun, Mar, 2, 120};
	static TimeChangeRule std = {"CET",  Last, Sun, Oct, 3,  60};
	static Timezone tz(dst, std);

	time_t local = tz.toLocal(ntp.getEpochTime());

	if (WiFi.status() != WL_CONNECTED) {
		matrix.setFont(&TomThumb);
		matrix.setTextColor(matrix.Color(255, 128, 128));
		matrix.setCursor(1, 6);
		matrix.print("WiFi...");

	} else {
		const int8_t h = hour(local);
		const int8_t m = minute(local);
		const int8_t s = second(local);

		for (uint8_t i = 0; i < 32; i++) {
			if (drops[i].v > 0) {
				drops[i].v -= 1;
			} else {
				drops[i].x = random(0, 24);
				drops[i].y = random(0, 8);
				drops[i].v = random(16, 64);
			}
		
			const uint32_t color = matrix.ColorHSV(calc_hue(h, m, s), 255, drops[i].v);	
			matrix.drawPixel(drops[i].x, drops[i].y, matrix.Color(color >> 16, color >> 8, color >> 0));
		}

		matrix.setFont(&Clock4x6);

		if (ntp.getEpochTime() % 2 == 0) {
			const uint16_t color = matrix.Color(0x7f, 0x7f, 0x7f);
			matrix.drawPixel(11, 1, color);
			matrix.drawPixel(11, 2, color);
			matrix.drawPixel(12, 1, color);
			matrix.drawPixel(12, 2, color);
			
			matrix.drawPixel(11, 5, color);
			matrix.drawPixel(11, 6, color);
			matrix.drawPixel(12, 5, color);
			matrix.drawPixel(12, 6, color);
		}

		matrix.setTextColor(matrix.Color(0xFF, 0xFF, 0xFF));

		matrix.setCursor(1, 1);
		matrix.print(NUMS[h/10]);
		matrix.setCursor(6, 1);
		matrix.print(NUMS[h%10]);

		matrix.setCursor(14, 1);
		matrix.print(NUMS[m/10]);
		matrix.setCursor(19, 1);
		matrix.print(NUMS[m%10]);
	}

	matrix.show();
	delay(50);
}

