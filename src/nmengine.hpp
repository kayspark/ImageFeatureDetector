#pragma once
/*
 * Copyright (c) 2017-2019, nepes corp, All rights reserved.
 * Version 1.0.2T : for testing purpose.
 */

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif
constexpr auto NM_NCR = 0x00;
constexpr auto NM_COMP = 0x01;
constexpr auto NM_LCOMP = 0x02;
constexpr auto NM_DIST = 0x03;
constexpr auto NM_INDEXCOMP = 0x03;
constexpr auto NM_CAT = 0x04;
constexpr auto NM_AIF = 0x05;
constexpr auto NM_MINIF = 0x06;
constexpr auto NM_MAXIF = 0x07;
constexpr auto NM_TESTCOMP = 0x08;
constexpr auto NM_TESTCAT = 0x09;
constexpr auto NM_NID = 0x0A;
constexpr auto NM_GCR = 0x0B;
constexpr auto NM_RSTCHAIN = 0x0C;
constexpr auto NM_NSR = 0x0D;
constexpr auto NM_POWERSAVE = 0x0E;
constexpr auto NM_NCOUNT = 0x0F;
constexpr auto NM_FORGET = 0x0F;

constexpr auto NEURON_MEMORY = 256;
constexpr auto CLASSIFY_MAX_K = 9;

constexpr auto CAM_FRAME_BUF = 1843200;

using nm_return_status = enum _nm_return_status {
  NM_SUCCESS = 0,
  NM_ERROR_UNKNOWN = 100,
  NM_ERROR_DEVICE_NOT_FOUND,
  NM_ERROR_DEVICE_INFO_FETCH_FAILED,
  NM_ERROR_DEVICE_OPEN_FAILED,
  NM_ERROR_INIT_FAILED,
  NM_ERROR_INVALID_PARAMETER,
  NM_ERROR_NOT_SUPPORT,
  NM_ERROR_IO_TIMEOUT,
  NM_ERROR_IO_FAILED
} ;

using nm_network_result = enum _nm_network_result {
  NM_LEARN_ALREADY_KNOWN = 0,
  NM_LEARN_SUCCESS = 1,
  NM_LEARN_ADJUSTED = 2,
  NM_LEARN_DEGENERATED = 3,
  NM_CLASSIFY_UNKNOWN = 0,
  NM_CLASSIFY_UNCERTAIN = 4,
  NM_CLASSIFY_IDENTIFIED = 8
} ;

using nm_network_type = enum _nm_network_type { RBF = 0, KNN = 1 } ;

using nm_norm_type = enum _nm_norm_type { L1 = 0, Lsup = 1 } ;

using nm_device = struct _nm_device {
  void *dev;
  void *handle;
  void *lock;
  uint16_t type;
  uint16_t id;
  uint16_t vid;
  uint16_t pid;
  uint16_t version;
  uint8_t is_open;
} ;

using nm_network_info = struct _nm_network_info {
  uint32_t neuron_size;
  uint16_t neuron_memory_size;
  uint16_t version;
} ;

using nm_context = struct _nm_context {
  uint16_t context;
  uint16_t norm;
  uint16_t minif;
  uint16_t maxif;
} ;

using nm_network_status = struct _nm_network_status {
  uint32_t network_used;
  uint16_t network_type;
  uint16_t context;
  uint16_t norm;
} ;

using nm_neuron = struct _nm_neuron {
  uint32_t nid;
  uint16_t size;
  uint16_t ncr;
  uint16_t aif;
  uint16_t minif;
  uint16_t cat;
  uint8_t model[256];
} ;

using nm_classify_req = struct _nm_classify_req {
  nm_network_result status;
  uint16_t size;
  uint16_t k;
  uint16_t matched_count;
  uint32_t nid[CLASSIFY_MAX_K];
  uint16_t degenerated[CLASSIFY_MAX_K];
  uint16_t distance[CLASSIFY_MAX_K];
  uint16_t category[CLASSIFY_MAX_K];
  uint8_t vector[256];
} ;

using nm_learn_req = struct _nm_learn_req {
  nm_network_result ns;
  nm_neuron affected_neurons[10];
  uint16_t affected_count;
  uint16_t category;
  uint16_t size;
  uint8_t vector[256];
  uint8_t query_affected;
} ;

/**
 * Returns a list of the NM500 devices.
 */
extern uint16_t nm_get_devices(nm_device *devices, uint16_t count);

/**
 * Opens selected device and performs the initialization of the NM500.
 */
extern uint16_t nm_connect(nm_device *device);

/**
 * Closes selected device
 */
extern uint16_t nm_close(nm_device *device);

/**
 * Clears all knowledge on the NeuroMem network
 */
extern uint16_t nm_forget(nm_device *device);

/**
 * Clears all knowledge on the NeuroMem network
 */
extern uint16_t nm_reset(nm_device *device);

/**
 * Returns the version of the device firmware.
 * Usually represented as follows
 * [15:8] Board Identifier
 * [ 7:4] Board Version
 * [ 3:0] FPGA Version
 */
extern uint16_t nm_get_version(nm_device *device);

/**
 * Returns information about the NM500 neural network(NeuroMem) and device
 * Note: this function performs the initialization of the NM500
 * to count the total number of neurons.
 * Therefore, do not use this function during learning or recogition.
 * This function is only useful if you want to check the overall network information
 * at the beginning.
 */
extern uint16_t nm_get_network_info(nm_device *device, nm_network_info *info);

/**
 * Returns the number of the neurons used on the NeuroMem network
 */
extern uint32_t nm_get_neuron_count(nm_device *device);

/**
 * Returns information about the current status of the NeuroMem network
 */
extern uint16_t nm_get_network_status(nm_device *device, nm_network_status *status);

/**
 * Sets the NeuroMem network mode
 * 0: RBF network mode, 1: KNN network mode
 */
extern uint16_t nm_set_network_type(nm_device *device, uint16_t type);

/**
 * Returns information about current network mode
 * 0: RBF network mode, 1: KNN network mode
 */
extern uint16_t nm_get_network_type(nm_device *device);

/**
 * Sets the current(global) context on the NeuroMem network
 */
extern uint16_t nm_set_context(nm_device *device, nm_context *context);

/**
 * Returns information about the current context on the NeuroMem network
 */
extern uint16_t nm_get_context(nm_device *device, nm_context *context);

/**
 * Trains a neuron(s) with given vector on the NeuroMem network.
 */
extern uint16_t nm_learn(nm_device *device, nm_learn_req *data);

/**
 * Identify to which of a set of categories input vector belongs
 */
extern uint16_t nm_classify(nm_device *device, nm_classify_req *data);

/**
 * Returns the neuron information corresponding to the given id.
 */
extern uint16_t nm_read_neuron(nm_device *device, nm_neuron *neuron);

/**
 * Returns a list of all the neurons learned.
 */
extern uint32_t nm_read_neurons(nm_device *device, nm_neuron *neurons, uint32_t count);

/**
 * Uploads the knowledge model (list of neurons) to NM500.
 */
extern uint32_t nm_write_neurons(nm_device *device, nm_neuron *neurons, uint32_t count);

/**
 * Returns the value of the NM500 register specified
 * The power_save function must be called after using this function directly.
 */
extern uint16_t nm_read(nm_device *device, uint16_t reg);

/**
 * Sets the value of the NM500 register specified directly.
 * The power_save function must be called after using this function directly.
 */
extern uint16_t nm_write(nm_device *device, uint16_t reg, uint16_t data);

/**
 * Set the power mode of the NM500. It immediately changes to the power saving mode.
 */
extern uint16_t nm_power_save(nm_device *device);

#ifdef __cplusplus
}
#endif
