#include <zephyr/kernel.h>
#include <zephyr/drivers/hwinfo.h>

#include <app_version.h>
#include <string.h>

#include "dect_phy/dect_phy.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

int main()
{
	int err;
	size_t tx_len;
	uint8_t tx_data[32] = {0};

    LOG_INF("Board: %s, app version: %s", CONFIG_BOARD, APP_VERSION_STRING);

	uint16_t dev_id;
	hwinfo_get_device_id((void *)&dev_id, sizeof(dev_id));

	LOG_INF("Device ID: %u (0x%04x)", dev_id, dev_id);

	err = dect_phy_init(dev_id);
	if (err) {
		LOG_ERR("dect_phy_init failed, err %d", err);
		return err;
	}

	while (1) {
		LOG_INF("Transmitting...");
		size_t tx_len = sprintf(tx_data, "Hello my friend!");
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

	LOG_INF("Exiting...");

	return 0;
}