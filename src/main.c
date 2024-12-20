#include <zephyr/kernel.h>
#include <zephyr/drivers/mfd/npm1300.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>

#include <app_version.h>
#include <string.h>

#include "dect_phy/dect_phy.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300));
static const struct device *leds = DEVICE_DT_GET(DT_NODELABEL(npm1300_leds));
static const struct device *regulators = DEVICE_DT_GET(DT_NODELABEL(npm1300_regulators));
static const struct device *charger = DEVICE_DT_GET(DT_NODELABEL(npm1300_charger));

#define FAST_FLASH_MS	100
#define SLOW_FLASH_MS	500
#define PRESS_SHORT_MS	1000
#define PRESS_MEDIUM_MS 5000

static volatile int flash_time_ms = SLOW_FLASH_MS;
static volatile bool vbus_connected;

static void event_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	static int press_t;

	if (pins & BIT(NPM1300_EVENT_SHIPHOLD_PRESS)) {
		press_t = k_uptime_get();
	}

	if (pins & BIT(NPM1300_EVENT_SHIPHOLD_RELEASE)) {
		press_t = k_uptime_get() - press_t;

		if (press_t < PRESS_SHORT_MS) {
			LOG_INF("Short press");
			flash_time_ms = FAST_FLASH_MS;
			led_blink(leds, 2U, flash_time_ms, flash_time_ms);
		} else if (press_t < PRESS_MEDIUM_MS) {
			LOG_INF("Medium press");
			flash_time_ms = SLOW_FLASH_MS;
			led_blink(leds, 2U, flash_time_ms, flash_time_ms);
		} else {
			LOG_INF("Long press");
			if (vbus_connected) {
				LOG_INF("Ship mode entry not possible with USB connected");
			} else {
				regulator_parent_ship_mode(regulators);
			}
		}
	}

	if (pins & BIT(NPM1300_EVENT_VBUS_DETECTED)) {
		LOG_INF("Vbus connected");
		vbus_connected = true;
	}

	if (pins & BIT(NPM1300_EVENT_VBUS_REMOVED)) {
		LOG_INF("Vbus removed");
		vbus_connected = false;
	}
}

bool configure_events(void)
{
	if (!device_is_ready(pmic)) {
		LOG_INF("Pmic device not ready.");
		return false;
	}

	if (!device_is_ready(regulators)) {
		LOG_INF("Regulator device not ready.");
		return false;
	}

	if (!device_is_ready(charger)) {
		LOG_INF("Charger device not ready.");
		return false;
	}

	static struct gpio_callback event_cb;

	gpio_init_callback(&event_cb, event_callback,
			   BIT(NPM1300_EVENT_SHIPHOLD_PRESS) | BIT(NPM1300_EVENT_SHIPHOLD_RELEASE) |
				   BIT(NPM1300_EVENT_VBUS_DETECTED) |
				   BIT(NPM1300_EVENT_VBUS_REMOVED));

	mfd_npm1300_add_callback(pmic, &event_cb);

	/* Initialise vbus detection status */
	struct sensor_value val;
	int ret = sensor_attr_get(charger, SENSOR_CHAN_CURRENT, SENSOR_ATTR_UPPER_THRESH, &val);

	if (ret < 0) {
		return false;
	}

	vbus_connected = (val.val1 != 0) || (val.val2 != 0);

	return true;
}

int main()
{
	int err;
	size_t tx_len;
	uint8_t tx_data[32] = {0};

    LOG_INF("Board: %s, app version: %s", CONFIG_BOARD, APP_VERSION_STRING);

	uint16_t dev_id;
	hwinfo_get_device_id((void *)&dev_id, sizeof(dev_id));

	LOG_INF("Device ID: %u (0x%04x)", dev_id, dev_id);

	if (!device_is_ready(leds)) {
		LOG_ERR("LED device is not ready");
		return -1;
	}

	if (!configure_events()) {
		LOG_ERR("Error: could not configure events");
		return -1;
	}

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO is not ready");
		return -1;
	}

	err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (err) {
		LOG_ERR("gpio_pin_configure_dt failed, err %d", err);
		return err;
	}

	err = dect_phy_init(dev_id);
	if (err) {
		LOG_ERR("dect_phy_init failed, err %d", err);
		return err;
	}

	gpio_pin_set_dt(&led, 1);
	led_blink(leds, 2U, flash_time_ms, flash_time_ms);

	while (1) {
		LOG_DBG("Transmitting...");
		size_t tx_len = sprintf(tx_data, "Hello from %u (0x%04x)", dev_id, dev_id);
		err = dect_phy_transmit(0, tx_data, tx_len);
		if (err) {
			LOG_ERR("dect_phy_transmit failed, err %d", err);
			return err;
		}

		err = dect_phy_receive(1);
		if (err) {
			LOG_ERR("dect_phy_receive failed, err %d", err);
			return err;
		}
	}

	err = dect_phy_deinit();
	if (err) {
		LOG_ERR("dect_phy_deinit failed, err %d", err);
		return err;
	}

	LOG_DBG("Exiting...");

	return 0;
}