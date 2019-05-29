/*
* nm_classifier.cpp
* Copyright (c) 2018,2019 nepes inc, All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.* this list of conditions
and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*  Please use the NeuroMem windows SDK 1.0.3 or later
*
*
*/

#include "nm_classifier.hpp"
#include "opencvhelper.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>
#include <qdir.h>
#include <string>

using namespace cv;
nm_classifier::nm_classifier() {
    m_category_set.insert(0);
#if defined _WIN32
    m_device = std::make_unique<NeuroMem::NeuroMemDevice>();
    NeuroMem::NeuroMemEngine::GetDevices(m_device.get(), 1);
    if (NeuroMem::NeuroMemEngine::Connect(m_device.get()) == 100) {
        std::cout << "connection error" << std::endl;
        return;
    }
#else
    m_device = std::make_unique<nm_device>();

    uint8_t detect_count = 10;
    nm_get_devices(m_device.get(), &detect_count);
    if (nm_connect(m_device.get()) == 100) {
        std::cout << "connection error" << std::endl;
        return;
    }
#endif
    init(7000, 2000);
}

void nm_classifier::init(const uint16_t maxif, const uint16_t minif) {
    m_maxif = maxif;
    m_minif = minif;
    set_feature_algorithm(enum_feature_algorithm::sub_sampling);
#ifdef _WIN32
    NeuroMem::NeuroMemEngine::Reset(m_device.get());
    NeuroMem::NeuroMemEngine::SetNetworkType(m_device.get(), NeuroMem::RBF);
    set_context(1, NeuroMem::NeuroMemNormType::L1, minif, maxif);
#else

    nm_context ctx;
    ctx.maxif = maxif;
    ctx.minif = minif;
    ctx.norm = nm_norm_type::L1;
    ctx.context = 1;
    nm_reset(m_device.get());
    nm_set_network_type(m_device.get(), RBF);
    nm_set_context(m_device.get(), &ctx);
#endif
}

nm_classifier::~nm_classifier() {
    if (m_device != nullptr && m_device->handle != nullptr) {
#ifdef _WIN32
        m_device->handle = nullptr;
#else
        nm_close(m_device.get());
#endif
        m_device.release();
    }
}

bool nm_classifier::is_loaded_from_file() const { return m_loaded_; }

enum_feature_algorithm nm_classifier::get_feature_algorithm() const { return m_algorithm; }

uint16_t nm_classifier::get_neuron_vector_size() const { return m_neuron_vector_size; }

void nm_classifier::set_learning_mode(bool mode) { learning_mode_ = mode; }

uint16_t nm_classifier::maxif() const { return m_maxif; }

uint16_t nm_classifier::minif() const { return m_minif; }

void nm_classifier::set_maxif(const uint16_t max) { m_maxif = max; }

void nm_classifier::set_minif(const uint16_t min) { m_minif = min; }

void nm_classifier::set_feature_algorithm(enum_feature_algorithm algorithm) {
    if (m_algorithm == algorithm) {
        return;
    }
    switch (algorithm) {
        case enum_feature_algorithm::hog:
            if (m_hog.empty()) {
                std::cout << "new hog instance created" << std::endl;
                m_hog = cv::makePtr<HOGDescriptor>(Size(32, 32),                   // winSize
                                                   Size(20, 20),                   // blocksize
                                                   Size(12, 12),                   // blockStride,
                                                   Size(10, 10),                   // cellSize,
                                                   16,                             // nbins,
                                                   1,                              // derivAper,
                                                   -1,                             // winSigma,
                                                   HOGDescriptor::L2Hys,           // histogramNormType,
                                                   0.2,                            // L2HysThresh,
                                                   true,                           // gammal correction,
                                                   HOGDescriptor::DEFAULT_NLEVELS, // nlevels=64
                                                   false);
            }
            m_neuron_vector_size = 256;
            break;
        case enum_feature_algorithm::default_:
            m_neuron_vector_size = 256;
            break;
        case enum_feature_algorithm::sub_sampling:
            m_neuron_vector_size = 256;
            break;
    }

    m_algorithm = algorithm;
}

int nm_classifier::neuron_count() {
    int ret = 0;

    uint32_t neuron_count = 10;
#ifdef _WIN32
    ret = NeuroMem::NeuroMemEngine::GetNeuronCount(m_device.get());
#else
    ret = nm_get_neuron_count(m_device.get(), &neuron_count);
#endif
    return ret;
}

void nm_classifier::set_context(uint16_t context, const uint16_t norm, const uint16_t minif, const uint16_t maxif) {
#ifdef _WIN32
    NeuroMem::NeuroMemContext ctx;
    ctx.context = context;
    ctx.norm = norm;
    ctx.minif = minif;
    ctx.maxif = maxif;
    m_maxif = maxif;
    m_minif = minif;
    NeuroMem::NeuroMemEngine::SetContext(m_device.get(), &ctx);
#else
    nm_context ctx;
    ctx.context = context;
    ctx.norm = norm;
    ctx.minif = minif;
    ctx.maxif = maxif;
    m_maxif = maxif;
    m_minif = minif;
    nm_set_context(m_device.get(), &ctx);

#endif //_WIN32
}

#ifdef _WIN32
bool nm_classifier::classify(NeuroMem::NeuroMemClassifyReq &req) {
    using nm_network_result = NeuroMem::NeuroMemNetworkResult;
    req.size = m_neuron_vector_size;
    req.k = 1;
    bool ret = false;
    assert(m_device->handle != nullptr);
    NeuroMem::NeuroMemEngine::Classify(m_device.get(), &req);
    if ((req.status == nm_network_result::NM_CLASSIFY_IDENTIFIED) ||
        (req.distance[0] < nm_network_result::NM_CLASSIFY_UNKNOWN)) {
        ret = true;
    }
    return ret;
}

void nm_classifier::learn(NeuroMem::NeuroMemLearnReq &req) {
    req.size = m_neuron_vector_size;
    req.category = m_category_set.size();
    NeuroMem::NeuroMemEngine::Learn(m_device.get(), &req);
    if (req.ns > 0) {
        const auto ret = m_category_set.insert(req.category);
        if (!ret.second) {
            std::cout << "cannot be learnd for the category :" << req.category << std::endl;
        } else {
            std::cout << "learned as cat: " << req.category << std::endl;
        }
    } else {
        std::cout << "ns is less or equal to zero: " << std::endl;
    }
}

#else //  others
bool nm_classifier::classify(nm_classify_req &req) {
    req.vector_size = m_neuron_vector_size;
    req.k = 1;
    bool ret = false;
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        nm_classify(m_device.get(), &req);
        if ((req.status == nm_network_result::NM_CLASSIFY_IDENTIFIED) || (req.distance[0] < NM_CLASSIFY_UNKNOWN)) {
            ret = true;
        }
    }
    return ret;
}

void nm_classifier::learn(nm_learn_req &req) {
    req.vector_size = m_neuron_vector_size;
    if (req.category == 65535)
        req.category = m_category_set.size();
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        nm_learn(m_device.get(), &req);
    }

    if (req.status > 0) {
        const auto ret = m_category_set.insert(req.category);
        if (!ret.second) {
            std::cout << "cannot be learnd for the category :" << req.category << std::endl;
        } else {
            std::cout << "learned as cat: " << req.category << std::endl;
        }
    } else {
        std::cout << "ns is less or equal to zero: " << std::endl;
    }
}
#endif

uint16_t nm_classifier::deleteKnowledge(cv::Mat &in) {
    uint16_t cat = classify(in);
    if (cat < UNKNOWN)
        learn(in, 0); // degenerate manually
    return cat;
}

void nm_classifier::learn(cv::Mat &in, int cat) {
    std::vector<uint8_t> feature;

    feature.reserve(m_neuron_vector_size);
    extract_feature_vector(in, feature);

#ifdef _WIN32
    NeuroMem::NeuroMemLearnReq lreq;
    lreq.size = m_neuron_vector_size;
#else
    nm_learn_req lreq;
    lreq.vector_size = m_neuron_vector_size;
#endif
    uint16_t tcat = 0;
    if (cat < 0)
        tcat = m_category_set.size();
    else
        tcat = cat;
    lreq.category = tcat;

    std::move(feature.begin(), feature.end(), std::begin(lreq.vector));
    learn(lreq);
}

uint16_t nm_classifier::classify(cv::Mat &in) {
    std::vector<uint8_t> feature;
    feature.reserve(m_neuron_vector_size);
    extract_feature_vector(in, feature);
#ifdef _WIN32
    NeuroMem::NeuroMemClassifyReq req;
#else
    nm_classify_req req;
#endif
    std::move(feature.begin(), feature.end(), std::begin(req.vector));
    classify(req);
    uint16_t ret = req.category[0];
    std::string s = (ret < UNKNOWN) ? " classified" : " unknown";
    std::cout << "classification: " << req.category[0] << " , " << req.distance[0] << s << std::endl;

    return ret;
}

uint32_t nm_classifier::file_to_neurons() {
    uint32_t neuron_count = 0; // NeuroMemEngine::GetNeuronCount(&hnn);
    std::cout << "neuron count: " << neuron_count << std::endl;
    uint16_t header[4];
    std::ifstream file, file_name;

    file.open("backup/backup.hex", std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Skip the restore process because there isn't backup.hex file" << std::endl;
        return (0);
    }

#ifdef _WIN32
    NeuroMem::NeuroMemContext ctx{};
    NeuroMem::NeuroMemEngine::GetContext(m_device.get(), &ctx);
#else
    nm_context ctx;
    nm_get_context(m_device.get(), &ctx);
#endif

    std::cout << "file to neuron before context:" << ctx.maxif << std::endl;
    std::cout << "Start restore process from backup.hex file" << std::endl;

    const char *path = "backup/backup.hex";
    nm_neuron *neurons = (nm_neuron *)malloc(sizeof(nm_neuron) * neuron_count);
    neurons[0].size = NEURON_MEMORY;

    nm_load_model(m_device.get(), neurons, &neuron_count, path);

    printf("[tc_load_model] neuron count: %d\n", neuron_count);
    for (int i = 0; i < neuron_count; i++) {
        printf("[tc_load_model] nid: %-5d ncr: %-5d cat: %-5d aif: %-5d minif: %-5d\n", neurons[i].nid, neurons[i].ncr,
               neurons[i].cat, neurons[i].aif, neurons[i].minif);
        for (int j = 0; j < NEURON_MEMORY; j++) {
            printf("%-3d ", neurons[i].model[j]);
        }
        printf("\n");
    }

    free(neurons);

    std::cout << "Restore process is finished." << std::endl;
    m_loaded_ = true;
    return neuron_count;
}

void nm_classifier::neurons_to_file() {
    uint32_t neuron_count = 0;
    uint16_t r = NM_SUCCESS;
#ifdef _WIN32
    neuron_count = NeuroMem::NeuroMemEngine::GetNeuronCount(m_device.get());
    if (neuron_count == 0) {
        return;
    }
#else
    r = nm_get_neuron_count(m_device.get(), &neuron_count);
    if (neuron_count == 0) {
        return;
    }
#endif

    const char *filename = "backup/backup.hex";
    cv::utils::fs::createDirectory("backup");

    r = nm_get_neuron_count(m_device.get(), &neuron_count);
    printf("[tc_save_model] neuron count: %d\n", neuron_count);

    nm_neuron *neurons = (nm_neuron *)malloc(sizeof(nm_neuron) * neuron_count);
    neurons[0].size = NEURON_MEMORY;

    r = nm_read_neurons(m_device.get(), neurons, &neuron_count);

    r = nm_save_model(m_device.get(), neurons, neuron_count, filename);
    printf("[tc_save_model] save neurons to '%s' ... %d\n", filename, r);

    free(neurons);
}

void nm_classifier::pyramid_reduction(Mat &input, Mat &output, int size) {
    auto pyr = input.clone();
    for (auto i = 0; pyr.rows > size && pyr.cols > size; i++) {
        if ((pyr.rows / 2) >= size || (pyr.cols / 2) >= size) {
            pyrDown(pyr, pyr, Size(pyr.cols / 2, pyr.rows / 2));
        } else {
            break;
        }
    }
    resize(pyr, output, Size(size, size), 0, 0, INTER_LINEAR);
}

void nm_classifier::extract_feature_hog(Mat &input, Mat &output) {
    //  if (input.rows > 32 && input.cols > 32)
    //    pyramid_reduction(input, input, 32);
    cv::resize(input, input, cv::Size(32, 32));
    std::vector<float> descriptors_values;

    m_hog->compute(input, descriptors_values);
    const Mat mat(descriptors_values, CV_64FC1);

    normalize(mat, output, 0, 255, NORM_MINMAX, CV_8UC1);
    //	std::cout << "hog:" <<  output.size() << std::endl;
}

void nm_classifier::extract_feature_vector(Mat input, std::vector<uint8_t> &v) {
    Mat output;
    // some algorithms need color images , whereas most are needed gray.
    switch (m_algorithm) {
        case enum_feature_algorithm::hog:
            if (input.channels() != 1) {
                cvtColor(input, input, cv::COLOR_BGR2GRAY);
            }
            extract_feature_hog(input, output);
            break;
        case enum_feature_algorithm::sub_sampling:
            if (input.channels() != 1) {
                cvtColor(input, input, cv::COLOR_BGR2GRAY);
            }
            cv::resize(input, output, cv::Size(16, 16));
            break;

        case enum_feature_algorithm::default_:
            break;
    }

    // fill_neuron_vector(output, v);
    for (int j = 0; j < m_neuron_vector_size; j++) {
        v.push_back(*output.data++);
    }
    // std::cout << "Mat: " << output << std::endl;
}

void nm_classifier::read_neurons(std::vector<Mat> &vl) {

    uint32_t neuron_count = 0;
#ifdef _WIN32
    neuron_count = NeuroMem::NeuroMemEngine::GetNeuronCount(m_device.get());
    if (neuron_count == 0) {
        return;
    }
    std::vector<NeuroMem::NeuroMemNeuron> neurons(neuron_count);
    neurons[0].size = m_neuron_vector_size;
    NeuroMem::NeuroMemEngine::ReadNeurons(m_device.get(), &neurons[0], neuron_count);
#else
    uint16_t r = nm_get_neuron_count(m_device.get(), &neuron_count);
    if (neuron_count == 0) {
        return;
    }
    std::vector<nm_neuron> neurons(neuron_count);
    neurons[0].size = m_neuron_vector_size;
    r = nm_read_neurons(m_device.get(), &neurons[0], &neuron_count);
#endif
    for (const auto &neuron : neurons) {
        cv::Mat r(16, 16, CV_8UC1);
        std::copy(neuron.model, neuron.model + neuron.size,
                  r.data); //    memcpy(r.data, neuron.model, neuron.size*sizeof(uchar));
        vl.push_back(r);
    }
}

//
// get neurons an put it into QListWidget as widget items
//
void nm_classifier::read_neurons(QListWidget *ql) {

    uint16_t r = 0;
    uint32_t neuron_count = 0;
#ifdef _WIN32
    neuron_count = NeuroMem::NeuroMemEngine::GetNeuronCount(m_device.get());
    if (neuron_count == 0) {
        return;
    }
    std::vector<NeuroMem::NeuroMemNeuron> neurons(neuron_count);
    neurons[0].size = m_neuron_vector_size;
    NeuroMem::NeuroMemEngine::ReadNeurons(m_device.get(), &neurons[0], neuron_count);
#else
    r = nm_get_neuron_count(m_device.get(), &neuron_count);
    if (neuron_count == 0) {
        return;
    }
    std::vector<nm_neuron> neurons(neuron_count);
    neurons[0].size = m_neuron_vector_size;
    r = nm_read_neurons(m_device.get(), &neurons[0], &neuron_count);
#endif
    for (const auto &neuron : neurons) {
        cv::Mat r(16, 16, CV_8UC1);
        std::copy(neuron.model, neuron.model + neuron.size,
                  r.data); //    memcpy(r.data, neuron.model, neuron.size*sizeof(uchar));
        QImage img = Mat2QImage(r);
        QPixmap pix = QPixmap::fromImage(img).scaled(QSize(100, 100), Qt::KeepAspectRatio);
        ql->addItem(new QListWidgetItem(QIcon(pix), QString(neuron.cat)));
    }
}
