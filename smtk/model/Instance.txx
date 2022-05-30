//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_Instance_txx
#define smtk_model_Instance_txx

#include "smtk/model/Resource.h"
#include "smtk/model/Resource.txx"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/ShellEntity.txx" // for deleteEntities()
#include "smtk/model/Tessellation.h"

#include <numeric>
#include <sstream>

namespace smtk
{
namespace model
{

template<typename I>
Instance Instance::clonePlacements(I begin, I end, bool relate)
{
  auto rsrc = this->resource();
  if (!rsrc)
  {
    return Instance();
  }

  std::ostringstream name;
  const auto& sourceTess = this->hasTessellation();
  name << this->name();
  if (this->name().rfind("subset") == std::string::npos)
  {
    name << " subset";
  }
  if (sourceTess)
  {
    name << " (" << (end - begin) << "/" << sourceTess->coords().size() / 3 << ")";
  }
  Instance tmp = rsrc->addInstance(this->prototype());
  tmp.setRule("tabular");
  tmp.setName(name.str());

  const auto& sourceCoords = sourceTess->coords();
  FloatList::const_iterator crit = sourceCoords.begin();
  FloatList::const_iterator orit;
  FloatList::const_iterator scit;
  FloatList::const_iterator coit;
  FloatList::const_iterator mkit;
  IntegerList subsetIndices;
  subsetIndices.reserve(end - begin);
  bool hasOrientation = this->hasFloatProperty(Instance::orientations);
  bool hasScales = this->hasFloatProperty(Instance::scales);
  bool hasMasks = this->hasFloatProperty(Instance::masks);
  bool hasColors = this->hasFloatProperty(Instance::colors);
  FloatList* tmpOrient = nullptr;
  FloatList* tmpScales = nullptr;
  FloatList* tmpColors = nullptr;
  FloatList* tmpMasks = nullptr;
  if (hasOrientation)
  {
    orit = this->floatProperty(Instance::orientations).begin();
    tmpOrient = &tmp.floatProperty(Instance::orientations);
  }
  if (hasScales)
  {
    scit = this->floatProperty(Instance::scales).begin();
    tmpScales = &tmp.floatProperty(Instance::scales);
  }
  if (hasMasks)
  {
    mkit = this->floatProperty(Instance::masks).begin();
    tmpMasks = &tmp.floatProperty(Instance::masks);
  }
  if (hasColors)
  {
    coit = this->floatProperty(Instance::colors).begin();
    tmpColors = &tmp.floatProperty(Instance::colors);
  }
  auto& places = tmp.floatProperty(Instance::placements);
  for (auto ii = begin; ii != end; ++ii)
  {
    auto pointId = *ii;
    subsetIndices.push_back(pointId);
    places.insert(places.end(), crit + 3 * pointId, crit + 3 * (pointId + 1));
    if (hasOrientation)
    {
      tmpOrient->insert(tmpOrient->end(), orit + 3 * pointId, orit + 3 * (pointId + 1));
    }
    if (hasScales)
    {
      tmpScales->insert(tmpScales->end(), scit + 3 * pointId, scit + 3 * (pointId + 1));
    }
    if (hasColors)
    {
      tmpColors->insert(tmpColors->end(), coit + 4 * pointId, coit + 4 * (pointId + 1));
    }
    if (hasMasks)
    {
      tmpMasks->insert(tmpMasks->end(), mkit + pointId, mkit + (pointId + 1));
    }
  }
  if (relate)
  {
    tmp.setIntegerProperty(Instance::subset, subsetIndices);
    auto tmpRec = tmp.entityRecord();
    tmpRec->modelResource()->addDualArrangement(
      this->entity(), tmp.entity(), SUPERSET_OF, 0, UNDEFINED);
  }
  return tmp;
}

template<typename Container>
Container Instance::divide(bool merge, Container* clonesIncluded)
{
  Container result;
  std::vector<std::vector<int>> output;
  // I. Iterate over child clones and uniquely assign each placement to an output.
  {
    std::set<int> taken;
    Container tmp;
    this->divideMapInternal(*this, taken, merge, output, clonesIncluded);
    auto* tess = this->generateTessellation();
    std::size_t size = static_cast<std::size_t>(tess->coords().size() / 3);

    std::vector<int> untaken;
    for (std::size_t placement = 0; placement < size; ++placement)
    {
      auto it = taken.find(static_cast<int>(placement));
      if (it == taken.end())
      {
        untaken.push_back(static_cast<int>(placement));
      }
    }
    output.push_back(untaken);
  }
  // II. Accumulate all the placements for each output (including the
  //     instance that will replace this intance
  for (auto& entry : output)
  {
    Instance div = this->clonePlacements(entry.begin(), entry.end(), false);
    result.insert(result.end(), div);
  }
  if (clonesIncluded)
  {
    clonesIncluded->insert(clonesIncluded->end(), *this);
  }

  return result;
}

template<typename Container>
Instance Instance::merge(const Container& instances)
{
  Instance result;
  if (instances.empty())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Cannot merge empty instance list.");
    return result;
  }
  if (instances.size() == 1)
  {
    result = *instances.begin();
    return result;
  }
  auto it = instances.begin();
  auto resource = it->resource();
  Instance archetype = *it;
  Tessellation* tess = archetype.generateTessellation();
  if (!tess)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Cannot merge instances because first entry has no tessellation.");
    return result;
  }
  // Clone the first entry, then traverse the inputs,
  // checking them as we go.
  std::vector<int> all(tess->coords().size() / 3);
  std::iota(all.begin(), all.end(), 0);
  result = archetype.clonePlacements(all.begin(), all.end(), /* relate */ false);
  result.setName("merged");
  bool ok = true;
  for (++it; it != instances.end(); ++it)
  {
    if (!(ok = result.checkMergeable(*it)))
    {
      break;
    }
  }
  if (!ok)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot merge instances because some are incompatible.");
    // Remove cloned result;
    std::set<Instance> kill;
    kill.insert(result);
    smtk::model::EntityRefArray dummy;
    resource->deleteEntities(kill, dummy, dummy, /* debug */ false);
    result = Instance();
    return result;
  }
  it = instances.begin();
  ok = true;
  for (++it; it != instances.end(); ++it)
  {
    ok &= result.mergeInternal(*it);
  }
  if (!ok)
  {
    // Remove cloned result;
    std::set<Instance> kill;
    kill.insert(result);
    smtk::model::EntityRefArray dummy;
    resource->deleteEntities(kill, dummy, dummy, /* debug */ false);
    result = Instance();
  }
  return result;
}

template<typename Container>
void Instance::divideMapInternal(
  Instance& clone,
  std::set<int>& taken,
  bool merge,
  std::vector<std::vector<int>>& output,
  Container* clonesIncluded)
{
  InstanceEntities children;
  EntityRefArrangementOps::appendAllRelations(clone, SUPERSET_OF, children);
  for (auto child : children)
  {
    if (child.isValid())
    {
      this->divideMapInternal(child, taken, merge, output, clonesIncluded);
    }
  }
  std::vector<int> storage;
  std::vector<int>* trueSubset = (merge && !output.empty() ? output.data() : &storage);
  if (clone.isClone())
  {
    if (clonesIncluded)
    {
      clonesIncluded->insert(clonesIncluded->end(), clone);
    }
    const auto& subsetIndices = clone.integerProperty("subset");
    for (auto entry : subsetIndices)
    {
      if (taken.find(entry) == taken.end())
      {
        trueSubset->push_back(entry);
        taken.insert(entry);
      }
    }
  }
  if (!trueSubset->empty() && (!merge || output.empty()))
  {
    output.push_back(*trueSubset);
  }
}

} // namespace model
} // namespace smtk

#endif // smtk_model_Instance_txx
