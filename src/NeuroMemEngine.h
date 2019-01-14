/*
 * Copyright (c) 2017-2018, nepes corp, All rights reserved.
 *
 */
#ifndef NEUROMEM_ENGINE_H
#define NEUROMEM_ENGINE_H

#if defined _WIN32 || defined __CYGWIN__
#if defined __GNUC__ || LLVM
#define NEUROMEMENGINEDLL_API __attribute__((dllexport))
#define __stdcall
#else
#define NEUROMEMENGINEDLL_API __declspec(dllexport)
#endif
#else
#define NEUROMEMENGINEDLL_API __attribute__((visibility("default")))
#define __stdcall
#endif

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NM_NCR 0x00
#define NM_COMP 0x01
#define NM_LCOMP 0x02
#define NM_DIST 0x03
#define NM_INDEXCOMP 0x03
#define NM_CAT 0x04
#define NM_AIF 0x05
#define NM_MINIF 0x06
#define NM_MAXIF 0x07
#define NM_TESTCOMP 0x08
#define NM_TESTCAT 0x09
#define NM_NID 0x0A
#define NM_GCR 0x0B
#define NM_RSTCHAIN 0x0C
#define NM_NSR 0x0D
#define NM_POWERSAVE 0x0E
#define NM_NCOUNT 0x0F
#define NM_FORGET 0x0F

#define NEURON_MEMORY 256
#define CLASSIFY_MAX_K 9

#define CAM_FRAME_BUF 1843200

namespace NeuroMem {
  typedef enum _NeuroMemReturnStatus {
    NM_SUCCESS = 0,
    NM_ERROR_UNKNOWN,
    NM_ERROR_DEVICE_NOT_FOUND,
    NM_ERROR_DEVICE_INFO_FETCH_FAILED,
    NM_ERROR_DEVICE_OPEN_FAILED,
    NM_ERROR_INIT_FAILED = 100,
    NM_ERROR_INVALID_PARAMETER,
    NM_ERROR_NOT_SUPPORT,
    NM_ERROR_IO_TIMEOUT,
    NM_ERROR_IO_FAILED
  } NeuroMemReturnStatus;

  typedef enum _NeuroMemNetworkResult {
    NM_LEARN_ALREADY_KNOWN = 0,
    NM_LEARN_SUCCESS = 1,
    NM_LEARN_DEGENERATED = 2,
    NM_CLASSIFY_UNKNOWN = 0,
    NM_CLASSIFY_UNCERTAIN = 4,
    NM_CLASSIFY_IDENTIFIED = 8
  } NeuroMemNetworkResult;

  typedef enum _NeuroMemNetworkType { RBF = 0, KNN = 1 } NeuroMemNetworkType;

  typedef enum _NeuroMemNormType { L1 = 0, Lsup = 1 } NeuroMemNormType;

  typedef struct _NeuroMemDevice {
    uint16_t idx;
    uint16_t type;
    uint16_t id;
    uint16_t vid;
    uint16_t pid;
    void *handle;
  } NeuroMemDevice;

  typedef struct _NeuroMemNetworkInfo {
    uint16_t neuronMemorySize;
    uint32_t neuronSize;
    uint16_t version;
  } NeuroMemNetworkInfo;

  typedef struct _NeuroMemContext {
    uint16_t context;
    uint16_t norm;
    uint16_t minif;
    uint16_t maxif;
  } NeuroMemContext;

  typedef struct _NeuroMemNetworkStatus {
    uint16_t networkType;
    uint32_t networkUsed;
    uint16_t context;
    uint16_t norm;
  } NeuroMemNetworkStatus;

  typedef struct _NeuroMemClassifyReq {
    uint8_t vector[256];
    uint16_t size;
    uint16_t k;
    NeuroMemNetworkResult status;
    uint16_t matchedCount;
    uint16_t degenerated[CLASSIFY_MAX_K];
    uint16_t distance[CLASSIFY_MAX_K];
    uint16_t category[CLASSIFY_MAX_K];
    uint32_t nid[CLASSIFY_MAX_K];
  } NeuroMemClassifyReq;

  typedef struct _NeuroMemLearnReq {
    uint8_t vector[256];
    uint16_t size;
    uint16_t category;
    NeuroMemNetworkResult ns;
  } NeuroMemLearnReq;

  typedef struct _NeuroMemNeuron {
    uint8_t model[256];
    uint16_t size;
    uint16_t ncr;
    uint16_t aif;
    uint16_t minif;
    uint16_t cat;
    uint32_t nid;
  } NeuroMemNeuron;

  typedef struct _DataCameraFrame {
    uint8_t *data;
    uint32_t offset;
    uint32_t size;
  } DataCameraFrame;

  class NeuroMemEngine {
  public:
    // Common functions
    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetDevices(NeuroMemDevice *devices, uint16_t count);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall Connect(NeuroMemDevice *device);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall Forget(NeuroMemDevice *device);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall Reset(NeuroMemDevice *device);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetVersion(NeuroMemDevice *device);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetNetworkInfo(NeuroMemDevice *device, NeuroMemNetworkInfo *info);
    static NEUROMEMENGINEDLL_API uint32_t __stdcall GetNeuronCount(NeuroMemDevice *device);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetNetworkStatus(NeuroMemDevice *device,
                                                                     NeuroMemNetworkStatus *status);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall SetNetworkType(NeuroMemDevice *device, uint16_t type);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetNetworkType(NeuroMemDevice *device);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall SetContext(NeuroMemDevice *device, NeuroMemContext *context);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetContext(NeuroMemDevice *device, NeuroMemContext *context);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall Learn(NeuroMemDevice *device, NeuroMemLearnReq *data);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall Classify(NeuroMemDevice *device, NeuroMemClassifyReq *data);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall ReadNeuron(NeuroMemDevice *device, NeuroMemNeuron *neuron);
    static NEUROMEMENGINEDLL_API uint32_t __stdcall ReadNeurons(NeuroMemDevice *device, NeuroMemNeuron *neurons,
                                                                uint32_t count);

    static NEUROMEMENGINEDLL_API uint32_t __stdcall WriteNeurons(NeuroMemDevice *device, NeuroMemNeuron *neurons,
                                                                 uint32_t count);

    static NEUROMEMENGINEDLL_API uint16_t __stdcall Read(NeuroMemDevice *device, uint16_t reg);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall Write(NeuroMemDevice *device, uint16_t reg, uint16_t data);

    // For Prodigy Board only
    static NEUROMEMENGINEDLL_API uint16_t __stdcall GetFrame(NeuroMemDevice *device, DataCameraFrame *frame);
    static NEUROMEMENGINEDLL_API uint16_t __stdcall ReadBuffer(NeuroMemDevice *device, uint8_t *data, uint16_t size);
    static NEUROMEMENGINEDLL_API uint32_t __stdcall SetROI(NeuroMemDevice *device, NeuroMemNeuron *neurons);
    static NEUROMEMENGINEDLL_API uint32_t __stdcall SetResolution(NeuroMemDevice *device, NeuroMemNeuron *neurons);
  };
} // namespace NeuroMem

#ifdef __cplusplus
}
#endif

#endif // NEUROMEMENGINE_H
