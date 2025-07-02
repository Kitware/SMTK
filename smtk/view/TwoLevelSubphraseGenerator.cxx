//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/TwoLevelSubphraseGenerator.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/SubphraseGenerator.txx"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

namespace smtk
{
namespace view
{

TwoLevelSubphraseGenerator::TwoLevelSubphraseGenerator() = default;

TwoLevelSubphraseGenerator::~TwoLevelSubphraseGenerator() = default;

DescriptivePhrases TwoLevelSubphraseGenerator::subphrases(DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  if (src)
  {
    auto comp = src->relatedComponent();
    if (!comp)
    {
      auto rsrc = src->relatedResource();
      if (rsrc)
      {
        TwoLevelSubphraseGenerator::childrenOfResource(src, rsrc, result);
      }
    }
    else
    {
      // In the 2-level view, components do not have children; they are leaves.
    }
  }
  return result;
}

int FindPhraseByTitle(const std::string& title, const DescriptivePhrases& phrases)
{
  int result = 0;
  for (const auto& phrase : phrases)
  {
    if (phrase->title() == title)
    {
      return result;
    }
    ++result;
  }
  return -1;
}

int FindPhraseLocation(DescriptivePhrase::Ptr& phr, const DescriptivePhrases& siblings)
{
  // auto it = std::upper_bound(siblings.begin(), siblings.end(), phr,
  // DescriptivePhrase::compareByTypeThenTitle);
  auto it =
    std::upper_bound(siblings.begin(), siblings.end(), phr, DescriptivePhrase::compareByTitle);
  int locn = static_cast<int>(it == siblings.end() ? siblings.size() : it - siblings.end());
  return locn;
}

bool TwoLevelSubphraseGenerator::findSortedLocation(
  Path& pathInOut,
  smtk::model::EntityPtr entity,
  DescriptivePhrase::Ptr& phr,
  const DescriptivePhrase::Ptr& parent) const
{
  if (!entity || !parent || !parent->areSubphrasesBuilt())
  {
    // If the user has not opened the parent phrase, do not
    // add to it; when subphrases are generated later (on demand)
    // they should include \a attr.
    return false;
  }
  smtk::model::BitFlags entityType = entity->entityFlags() & smtk::model::ENTITY_MASK;
  int rsrcIdx = pathInOut[0];
  switch (entityType)
  {
    case smtk::model::AUX_GEOM_ENTITY:
    case smtk::model::MODEL_ENTITY:
    case smtk::model::GROUP_ENTITY:
    case smtk::model::CELL_ENTITY:
    {
      auto phrases = parent->subphrases();
      // We separate cells out by dimension, but not models, groups, aux geom, etc.:
      smtk::model::BitFlags flags = entity->isCellEntity() ? entity->entityFlags() : entityType;
      int ii = FindPhraseByTitle(smtk::model::Entity::flagSummary(flags, 1), phrases);
      if (ii >= 0)
      {
        int jj = FindPhraseLocation(phr, phrases[ii]->subphrases());
        phr->reparent(phrases[ii]);
        pathInOut = Path{ rsrcIdx, ii, jj };
        return true;
      }
    }
    break;
    default:
      break;
  }

  return false;
}

void TwoLevelSubphraseGenerator::childrenOfResource(
  DescriptivePhrase::Ptr src,
  smtk::resource::ResourcePtr rsrc,
  DescriptivePhrases& result)
{
  auto modelRsrc = dynamic_pointer_cast<smtk::model::Resource>(rsrc);
  auto attrRsrc = dynamic_pointer_cast<smtk::attribute::Resource>(rsrc);
  if (modelRsrc)
  {
    constexpr int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
      static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);
    auto models =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);
    if (!models.empty())
    {
      this->filterModelEntityPhraseCandidates(models);
      auto list = this->addModelEntityPhrases(models, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::MODEL_ENTITY, 1));
    }

    auto groups =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Groups>(smtk::model::GROUP_ENTITY, false);
    if (!groups.empty())
    {
      this->filterModelEntityPhraseCandidates(groups);
      auto list = this->addModelEntityPhrases(groups, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::GROUP_ENTITY, 1));
      // list->setCustomTitle("groups");
    }

    auto auxGeoms = modelRsrc->entitiesMatchingFlagsAs<smtk::model::AuxiliaryGeometries>(
      smtk::model::AUX_GEOM_ENTITY, false);
    if (!auxGeoms.empty())
    {
      this->filterModelEntityPhraseCandidates(auxGeoms);
      auto list = this->addModelEntityPhrases(auxGeoms, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::AUX_GEOM_ENTITY, 1));
      // list->setCustomTitle("auxiliary geometry");
    }

    auto volumes =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Volumes>(smtk::model::VOLUME, false);
    if (!volumes.empty())
    {
      this->filterModelEntityPhraseCandidates(volumes);
      auto list = this->addModelEntityPhrases(volumes, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::VOLUME, 1));
      // list->setCustomTitle("volumes");
    }

    auto faces = modelRsrc->entitiesMatchingFlagsAs<smtk::model::Faces>(smtk::model::FACE, false);
    if (!faces.empty())
    {
      this->filterModelEntityPhraseCandidates(faces);
      auto list = this->addModelEntityPhrases(faces, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::FACE, 1));
      // list->setCustomTitle("faces");
    }

    auto edges = modelRsrc->entitiesMatchingFlagsAs<smtk::model::Edges>(smtk::model::EDGE, false);
    if (!edges.empty())
    {
      this->filterModelEntityPhraseCandidates(edges);
      auto list = this->addModelEntityPhrases(edges, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::EDGE, 1));
      // list->setCustomTitle("edges");
    }

    auto vertices =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Vertices>(smtk::model::VERTEX, false);
    if (!vertices.empty())
    {
      this->filterModelEntityPhraseCandidates(vertices);
      auto list = this->addModelEntityPhrases(vertices, src, 0, result, mutability);
      list->setCustomTitle(smtk::model::Entity::flagSummary(smtk::model::VERTEX, 1));
      // list->setCustomTitle("vertices");
    }
  }
  else if (attrRsrc)
  {
    std::vector<smtk::attribute::AttributePtr> attrs;
    attrRsrc->attributes(attrs);
    for (const auto& attr : attrs)
    {
      if (attr)
      {
        result.push_back(ComponentPhraseContent::createPhrase(attr, 0, src));
      }
    }
    std::sort(result.begin(), result.end(), DescriptivePhrase::compareByTypeThenTitle);
  }
}
} // namespace view
} // namespace smtk
