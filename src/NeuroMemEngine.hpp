/*
 * Copyright (c) 2017-2018, nepes corp, All rights reserved.
 * kspark @ nepes.co.kr
 */
#pragma once

#if not defined _WIN32
#include <nmengine.h>
#else

#if defined _WIN32 || defined __CYGWIN__
#if defined __GNUC__ || LLVM
#define nm_engine_api __attribute__((dllexport))
#define __stdcall
#else
#define nm_engine_api __declspec(dllexport)
#endif
#else
#define nm_engine_api __attribute__((visibility("default")))
#define __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>

namespace NeuroMem {
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

  using NeuroMemReturnStatus = enum _NeuroMemReturnStatus {
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
  };

  using NeuroMemNetworkResult = enum _NeuroMemNetworkResult {
    NM_LEARN_ALREADY_KNOWN = 0,
    NM_LEARN_SUCCESS = 1,
    NM_LEARN_DEGENERATED = 2,
    NM_CLASSIFY_UNKNOWN = 0,
    NM_CLASSIFY_UNCERTAIN = 4,
    NM_CLASSIFY_IDENTIFIED = 8
  };

  using NeuroMemNetworkType = enum _NeuroMemNetworkType { RBF = 0, KNN = 1 };
  using NeuroMemNormType = enum _NeuroMemNormType { L1 = 0, Lsup = 1 };

  using NeuroMemDevice = struct _NeuroMemDevice {
    uint16_t idx;
    uint16_t type;
    uint16_t id;
    uint16_t vid;
    uint16_t pid;
    void *handle;
  };

  using NeuroMemNetworkInfo = struct _NeuroMemNetworkInfo {
    uint16_t neuronMemorySize;
    uint32_t neuronSize;
    uint16_t version;
  };

  using NeuroMemContext = struct _NeuroMemContext {
    uint16_t context;
    uint16_t norm;
    uint16_t minif;
    uint16_t maxif;
  };

  using NeuroMemNetworkStatus = struct _NeuroMemNetworkStatus {
    uint16_t networkType;
    uint32_t networkUsed;
    uint16_t context;
    uint16_t norm;
  };

  using NeuroMemClassifyReq = struct _NeuroMemClassifyReq {
    uint8_t vector[256];
    uint16_t size;
    uint16_t k;
    NeuroMemNetworkResult status;
    uint16_t matchedCount;
    uint16_t degenerated[CLASSIFY_MAX_K];
    uint16_t distance[CLASSIFY_MAX_K];
    uint16_t category[CLASSIFY_MAX_K];
    uint32_t nid[CLASSIFY_MAX_K];
  };

  using NeuroMemLearnReq = struct _NeuroMemLearnReq {
    uint8_t vector[256];
    uint16_t size;
    uint16_t category;
    NeuroMemNetworkResult ns;
  };

  using NeuroMemNeuron = struct _NeuroMemNeuron {
    uint8_t model[256];
    uint16_t size;
    uint16_t ncr;
    uint16_t aif;
    uint16_t minif;
    uint16_t cat;
    uint32_t nid;
  };

  using DataCameraFrame = struct _DataCameraFrame {
    uint8_t *data;
    uint32_t offset;
    uint32_t size;
  };

  class NeuroMemEngine {
  public:
    // Common functions
    static nm_engine_api uint16_t __stdcall GetDevices(NeuroMemDevice *devices, uint16_t count);
    static nm_engine_api uint16_t __stdcall Connect(NeuroMemDevice *device);

    static nm_engine_api uint16_t __stdcall Forget(NeuroMemDevice *device);
    static nm_engine_api uint16_t __stdcall Reset(NeuroMemDevice *device);

    static nm_engine_api uint16_t __stdcall GetVersion(NeuroMemDevice *device);
    static nm_engine_api uint16_t __stdcall GetNetworkInfo(NeuroMemDevice *device, NeuroMemNetworkInfo *info);
    static nm_engine_api uint32_t __stdcall GetNeuronCount(NeuroMemDevice *device);
    static nm_engine_api uint16_t __stdcall GetNetworkStatus(NeuroMemDevice *device, NeuroMemNetworkStatus *status);

    static nm_engine_api uint16_t __stdcall SetNetworkType(NeuroMemDevice *device, uint16_t type);
    static nm_engine_api uint16_t __stdcall GetNetworkType(NeuroMemDevice *device);

    static nm_engine_api uint16_t __stdcall SetContext(NeuroMemDevice *device, NeuroMemContext *context);
    static nm_engine_api uint16_t __stdcall GetContext(NeuroMemDevice *device, NeuroMemContext *context);

    static nm_engine_api uint16_t __stdcall Learn(NeuroMemDevice *device, NeuroMemLearnReq *data);
    static nm_engine_api uint16_t __stdcall Classify(NeuroMemDevice *device, NeuroMemClassifyReq *data);

    static nm_engine_api uint16_t __stdcall ReadNeuron(NeuroMemDevice *device, NeuroMemNeuron *neuron);
    static nm_engine_api uint32_t __stdcall ReadNeurons(NeuroMemDevice *device, NeuroMemNeuron *neurons,
                                                        uint32_t count);

    static nm_engine_api uint32_t __stdcall WriteNeurons(NeuroMemDevice *device, NeuroMemNeuron *neurons,
                                                         uint32_t count);

    static nm_engine_api uint16_t __stdcall Read(NeuroMemDevice *device, uint16_t reg);
    static nm_engine_api uint16_t __stdcall Write(NeuroMemDevice *device, uint16_t reg, uint16_t data);

    // For Prodigy Board only
    static nm_engine_api uint16_t __stdcall GetFrame(NeuroMemDevice *device, DataCameraFrame *frame);
    static nm_engine_api uint16_t __stdcall ReadBuffer(NeuroMemDevice *device, uint8_t *data, uint16_t size);
    static nm_engine_api uint32_t __stdcall SetROI(NeuroMemDevice *device, NeuroMemNeuron *neurons);
    static nm_engine_api uint32_t __stdcall SetResolution(NeuroMemDevice *device, NeuroMemNeuron *neurons);
  };
} // namespace NeuroMem

#ifdef __cplusplus
}
#endif

#endif // not linux
