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

void PrimitiveData::AddTarget(const AccessorData *positions, const AccessorData *normals, const AccessorData *tangents)
{
    targetAccessors.push_back(std::make_tuple(
        positions->ix,
        normals ? normals->ix : -1,
        tangents ? tangents ->ix : -1
    ));
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
    if (!d.targetAccessors.empty()) {
        json targets {};
        int pIx, nIx, tIx;
        for (auto accessor : d.targetAccessors) {
            std::tie(pIx, nIx, tIx) = accessor;
            json target {};
            if (pIx >= 0) { target["POSITION"] = pIx; }
            if (nIx >= 0) { target["NORMAL"] = nIx; }
            if (tIx >= 0) { target["TANGENT"] = tIx; }
            targets.push_back(target);
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
