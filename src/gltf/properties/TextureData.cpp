/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "TextureData.hpp"

#include "ImageData.hpp"
#include "SamplerData.hpp"

TextureData::TextureData(std::string name, const SamplerData& sampler, const ImageData& source)
    : Holdable(), name(std::move(name)), sampler(sampler.ix), source(source.ix) {}

json TextureData::serialize() const {
  return {{"name", name}, {"sampler", sampler}, {"source", source}};
}
