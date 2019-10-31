//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Instance.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/PointLocatorExtension.h"
#include "smtk/model/Resource.h"

#include <random>

namespace smtk
{
namespace model
{

EntityRef Instance::prototype() const
{
  return EntityRefArrangementOps::firstRelation<EntityRef>(*this, INSTANCE_OF);
}

bool Instance::setPrototype(const EntityRef& proto)
{
  EntityRef current = EntityRefArrangementOps::firstRelation<EntityRef>(*this, INSTANCE_OF);
  if (proto == current)
  {
    return false;
  }

  auto rec = this->entityRecord();
  auto arr = this->findArrangement(INSTANCE_OF, 0);
  if (arr)
  {
    rec->unarrange(INSTANCE_OF, 0, true);
  }
  rec->modelResource()->addDualArrangement(
    proto.entity(), this->entity(), INSTANCED_BY, 0, UNDEFINED);
  return true;
}

static void GenerateTabularTessellation(Instance& inst, Tessellation* placements)
{
  if (!inst.hasFloatProperties())
  {
    return;
  }
  const std::vector<double>& posn = inst.floatProperty(smtk::model::Instance::placements);
  int numPosn = static_cast<int>(posn.size() / 3);
  for (int ii = 0; ii < numPosn; ++ii)
  {
    placements->addPoint(&posn[3 * ii]);
  }
}

static void GenerateRandomTessellation(Instance& inst, Tessellation* placements)
{
  if (!inst.hasFloatProperties() || !inst.hasIntegerProperties())
  {
    return;
  }
  const std::vector<long>& numPts = inst.integerProperty("sample size");
  std::vector<long> seed = inst.integerProperty("seed");
  const std::vector<double>& voi = inst.floatProperty("voi");
  if (numPts.size() != 1 || voi.size() != 6 || seed.size() > 1)
  {
    return;
  }
  if (seed.empty())
  {
    // Generate and store a seed
    std::random_device rd;
    inst.setIntegerProperty("seed", static_cast<IntegerList::value_type>(rd()));
    seed = inst.integerProperty("seed");
  }
  int npts = numPts[0];
  unsigned iseed = static_cast<unsigned>(seed[0]);
  std::mt19937 gen(iseed);
  std::uniform_real_distribution<> distribX(voi[0], voi[1]);
  std::uniform_real_distribution<> distribY(voi[2], voi[3]);
  std::uniform_real_distribution<> distribZ(voi[4], voi[5]);
  for (auto ii = 0; ii < npts; ++ii)
  {
    double pt[3] = { distribX(gen), distribY(gen), distribZ(gen) };
    placements->addPoint(pt);
  }
}

static void GenerateRandomOnSurfaceTessellation(Instance& inst, Tessellation* placements)
{
  if (!inst.hasIntegerProperties())
  {
    return;
  }
  const std::vector<long>& numPts = inst.integerProperty("sample size");
  std::vector<long> seed = inst.integerProperty("seed");
  EntityRef sampleSurface = inst.sampleSurface();
  if (numPts.size() != 1 || seed.size() > 1 || !sampleSurface.isValid())
  {
    return;
  }
  if (seed.empty())
  {
    // Generate and store a seed
    std::random_device rd;
    inst.setIntegerProperty("seed", static_cast<IntegerList::value_type>(rd()));
    seed = inst.integerProperty("seed");
  }
  std::size_t npts = numPts[0];
  unsigned iseed = static_cast<unsigned>(seed[0]);

  std::vector<double> points;

  smtk::common::Extension::visitAll(
    [&](const std::string&, smtk::common::Extension::Ptr extension) {
      bool success = false;
      auto snapper = smtk::dynamic_pointer_cast<smtk::model::PointLocatorExtension>(extension);

      if (snapper != nullptr)
      {
        success = snapper->randomPoint(sampleSurface, npts, points, iseed);
        success &= (points.size() == npts);
      }
      return std::make_pair(success, success);
    });

  for (std::size_t i = 0; i < npts; ++i)
  {
    placements->addPoint(&points[3 * i]);
  }
}

static void SnapPlacementsTo(const Instance& inst, const EntityRefs& snaps, Tessellation* tess)
{
  if (snaps.empty() || !snaps.begin()->isValid() || !inst.isValid())
  {
    return;
  }
  else if (snaps.size() > 1)
  {
    smtkWarningMacro(inst.resource()->log(), "Expected a single model entity to snap to, got "
        << snaps.size() << ". "
        << "Ignoring all but first (" << snaps.begin()->name() << ")");
  }

  std::string snapRule;
  if (inst.hasStringProperty("snap rule"))
  {
    snapRule = inst.stringProperty("snap rule")[0];
  }
  if (snapRule.empty())
  {
    smtkWarningMacro(inst.resource()->log(), "No rule for how to perform snap.");
    return;
  }
  bool snapToPoint = false;
  if (snapRule == "snap to point")
  {
    snapToPoint = true;
  }
  smtk::common::Extension::visitAll(
    [&](const std::string&, smtk::common::Extension::Ptr extension) {
      bool success = false;
      auto snapper = smtk::dynamic_pointer_cast<smtk::model::PointLocatorExtension>(extension);

      if (snapper != nullptr)
      {
        success =
          snapper->closestPointOn(*snaps.begin(), tess->coords(), tess->coords(), snapToPoint);
      }
      return std::make_pair(success, success);
    });
}

static void ComputeBounds(Tessellation* tess, const std::vector<double>& pbox, double bbox[6])
{
  // Compute bbox of points in tessellation
  (void)tess;
  (void)bbox;
  // Add pbox min to bbox min, pbox max to bbox max
  (void)pbox;
}

// TODO: This is hardcoded for now, but should allow for lambdas to be registered
// as rules and invoked to generate the tessellation.
Tessellation* Instance::generateTessellation()
{
  std::string rule;
  EntityRef proto;
  if (!this->isValid() || (rule = this->rule()).empty() || !(proto = this->prototype()).isValid())
  {
    return nullptr;
  }

  Tessellation* tess = this->resetTessellation();
  if (rule == "tabular")
  {
    GenerateTabularTessellation(*this, tess);
  }
  else if (rule == "uniform random")
  {
    GenerateRandomTessellation(*this, tess);
  }
  else if (rule == "uniform random on surface")
  {
    GenerateRandomOnSurfaceTessellation(*this, tess);
  }

  EntityRefs snaps = this->snapEntities();
  SnapPlacementsTo(*this, snaps, tess);

  std::vector<double> pbox = proto.boundingBox();
  double bbox[6];
  ComputeBounds(tess, pbox, bbox);
  this->setBoundingBox(bbox);
  return tess;
}

std::string Instance::rule() const
{
  static const std::string rule = "rule";

  auto comp = this->component();
  if (!comp)
  {
    return "empty";
  }

  auto stringProperties = comp->properties().get<std::vector<std::string> >();
  return (!stringProperties.contains(rule) || stringProperties.at(rule).empty())
    ? "empty"
    : stringProperties.at(rule)[0];
}

bool Instance::setRule(const std::string& nextRule)
{
  // Only accept valid rules:
  if (nextRule != "tabular" && nextRule != "uniform random" &&
    nextRule != "uniform random on surface")
  {
    return false;
  }
  // Only place rules on instance entities:
  if (!this->isValid())
  {
    return false;
  }
  // Only change the rule if we need to:
  std::string currRule = this->rule();
  if (currRule != nextRule)
  {
    this->setStringProperty("rule", nextRule);
    return true;
  }
  return false;
}

namespace
{
static const std::string sampleSurfaceStr = "sample surface";
}

bool Instance::setSampleSurface(const EntityRef& surface)
{
  if (!this->isValid() || !surface.isValid())
  {
    return false;
  }
  auto comp = this->component();
  if (!comp)
  {
    return false;
  }
  comp->properties().get<smtk::common::UUID>()[sampleSurfaceStr] = surface.entity();
  return true;
}

EntityRef Instance::sampleSurface() const
{
  EntityRef result;
  // TODO: This is a total hack for now. We should use Arrangements but
  // there's not really a good one for this use case.
  auto comp = this->component();
  if (!comp)
  {
    return result;
  }

  smtk::common::UUID id = comp->properties().get<smtk::common::UUID>().at(sampleSurfaceStr);
  smtk::model::Resource::Ptr resource = this->resource();
  return EntityRef(resource, id);
}

EntityRefs Instance::snapEntities() const
{
  EntityRefs result;
  // TODO: This is a total hack for now. We should use Arrangements but
  // there's not really a good one for this use case.
  auto comp = this->component();
  if (!comp)
  {
    return result;
  }

  auto intProperties = comp->properties().get<IntegerList>();
  static const std::string snapToEntitiesStr = "snap to entities";
  if (!intProperties.contains(snapToEntitiesStr))
  {
    return result;
  }

  const IntegerList& snapToEntities = intProperties.at(snapToEntitiesStr);
  smtk::model::Resource::Ptr resource = this->resource();
  int numEnts = static_cast<int>(snapToEntities.size() / 8);
  IntegerList::const_iterator pit = snapToEntities.begin();
  for (int ii = 0; ii < numEnts; ++ii)
  {
    smtk::common::UUID::value_type data[smtk::common::UUID::SIZE];
    for (int jj = 0; jj < 8; ++jj, ++pit)
    {
      data[2 * jj] = *pit % 256;
      data[2 * jj + 1] = (*pit / 256) % 256;
    }
    result.insert(EntityRef(resource, smtk::common::UUID(data, data + smtk::common::UUID::SIZE)));
  }
  return result;
}

bool Instance::addSnapEntity(const EntityRef& snapTo)
{
  EntityRefs curSnaps = this->snapEntities();
  if (curSnaps.insert(snapTo).second)
  {
    this->setSnapEntities(curSnaps);
    return true;
  }
  return false;
}

bool Instance::removeSnapEntity(const EntityRef& snapTo)
{
  EntityRefs curSnaps = this->snapEntities();
  if (curSnaps.erase(snapTo) > 0)
  {
    this->setSnapEntities(curSnaps);
    return true;
  }
  return false;
}

bool Instance::setSnapEntity(const EntityRef& snapTo)
{
  EntityRefs snapSet;
  snapSet.insert(snapTo);
  return this->setSnapEntities(snapSet);
}

bool Instance::setSnapEntities(const EntityRefs& snapTo)
{
  if (!this->isValid())
  {
    return false;
  }
  if (!this->hasIntegerProperties() && snapTo.empty())
  {
    return false;
  }
  auto comp = this->component();
  if (!comp)
  {
    return false;
  }
  auto intProperties = comp->properties().get<IntegerList>();
  static const std::string snapToEntitiesStr = "snap to entities";
  if (snapTo.empty())
  {
    if (!intProperties.contains(snapToEntitiesStr))
    {
      return false;
    }
    else
    {
      intProperties.erase(snapToEntitiesStr);
      return true;
    }
  }

  IntegerList replacement;
  replacement.resize(8 * snapTo.size());
  int numEnts = static_cast<int>(snapTo.size());
  EntityRefs::const_iterator sit = snapTo.begin();
  IntegerList::iterator rit = replacement.begin();
  for (int ii = 0; ii < numEnts; ++ii, ++sit)
  {
    smtk::common::UUID uid(sit->entity());
    smtk::common::UUID::const_iterator uit = uid.begin();
    for (int jj = 0; jj < 8; ++jj, ++rit)
    {
      *rit = uit[2 * jj] + 256 * uit[2 * jj + 1];
    }
  }
  if (!intProperties.contains(snapToEntitiesStr))
  {
    intProperties[snapToEntitiesStr] = replacement;
  }
  else
  {
    if (intProperties[snapToEntitiesStr] == replacement)
    {
      return false;
    }
    intProperties[snapToEntitiesStr] = replacement;
  }
  return true;
}

std::size_t Instance::numberOfPlacements()
{
  const Tessellation* tess = this->hasTessellation();
  return tess ? tess->coords().size() / 3 : 0;
}

bool Instance::isClone() const
{
  // We could also check whether this instance has
  // a SUBSET_OF relation to another instance:
  return this->hasIntegerProperty(Instance::subset);
}

bool Instance::checkMergeable(const Instance& other) const
{
  // TODO: Expand this to include checks on properties like color, mask, scaling, ...
  bool protoMatch = (other.prototype() == this->prototype());
  bool snapsMatch = (other.snapEntities() == this->snapEntities());
  return protoMatch && snapsMatch;
  /*
    (other.prototype() == this->prototype()) &&
    (other.snapEntities() == this->snapEntities());
    */
}

bool Instance::mergeInternal(const Instance& other)
{
  // I. Determine sizes (placements) of instances involved
  auto& places = this->floatProperty(Instance::placements);
  const Tessellation* otherTess = other.hasTessellation();
  const auto& otherCoords = otherTess->coords();
  std::size_t numOtherPlacements = otherCoords.size() / 3;
  std::size_t numThisPlacements = places.size() / 3;

  // II. Determine which optional tabular properties are present
  //     on this instance and other.
  bool hasOrientation = this->hasFloatProperty(Instance::orientations) &&
    !this->floatProperty(Instance::orientations).empty();
  bool hasScales =
    this->hasFloatProperty(Instance::scales) && !this->floatProperty(Instance::scales).empty();
  bool hasMasks =
    this->hasFloatProperty(Instance::masks) && !this->floatProperty(Instance::masks).empty();
  bool hasColors =
    this->hasFloatProperty(Instance::colors) && !this->floatProperty(Instance::colors).empty();
  bool otherHasOrientation = other.hasFloatProperty(Instance::orientations) &&
    !other.floatProperty(Instance::orientations).empty();
  bool otherHasScales =
    other.hasFloatProperty(Instance::scales) && !other.floatProperty(Instance::scales).empty();
  bool otherHasMasks =
    other.hasFloatProperty(Instance::masks) && !other.floatProperty(Instance::masks).empty();
  bool otherHasColors =
    other.hasFloatProperty(Instance::colors) && !other.floatProperty(Instance::colors).empty();

  // III. Append \a other's tessellation coordinates to this instance's placements.
  //
  // Note that this works out because _this_ is required to have its rule
  // set to "tabular" while the _other_ instance may not but will always
  // have coordinates in its tessellation. The other fields (orientation,
  // masks, etc) are only present for tabular rules.
  places.insert(places.end(), otherCoords.begin(), otherCoords.end());

  // IV. Now pad this instance's properties as required to support those
  //     which are present on the other instance but not this one:
  if (!hasOrientation && otherHasOrientation)
  {
    // Pad this instance with default orientation data.
    this->floatProperty(Instance::orientations).resize(numThisPlacements * 3, 0.0);
  }
  if (!hasScales && otherHasScales)
  {
    // Pad this instance with default scaling data.
    this->floatProperty(Instance::scales).resize(numThisPlacements * 3, 1.0);
  }
  if (!hasMasks && otherHasMasks)
  {
    // Pad this instance with default masking data.
    this->floatProperty(Instance::masks).resize(numThisPlacements, 0.0);
  }
  if (!hasColors && otherHasColors)
  {
    // Pad this instance with default scaling data.
    this->floatProperty(Instance::colors).resize(numThisPlacements * 4, 1.0);
  }

  // V. Finally, append properties present from the other instance
  //    or pad if they are needed but not present.
  if (otherHasOrientation)
  {
    const FloatList& otherOrient(other.floatProperty(Instance::orientations));
    FloatList& thisOrient = this->floatProperty(Instance::orientations);
    thisOrient.insert(thisOrient.end(), otherOrient.begin(), otherOrient.end());
  }
  else if (hasOrientation)
  {
    FloatList otherOrient(numOtherPlacements * 3, 0.0);
    FloatList& thisOrient = this->floatProperty(Instance::orientations);
    thisOrient.insert(thisOrient.end(), otherOrient.begin(), otherOrient.end());
  }

  return true;
}

} // namespace model
} // namespace smtk
