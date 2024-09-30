/**
 * @file dect_phy.h
 * @brief Header file for the DECT NR+ PHY implementation.
 * 
 * 
 * @copyright Copyright (c) 2024
 * @author Samy Francelet
 */

#ifndef DECT_PHY_H
#define DECT_PHY_H

int dect_phy_init(const uint16_t device_id);

int dect_phy_transmit(uint32_t handle, void *data, size_t data_len):
int dect_phy_receive(uint32_t handle);

#endif // DECT_PHY_H