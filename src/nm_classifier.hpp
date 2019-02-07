/*
 * nm_face_recognizer.h
 * Copyright (c) 2018, nepes inc, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.* this list of conditions and the following disclaimer.
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
 */

#pragma once

#include "NeuroMemEngine.hpp"
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <set>
#include <memory>

enum class enum_feature_algorithm { default_, hog, sub_sampling };

class nm_classifier {

private:
  const static int pca_component = 15;
  const static int NEURON_SIZE = 256;
  const static int MAX_NEURON = 576;
  const static int UNKNOWN = 65535;

  uint16_t m_neuron_vector_size = 256;
#ifdef __linux__
  std::unique_ptr<neuromem::nm_device> m_device;
#else
  std::unique_ptr<NeuroMem::NeuroMemDevice> m_device;
#endif
  std::string m_tracker_net_model_configuration;
  std::string m_tracker_net_model_binary;

  cv::Ptr<cv::HOGDescriptor> m_hog;

  // dnn params
  int in_width_ = 300;
  int in_height_ = 300;
  const double in_scale_factor_ = 1.0;
  std::map<uint16_t, std::string> m_names_;
  std::set<uint16_t> m_category_set;
  bool m_loaded_ = false;

  bool learning_mode_ = false; // 0: recognize, 1: learn, 90: end-of-program
  uint16_t m_menu = 0;
  uint16_t m_maxif = 0;
  uint16_t m_minif = 0;
  std::vector<cv::Mat> m_features;

private:
  enum_feature_algorithm m_algorithm = enum_feature_algorithm::default_;

public:
 #ifdef __linux__
    void learn(neuromem::nm_learn_req &req);
    bool classify(neuromem::nm_classify_req &req);
 #else
    void learn(NeuroMem::NeuroMemLearnReq &req);
    bool classify(NeuroMem::NeuroMemClassifyReq &req);
#endif
  bool is_loaded_from_file() const;
  enum_feature_algorithm get_feature_algorithm() const;
  uint16_t get_neuron_vector_size() const;

    void set_context(uint16_t context, uint16_t norm, uint16_t minif, uint16_t maxif);
  void pyramid_reduction(cv::Mat &input, cv::Mat &output, int size);
  void set_feature_algorithm(enum_feature_algorithm algorithm);

  nm_classifier();
  void init(uint16_t maxif, uint16_t minif);
  ~nm_classifier();
  bool classify(cv::Mat &in);
  void learn(cv::Mat &in, int cat =-1);
  uint32_t file_to_neurons();
  void neurons_to_file();

  void extract_feature_vector(cv::Mat input, std::vector<uint8_t> &v);
  void extract_feature_hog(cv::Mat &input, cv::Mat &output);

  void set_learning_mode(bool mode);
  uint16_t maxif() const;
  uint16_t minif() const;
  void set_maxif(uint16_t max);
  void set_minif(uint16_t min);
};
