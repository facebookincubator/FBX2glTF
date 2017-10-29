/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "PrimitiveData.h"

#include "MaterialData.h"
#include "AccessorData.h"
#include "BufferViewData.h"

PrimitiveData::PrimitiveData(const AccessorData &indices, const MaterialData &material, std::shared_ptr<draco::Mesh> dracoMesh)
    : indices(indices.ix),
      material(material.ix),
      mode(TRIANGLES),
      dracoMesh(dracoMesh),
      dracoBufferView(-1) {}

PrimitiveData::PrimitiveData(const AccessorData &indices, const MaterialData &material)
    : indices(indices.ix),
      material(material.ix),
      mode(TRIANGLES),
      dracoMesh(nullptr),
      dracoBufferView(-1)
{
}

void PrimitiveData::AddAttrib(std::string name, const AccessorData &accessor)
{
    attributes[name] = accessor.ix;
}

void PrimitiveData::NoteDracoBuffer(const BufferViewData &data)
{
    dracoBufferView = data.ix;
}

void PrimitiveData::AddTarget(const AccessorData &positions)
{
    targetPositionAccessors.push_back(positions.ix);
}

void to_json(json &j, const PrimitiveData &d) {
    j = {
        { "material", d.material },
        { "mode", d.mode },
        { "attributes", d.attributes }
    };
    if (d.indices >= 0) {
        j["indices"] = d.indices;
    }
    if (!d.targetPositionAccessors.empty()) {
        json targets {};
        for (int ii = 0; ii < d.targetPositionAccessors.size(); ii ++) {
            targets.push_back({
                { "POSITION", d.targetPositionAccessors[ii] }
            });
        }
        j["targets"] = targets;
    }
    if (!d.dracoAttributes.empty()) {
        j["extensions"] = {
            { KHR_DRACO_MESH_COMPRESSION, {
                { "bufferView", d.dracoBufferView },
                { "attributes", d.dracoAttributes }
            }}
        };
    }
}
