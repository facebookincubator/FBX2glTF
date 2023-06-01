// Copyright 2016 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file contains misc functions that are needed by several mesh related
// algorithms.

#ifndef DRACO_MESH_MESH_MISC_FUNCTIONS_H_
#define DRACO_MESH_MESH_MISC_FUNCTIONS_H_

#include "draco/mesh/corner_table.h"
#include "draco/mesh/mesh.h"

// The file contains functions that use both Mesh and CornerTable as inputs.
namespace draco {

// Creates a CornerTable from the position attribute of |mesh|. Returns nullptr
// on error.
std::unique_ptr<CornerTable> CreateCornerTableFromPositionAttribute(
    const Mesh *mesh);

// Creates a CornerTable from all attributes of |mesh|. Boundaries are
// automatically introduced on all attribute seams. Returns nullptr on error.
std::unique_ptr<CornerTable> CreateCornerTableFromAllAttributes(
    const Mesh *mesh);

// Returns true when the given corner lies opposite to an attribute seam.
inline bool IsCornerOppositeToAttributeSeam(CornerIndex ci,
                                            const PointAttribute &att,
                                            const Mesh &mesh,
                                            const CornerTable &ct) {
  const CornerIndex opp_ci = ct.Opposite(ci);
  if (opp_ci == kInvalidCornerIndex)
    return false;  // No opposite corner == no attribute seam.
  // Compare attribute value indices on both ends of the opposite edge.
  CornerIndex c0 = ct.Next(ci);
  CornerIndex c1 = ct.Previous(opp_ci);
  if (att.mapped_index(mesh.CornerToPointId(c0)) !=
      att.mapped_index(mesh.CornerToPointId(c1)))
    return true;
  c0 = ct.Previous(ci);
  c1 = ct.Next(opp_ci);
  if (att.mapped_index(mesh.CornerToPointId(c0)) !=
      att.mapped_index(mesh.CornerToPointId(c1)))
    return true;
  return false;
}

}  // namespace draco

#endif  // DRACO_MESH_MESH_MISC_FUNCTIONS_H_
