#pragma once
/*
 * Copyright (c) 2017-2019, nepes corp, All rights reserved.
 * Version 2.0.0
 */

#ifdef WIN32
#ifdef NMENGINEDLL_EXPORTS
#define NMENGINE_API __declspec(dllexport)
#else
#define NMENGINE_API __declspec(dllimport)
#endif
#else
#define NMENGINE_API
#endif

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

/**
 * Definition of return status code of api.
 */
using nm_return_status = enum struct _nm_return_status {
    NM_SUCCESS = 0,
    NM_ERROR_UNKNOWN = 100,
    NM_ERROR_DEVICE_NOT_FOUND,
    NM_ERROR_DEVICE_INFO_FETCH_FAILED,
    NM_ERROR_DEVICE_OPEN_FAILED,
    NM_ERROR_INIT_FAILED,
    NM_ERROR_INVALID_PARAMETER,
    NM_ERROR_NOT_SUPPORT,
    NM_ERROR_IO_TIMEOUT,
    NM_ERROR_IO_FAILED,
    NM_ERROR_RESOURCE

};

/**
 * Definition of network status codes of learning and classifying.
 */
using nm_network_result = enum struct _nm_network_result {
    // Learning result
    NM_LEARN_ALREADY_KNOWN = 0,
    NM_LEARN_SUCCESS = 1,
    NM_LEARN_ADJUSTED = 2,
    NM_LEARN_DEGENERATED = 3,
    // Classifying result
    NM_CLASSIFY_UNKNOWN = 0,
    NM_CLASSIFY_UNCERTAIN = 4,
    NM_CLASSIFY_IDENTIFIED = 8

};

/**
 * Definition of network type
 * It must be set to RBF when learning.
 */
typedef enum _nm_network_type {
    RBF = 0,
    KNN = 1

} nm_network_type;

/**
 * Definition of norm type that is used to measure distance between vectors
 * L1: Manhattan
 * Lsup: supremum(max)
 */
typedef enum _nm_norm_type {
    L1 = 0,
    Lsup = 1

} nm_norm_type;

/**
 * Definition of data type for device connection
 */
typedef struct _nm_device {
    void *dev;
    void *handle;
    void *lock;
    uint16_t type;
    uint16_t id;
    uint16_t vid;
    uint16_t pid;
    uint16_t version;
    uint8_t is_open;

} nm_device;

/**
 * Definition of data type for device network information
 * neuron_size: number of neurons
 * neuron_memory_size: memory size of each neuron.
 * version: version of device firmware
 */
typedef struct _nm_network_info {
    uint32_t neuron_count;
    uint16_t neuron_memory_size;
    uint16_t version;

} nm_network_info;

/**
 * Definition of data type for context
 * context: context id
 * norm: norm type
 * minif: minimum influence field
 * maxif: maximum influence field
 */
typedef struct _nm_context {
    uint16_t context;
    uint16_t norm;
    uint16_t minif;
    uint16_t maxif;

} nm_context;

/**
 * Definition of data type for network status
 * network_used: number of used/committed neurons
 * network_type: RBF or KNN
 * context: current/global context
 * norm: current norm type
 */
typedef struct _nm_network_status {
    uint32_t network_used;
    uint16_t context;
    uint8_t network_type;
    uint8_t norm;

} nm_network_status;

/**
 * Definition of data type for neuron
 * nid: neuron id
 * size: vector length used in neuron memory (vector size)
 * ncr: neuron context (context id, norm)
 * aif: active influence field (threshold of activation function)
 * minif: minimum influence field
 * model: prototype (stored weight memory)
 */
typedef struct _nm_neuron {
    uint32_t nid;
    uint16_t size;
    uint16_t ncr;
    uint16_t aif;
    uint16_t minif;
    uint16_t cat;
    uint8_t model[256];

} nm_neuron;

/**
 * Definition of data type for classifing request/response
 * <<input>>
 * vector_size: input data size
 * vector: input data
 * k: number of returns matched
 *
 * <<output>>
 * status: network status of classifying
 * matched_count: number(n) of matched
 * nid[n]: id of neuron matched
 * category[n]: category of neuron matched
 * degenerated[n]: degenerated flag of neuron matched
 *   1: degenerated
 * distance[n]: distance between input data and prototype of neuron matched
 */
typedef struct _nm_classify_req {
    nm_network_result status;
    uint16_t vector_size;
    uint16_t k;
    uint16_t matched_count;
    uint32_t nid[CLASSIFY_MAX_K];
    uint16_t degenerated[CLASSIFY_MAX_K];
    uint16_t distance[CLASSIFY_MAX_K];
    uint16_t category[CLASSIFY_MAX_K];
    uint8_t vector[256];

} nm_classify_req;

/**
 * Definition of data type for learning request/response
 * <<input>>
 * query_affected: flag for whether to retrieve affected neuron information
 *   generally flagging is not required.
 * category: category of input data(vector)
 * vector_size: input data size
 * vector: input data
 *
 * <<output>>
 * status: network status of learning
 * affected_count: number of affected neurons
 * affected_neurons[affected_count]: list of affected neurons
 */
typedef struct _nm_learn_req {
    nm_network_result status;
    nm_neuron affected_neurons[10];
    uint16_t affected_count;
    uint16_t category;
    uint16_t vector_size;
    uint8_t vector[256];
    uint8_t query_affected;

} nm_learn_req;

/**
 * Definition of data type for batch learning request/response
 * <<input>>
 * iterable: flag for whether to iterate batch learning
 * iter_count: number of iteration (epoch)
 * vector_count: number of vectors
 * vector_size: input data size
 * vectors: list of input data
 * categories: list of category of input data(vector)
 *
 * <<output>>
 */
typedef struct _nm_learn_batch_req {
    uint32_t vector_count;
    uint32_t *iter_result;
    uint16_t iter_count;
    uint16_t iterable;
    uint16_t vector_size;
    uint16_t *categories;
    uint8_t *vectors;

} nm_learn_batch_req;

/**
 * Definition of data type for clusterize request/response
 * <<input>>
 * initial_category: initial category id (it must be greater than 0)
 * incrementof: unit of increasement of category id
 * vector_count: number of vectors
 * vector_size: input data size
 * vectors: list of input data
 *
 * <<output>>
 */
typedef struct _nm_clusterize_req {
    uint32_t vector_count;
    uint16_t vector_size;
    uint16_t initial_category;
    uint16_t incrementof;
    uint8_t *vectors;

} nm_clusterize_req;

/**
 * Definition of data type for knowledge(or trained) model
 * count: number of neurons used/committed
 * max_context: the largest context id (1~127)
 * max_category: the largest cateogry id (1~32766)
 */
typedef struct _nm_model_info {
    uint32_t count;
    uint16_t max_context;
    uint16_t max_category;

} nm_model_info;

/**
 * Definition of data type for knowledge(or trained) model analysis
 * it shows distribution of neuron per category
 * <<input>>
 * context: target context id for analysis
 *
 * <<output>>
 * count: number of neurons used/committed in given context
 * histo_cat[the largest cateogry id + 1]: number of neurons per cateogory
 * histo_deg[the largest cateogry id + 1]: nember of degenerated neuron per category
 */
typedef struct _nm_model_stat {
    uint16_t context;
    uint32_t count;
    uint16_t *histo_cat;
    uint16_t *histo_deg;

} nm_model_stat;

/**
 * Definition of data type for camera frame image.
 * Notice, this is supported on certain devices with camera capabilities.
 * e.g prodigy
 */
typedef struct _nm_camera_frame {
    uint8_t *data;
    uint32_t offset;
    uint32_t size;

} nm_camera_frame;

/**
 * Gets a list of the NM500 devices.
 */
extern NMENGINE_API uint16_t nm_get_devices(nm_device *devices, uint8_t *count);

/**
 * Opens selected device and performs the initialization of the NM500.
 */
extern NMENGINE_API uint16_t nm_connect(nm_device *device);

/**
 * Closes selected device and release resources.
 */
extern NMENGINE_API uint16_t nm_close(nm_device *device);

/**
 * Clears all knowledge on the NeuroMem network
 */
extern NMENGINE_API uint16_t nm_forget(nm_device *device);

/**
 * Clears all knowledge on the NeuroMem network
 */
extern NMENGINE_API uint16_t nm_reset(nm_device *device);

/**
 * Gets the version of the device firmware. * Usually represented as follows
 * [15:8] Board Identifier
 * [ 7:4] Board Version
 * [ 3:0] FPGA Version
 */
extern NMENGINE_API uint16_t nm_get_version(nm_device *device, uint16_t *version);

/**
 * Gets information about the NM500 neural network(NeuroMem) and device
 * Note: this function performs the initialization of the NM500
 * to count the total number of neurons.
 * Therefore, do not use this function during learning or recogition.
 * This function is only useful if you want to check the overall network information
 * at the beginning.
 */
extern NMENGINE_API uint16_t nm_get_network_info(nm_device *device, nm_network_info *info);

/**
 * Gets the number of the neurons used on the NeuroMem network
 */
extern NMENGINE_API uint16_t nm_get_neuron_count(nm_device *device, uint32_t *count);

/**
 * Gets the information about the current status of the NeuroMem network
 */
extern NMENGINE_API uint16_t nm_get_network_status(nm_device *device, nm_network_status *status);

/**
 * Sets the NeuroMem network mode
 * 0: RBF network mode, 1: KNN network mode
 */
extern NMENGINE_API uint16_t nm_set_network_type(nm_device *device, uint8_t type);

/**
 * Gets information about current network mode
 * 0: RBF network mode, 1: KNN network mode
 */
extern NMENGINE_API uint16_t nm_get_network_type(nm_device *device, uint8_t *type);

/**
 * Sets the current(global) context on the NeuroMem network
 *
 * If you decrease the maxif value or increase the minif value,
 * the sensitivity of the network increases.
 */
extern NMENGINE_API uint16_t nm_set_context(nm_device *device, nm_context *context);

/**
 * Gets information about the current context on the NeuroMem network
 */
extern NMENGINE_API uint16_t nm_get_context(nm_device *device, nm_context *context);

/**
 * Trains a neuron(s) with given vector on the NeuroMem network.
 */
extern NMENGINE_API uint16_t nm_learn(nm_device *device, nm_learn_req *data);

/**
 * Trains a neuron(s) with given vectors on the NeuroMem network.
 */
extern NMENGINE_API uint16_t nm_learn_batch(nm_device *device, nm_learn_batch_req *data);

/**
 * Clusterize given vectors on the NeuroMem network.
 */
extern NMENGINE_API uint16_t nm_clusterize(nm_device *device, nm_clusterize_req *data);

/**
 * Identifies to which of a set of categories input vector belongs
 */
extern NMENGINE_API uint16_t nm_classify(nm_device *device, nm_classify_req *data);

/**
 * Gets the neuron information corresponding to the given id.
 */
extern NMENGINE_API uint16_t nm_read_neuron(nm_device *device, nm_neuron *neuron);

/**
 * Gets a list of all the neurons learned.
 */
extern NMENGINE_API uint16_t nm_read_neurons(nm_device *device, nm_neuron *neurons, uint32_t *count);

/**
 * Uploads the knowledge model (list of neurons) to NM500.
 */
extern NMENGINE_API uint16_t nm_write_neurons(nm_device *device, nm_neuron *neurons, uint32_t *count);

/**
 * Gets the value of the NM500 register specified
 * The power_save function must be called after using this function directly.
 */
extern NMENGINE_API uint16_t nm_read(nm_device *device, uint16_t reg, uint16_t *data);

/**
 * Sets the value of the NM500 register specified directly.
 * The power_save function must be called after using this function directly.
 */
extern NMENGINE_API uint16_t nm_write(nm_device *device, uint16_t reg, uint16_t data);

/**
 * Sets the power mode of the NM500. It immediately changes to the power saving mode.
 */
extern NMENGINE_API uint16_t nm_power_save(nm_device *device);

/**
 * Gets the size information of the knowledge model.
 */
extern NMENGINE_API uint16_t nm_get_model_info(nm_device *device, nm_model_info *info);

/**
 * Gets the category distribution stats of the given context in knowledge model
 */
extern NMENGINE_API uint16_t nm_get_model_stat(nm_device *device, nm_model_stat *stat);

/**
 * Saves knowledge model to file
 */
extern NMENGINE_API uint16_t nm_save_model(nm_device *device, nm_neuron *neurons, uint32_t count, const char *path);

/**
 * Reads knowledge model from file and upload it to NM500.
 */
extern NMENGINE_API uint16_t nm_load_model(nm_device *device, nm_neuron *neurons, uint32_t *count, const char *path);

//
// Camera APIs that is supported on certain devices with camera capabilities.
// e.g prodigy
//

/**
 * Gets the camera frame data from device
 */
extern NMENGINE_API uint16_t nm_get_camera_frame(nm_device *device, nm_camera_frame *frame);

/**
 * Sets the region of interest of the camera frame
 */
extern NMENGINE_API uint16_t nm_set_camera_roi(nm_device *device, uint16_t x, uint16_t y, uint16_t width,
                                               uint16_t height);

//
// Flash Memory APIs that is supported on certain devices with flash memory capabilities.
// e.g brilliant, prodigy
//

/**
 * Updates firmware image to the device
 */
extern NMENGINE_API uint16_t nm_update_firmware(nm_device *device, const char *path);

#ifdef __cplusplus
}
#endif
