#include "Device/Device.h"
#include "Relay/Relay.h"
#include "ESP32WebServer/ESP32WebServer.h"
#include "Logger/Logger.h"
#include "Logger/Loggers/API.hpp"
#include "Logger/Loggers/Serial.hpp"
#include "Notification/Notification.h"

#include <Arduino.h>
#include <SPIFFS.h>
#include <CronAlarms.h>

#include <esp_task_wdt.h>

const char VERSION[8] = "v1.2.1";

Logger logger = Logger("main");
Relay relay = Relay("Switch", 33);

#define WDT_TIMEOUT 20

void initTasks();
void printSetup();
void handleLightRelay();

int8_t turnOnOff(float current_value, float threshold_value);
float getValue();

void GmailNotification(
	const char *title,
	const char *message);

void setup()
{
	Serial.begin(115200);

	esp_task_wdt_init(WDT_TIMEOUT, true);
	esp_task_wdt_add(NULL);

	Device::setup();

	Logger::addStream(Loggers::logToSerial);
	Logger::addStream(Loggers::logToAPI);

	Notification::addStream(GmailNotification);
	ESP32WebServer::start();

	initTasks();
	printSetup();

	char msg[64];
	snprintf(msg, 63, "Device started \nFirmware version: %s", VERSION);
	Notification::push("StarLights", msg);
}

void loop()
{
	Cron.delay();
	esp_task_wdt_reset();
}

void initTasks()
{
	logger.log("Initalizing tasks...");
	Cron.create(
		"0 */1 * * * *",
		[]()
		{ handleLightRelay(); },
		false);

	Cron.create(
		"0 0 4 * * *",
		Device::setupTime,
		false);

	Cron.create(
		"0 */2 * * * *",
		WiFiManager::manageConnection,
		false);

	Cron.create(
		"*/30 * * * * *",
		Device::sendHeartbeat,
		false);

}

void handleLightRelay()
{
	float value = getValue();
	float threshold = 30.0;
	int8_t turn_on_off = turnOnOff(threshold, value);

	if (turn_on_off == 1)
	{
		Notification::push("StarLights - Lights on", "");
		relay.turnOn();
	}
	else if (turn_on_off == 0)
	{
		Notification::push("StarLights - Lights off", "");
		relay.turnOff();
	}
}

int8_t turnOnOff(float threshold, float current_value)
{
	static int8_t on_off = -1; // Not changed state

	int8_t triggered = int8_t(current_value < threshold);
	int8_t state_changed = on_off != triggered;

	int8_t result = -1;
	if (state_changed)
	{
		result = bool(triggered);
		on_off = result;
	}
	return result;
}

float getValue()
{
	logger.log("Getting solar current reading...");

	StaticJsonDocument<128> doc;
	JsonObject payload = doc.to<JsonObject>();

	payload["window"] = 120;
	String response_raw = Device::device->getData(payload, "solar_panel_value", "v2.0");

	StaticJsonDocument<256> response;
	deserializeJson(response, response_raw);

	float result = response["value"];

	logger.logf("Current reading: %f", result);
	return result;
}

void GmailNotification(const char *title, const char *message)
{
	Device::device->postNotification(title, message);
}

void printSetup()
{
	char msg[64] = "";
	sprintf(msg, "Loaded firmware %s", VERSION);
	logger.log(msg);

	memset(msg, 0, 64);
	String _ip = WiFi.localIP().toString();

	char ip[16];
	_ip.toCharArray(ip, 16);
	sprintf(msg, "Device IP %s", ip);
	logger.log(msg);
}