/*
* nm_face_recognizer.cpp
* Copyright (c) 2018, nepes inc, All rights reserved.
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

#include "nm_classifier.h"
#include <algorithm>
#include <map>
#include <qdir.h>
#include <opencv2/opencv.hpp>

using namespace NeuroMem;
using namespace cv;

nm_classifier::nm_classifier(): m_device(std::make_unique<NeuroMem::NeuroMemDevice>()) {
  m_category_set_.insert(0);
#if defined _WIN32 
  NeuroMem::NeuroMemEngine::GetDevices(m_device.get(), 1);
  if (NeuroMem::NeuroMemEngine::Connect(m_device.get()) == 100) {
    std::cout << "connection error" << std::endl;
    return;
  }
#endif
  init(1000, 100);
}

void nm_classifier::init(const uint16_t maxif, const uint16_t minif) {
  m_maxif = maxif;
  m_minif = minif;
  set_feature_algorithm(enum_feature_algorithm::hog);
#if defined _WIN32
  NeuroMemEngine::Reset(m_device.get());
  NeuroMemEngine::SetNetworkType(m_device.get(), RBF);
  set_context(1, NeuroMemNormType::L1, minif, maxif);
#endif
}

nm_classifier::~nm_classifier() {
  if (m_device->handle != nullptr)
    m_device->handle = nullptr;
  if (m_device)
    m_device.release();
}


bool nm_classifier::is_loaded_from_file() const { return m_loaded_; }

void nm_classifier::get_names(std::vector<std::string> &ret) {
  ret.reserve(m_names_.size());

  std::for_each(std::begin(m_names_), std::end(m_names_),
                [&ret](const std::map<uint16_t, std::string>::value_type &entry) { ret.push_back(entry.second); });
}

enum_feature_algorithm nm_classifier::get_feature_algorithm() const { return m_algorithm; }

uint16_t nm_classifier::get_neuron_vector_size() const { return m_neuron_vector_size; }

void nm_classifier::set_learning_mode(bool mode) { learning_mode_ = mode; }

uint16_t nm_classifier::maxif() const { return m_maxif; }

uint16_t nm_classifier::minif() const { return m_minif; }

void nm_classifier::set_maxif(const uint16_t max) { m_maxif = max; }

void nm_classifier::set_minif(const uint16_t min) { m_minif = min; }

void nm_classifier::set_feature_algorithm(enum_feature_algorithm algorithm) {
  if (m_algorithm == algorithm)
    return;
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
      break;
  }

  m_algorithm = algorithm;
}

void nm_classifier::set_context(uint16_t context, const uint16_t norm, const uint16_t minif, const uint16_t maxif) {
  NeuroMemContext ctx;
  ctx.context = context;
  ctx.norm = norm;
  ctx.minif = minif;
  ctx.maxif = maxif;
  m_maxif = maxif;
  m_minif = minif;

  NeuroMemEngine::SetContext(m_device.get(), &ctx);
}

bool nm_classifier::classify(NeuroMemClassifyReq &req) {
  req.size = m_neuron_vector_size;
  req.k = 1;
  bool ret = false;
  assert(m_device->handle != nullptr);
#if defined _WIN32
    const int t = NeuroMemEngine::Classify(m_device.get(), &req);
    if (t == NM_CLASSIFY_IDENTIFIED || req.distance[0] < UNKNOWN) {
      ret = true;
    }
#endif
  return ret;
}

void nm_classifier::learn(NeuroMemLearnReq &req) {
  req.size = m_neuron_vector_size;
  req.category = m_category_set_.size();
#if defined _WIN32
  NeuroMemEngine::Learn(m_device.get(), &req);
#endif
  if (req.ns > 0) {
    const auto ret = m_category_set_.insert(req.category);
    if (!ret.second) 
      std::cout << "cannot be learnd for the category :" << req.category << std::endl;
  }
}

void nm_classifier::learn(cv::Mat &in) { 
  std::vector<uint8_t> feature(m_neuron_vector_size); 
  extract_feature_vector(in, feature);
  NeuroMemLearnReq lreq;
  std::move(feature.begin(), feature.end(), lreq.vector);
  learn(lreq);
}

bool nm_classifier::classify(cv::Mat &in) { 
  std::vector<uint8_t> feature(m_neuron_vector_size); 
  extract_feature_vector(in, feature);
  NeuroMemClassifyReq req;
  std::move(feature.begin(), feature.end(), req.vector);
  return classify(req);

}
uint32_t nm_classifier::file_to_neurons() {
  uint32_t neuron_count = 0; // NeuroMemEngine::GetNeuronCount(&hnn);

#if defined _WIN32
  std::cout << "neuron count: " << neuron_count << std::endl;

  uint16_t header[4];
  std::ifstream file, file_name;

  file.open("backup/backup.hex", std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Skip the restore process because there isn't backup.hex file" << std::endl;
    return (0);
  }

  file_name.open("backup/name.txt", std::ios::in);
  if (!file_name.is_open()) {
    std::cout << "Skip the restore process because there isn't backup.hex file" << std::endl;
    return (0);
  }
  NeuroMemContext ctx{};
  NeuroMemEngine::GetContext(m_device.get(), &ctx);
  std::cout << "file to neuron before context:" << ctx.maxif << std::endl;

  std::cout << "Start restore process from backup.hex file" << std::endl;

  file.read(reinterpret_cast<char *>(&header), sizeof(header));
  if (header[0] != 0x1704) {
    std::cout << "Header error 0: " << header[0] << std::endl;
    file.close();
    return (0);
  }
  if (header[1] != NEURON_SIZE) {
    std::cout << "Neuron size error 1" << std::endl;
    file.close();
    file_name.close();
    return (0);
  }
  if (header[2] > MAX_NEURON) {
    std::cout << "Ncount error" << std::endl;
    file.close();
    file_name.close();
    return (0);
  }
  neuron_count = header[2];
  std::cout << "neuronCount: " << neuron_count << std::endl;
  // auto *neurons = new NeuroMemNeuron[neuronCount];
  std::vector<NeuroMemNeuron> neurons(neuron_count);
  std::vector<uint16_t> buffer(m_neuron_vector_size + 5);
  uint16_t cat = 1;
  for (auto &&neuron : neurons) {
    file.read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint16_t) * (m_neuron_vector_size + 5));
    if (!file.eof()) {
      neuron.ncr = buffer[0];
      neuron.size = m_neuron_vector_size;
      neuron.aif = buffer[1 + m_neuron_vector_size];
      neuron.minif = buffer[2 + m_neuron_vector_size];
      neuron.cat = buffer[3 + m_neuron_vector_size];
      neuron.nid = buffer[4 + m_neuron_vector_size];
      std::cout << "nid= " << neuron.nid << " ,ncr= " << neuron.ncr << " ,cat= " << neuron.cat
                << " ,aif = " << neuron.aif << " , minif = " << neuron.minif << std::endl;
      std::move(std::begin(buffer) + 1, std::begin(buffer) + 1 + m_neuron_vector_size, std::begin(neuron.model));

      // std::cout << "  eob" << std::endl;
      if (neuron.cat > cat) {
        cat = neuron.cat;
        m_category_set_.insert(cat);
      }
    }
  }
  file.close();

  assert(m_device->handle != nullptr);
  NeuroMemEngine::WriteNeurons(m_device.get(), neurons.data(), neuron_count);
  neuron_count = NeuroMemEngine::GetNeuronCount(m_device.get());
  std::cout << "neurons count after write neurons = " << neuron_count << std::endl;
  std::cout << "Restoring names." << std::endl;

  if (file_name.is_open()) {
    std::string temp;
    for (uint16_t i = 0; i <= neuron_count; i++) {
      getline(file_name, temp);
      m_names_[i] = temp;
      if (m_names_[i].length() > 0) {
        std::cout << "user name  " << i << " " << m_names_[i] << std::endl;
      }
    }
    file_name.close();
  }
  std::cout << "Restore process is finished." << std::endl;

  m_loaded_ = true;
  NeuroMemEngine::GetContext(m_device.get(), &ctx);
  std::cout << "file to neuron after context:" << ctx.maxif << std::endl;

#endif
  return neuron_count;
}

void nm_classifier::neurons_to_file() {
#if defined _WIN32
  const uint32_t neuron_count = NeuroMem::NeuroMemEngine::GetNeuronCount(m_device.get());
  if (neuron_count <= 0)
    return;
  std::vector<NeuroMem::NeuroMemNeuron> neurons(neuron_count);
  neurons[0].size = m_neuron_vector_size;
  int size = neurons[0].size;
  NeuroMem::NeuroMemEngine::ReadNeurons(m_device.get(), &neurons[0], neuron_count);

  std::string filename = "backup/backup.hex";
  //   QString filename =
  std::ofstream file;
  QDir dir;
  // ReSharper disable once CppExpressionWithoutSideEffects
  dir.mkdir("backup");
  file.open("backup/backup.hex", std::ios::out | std::ios::binary);
  if (file.is_open()) {
    uint16_t header[4] = {0x1704, 0, 0, 0};
    header[1] = 256;
    header[2] = static_cast<uint16_t>(neuron_count);

    file.write(reinterpret_cast<const char *>(&header), sizeof(header));
    std::vector<uint16_t> buffer(size + 5);
    for (const auto neuron : neurons) {
      buffer[0] = neuron.ncr;
      std::move(std::begin(neuron.model), std::end(neuron.model), std::begin(buffer) + 1);
      //	for (int j = 0; j < size; j++)
      //		buffer[1 + j] = neuron.model[j];
      buffer[1 + size] = neuron.aif;
      buffer[2 + size] = neuron.minif;
      buffer[3 + size] = neuron.cat;
      buffer[4 + size] = static_cast<uint16_t>(neuron.nid);
      file.write(reinterpret_cast<char *>(&buffer[0]), sizeof(uint16_t) * (size + 5));
    }
    file.close();
    //	delete[] buffer;
  }
#endif
}

void nm_classifier::pyramid_reduction(Mat &input, Mat &output, int size) {
  auto pyr = input.clone();
  for (auto i = 0; pyr.rows > size && pyr.cols > size; i++) {
    if ((pyr.rows / 2) >= size || (pyr.cols / 2) >= size) {
      pyrDown(pyr, pyr, Size(pyr.cols / 2, pyr.rows / 2));
    } else
      break;
  }
  resize(pyr, output, Size(size, size), 0, 0, INTER_LINEAR);
}

void nm_classifier::extract_feature_hog(Mat &input, Mat &output) {
  if (input.rows > 32 && input.cols > 32)
    pyramid_reduction(input, input, 32);
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
      if (input.channels() != 1)
        cvtColor(input, input, cv::COLOR_BGR2GRAY);
      extract_feature_hog(input, output);
      break;
    case enum_feature_algorithm::sub_sampling:
      if (input.channels() != 1)
        cvtColor(input, input, cv::COLOR_BGR2GRAY);
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
