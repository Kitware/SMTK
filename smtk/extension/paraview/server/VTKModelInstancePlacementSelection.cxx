//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/VTKModelInstancePlacementSelection.h"
#include "smtk/extension/paraview/server/VTKModelInstancePlacementSelection_xml.h"

#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/view/Selection.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Instance.txx"
#include "smtk/model/Resource.h"

#include "smtk/model/operators/Delete.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/GarbageCollector.h"
#include "smtk/resource/Manager.h"

#include "smtk/io/Logger.h"

#include "vtkCellData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

namespace smtk
{
namespace view
{

VTKModelInstancePlacementSelection::VTKModelInstancePlacementSelection() = default;

VTKModelInstancePlacementSelection::~VTKModelInstancePlacementSelection() = default;

smtk::model::EntityPtr VTKModelInstancePlacementSelection::temporaryInstance(
  const smtk::model::EntityPtr& sourceInstance, vtkIdTypeArray* sourcePlacements)
{
  using Instance = smtk::model::Instance;

  smtk::model::EntityPtr ent;
  if (!sourceInstance || !sourcePlacements)
  {
    return ent;
  }

  Instance source(sourceInstance);

  // Copy the source placements into a new instance:
  vtkIdType np = sourcePlacements->GetNumberOfTuples();
  vtkIdType* pp = sourcePlacements->GetPointer(0);
  Instance tmp = source.clonePlacements(pp, pp + np);

  if (tmp.isValid())
  {
    // Name the new instance:
    tmp.setName(source.name() + " selection");

    // Delete the new instance when it is no longer in use:
    ent = tmp.entityRecord();
    auto deleter = this->manager()->create<smtk::model::Delete>();
    deleter->parameters()->associate(ent);
    auto resource = sourceInstance->modelResource();
    resource->manager()->garbageCollector()->add(deleter);
  }

  return ent;
}

bool VTKModelInstancePlacementSelection::transcribePlacementSelection(Result& result)
{
  bool didModify = false;
  if (this->selectingBlocks())
  {
    // We only pay attention to point/cell selections
    return didModify;
  }

  auto assoc = this->parameters()->associations();
  auto resource = assoc->valueAs<smtk::model::Resource>();
  if (!resource)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No associated resource for model selection.");
    return didModify;
  }
  auto resourceManager = resource->manager();
  if (!resourceManager)
  {
    // We require the resource to be managed for proper garbage selection.
    smtkErrorMacro(smtk::io::Logger::instance(), "Associated resource is unmanaged.");
    return didModify;
  }
  auto operationManager = this->manager();
  if (!operationManager)
  {
    // We require ourself to be managed for proper garbage selection.
    smtkErrorMacro(smtk::io::Logger::instance(), "Selection operation is unmanaged.");
    return didModify;
  }

  auto selnMgr = this->smtkSelection();
  auto selnBlock = this->vtkSelection();
  auto mbdsThing = this->vtkData();
  unsigned nn = selnBlock ? selnBlock->GetNumberOfNodes() : 0;
  std::map<vtkIdType, vtkIdTypeArray*> glyphPlacements;
  vtkSMTKResourceRepresentation* source = nullptr;
  for (unsigned ii = 0; ii < nn; ++ii)
  {
    auto selnNode = selnBlock->GetNode(ii);
    if (!selnNode)
    {
      continue;
    }
    vtkIdType propId = -1;
    vtkIdType compositeIndex = -1;
    auto sp = selnNode->GetProperties();
    if (sp->Has(vtkSelectionNode::PROP_ID()))
    {
      propId = sp->Get(vtkSelectionNode::PROP_ID());
      // std::cout << "  PROP_ID " << propId << "\n";
    }
#ifdef SMTK_DEBUG_SELECTION
    if (sp->Has(vtkSelectionNode::SOURCE_ID()))
    {
      std::cout << "  SOURCE_ID " << sp->Get(vtkSelectionNode::SOURCE_ID()) << "\n";
    }
#endif
    if (sp->Has(vtkSelectionNode::COMPOSITE_INDEX()))
    {
      compositeIndex = sp->Get(vtkSelectionNode::COMPOSITE_INDEX());
      // std::cout << "  COMPOSITE_INDEX " << compositeIndex << "\n";
    }
    if (sp->Has(vtkSelectionNode::SOURCE()))
    {
      source = vtkSMTKResourceRepresentation::SafeDownCast(sp->Get(vtkSelectionNode::SOURCE()));
      // std::cout << "  SOURCE " << source << "\n";
    }
    if (!source || source->GetResource() != resource || compositeIndex < 0)
    {
      continue;
    }
    if (propId == source->GetGlyphEntitiesActorPickId() ||
      propId == source->GetSelectedGlyphEntitiesActorPickId())
    {
      vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(selnNode->GetSelectionList());
      glyphPlacements.insert(std::make_pair(compositeIndex, ids));
    }
  }
  std::set<smtk::resource::ComponentPtr> seln;
  if (!glyphPlacements.empty())
  {
    auto created = result->findComponent("created");
    auto instances = vtkMultiBlockDataSet::SafeDownCast(
      mbdsThing->GetBlock(vtkResourceMultiBlockSource::BlockId::Value::Instances));
    if (instances)
    {
      auto mit = instances->NewIterator();
      for (mit->InitTraversal(); !mit->IsDoneWithTraversal(); mit->GoToNextItem())
      {
        auto git = glyphPlacements.find(mit->GetCurrentFlatIndex());
        if (git != glyphPlacements.end())
        {
          smtk::model::EntityPtr ent;
          auto cmp = resource->find(
            vtkResourceMultiBlockSource::GetDataObjectUUID(mit->GetCurrentMetaData()));
          if (cmp && (ent = dynamic_pointer_cast<smtk::model::Entity>(cmp)) && ent->isInstance())
          {
            auto tmp = this->temporaryInstance(ent, git->second);
            seln.insert(seln.end(), tmp);
            created->appendValue(tmp);
          }
          else
          {
            std::cout << "    component " << (cmp ? cmp->name() : "(null)") << " @ "
                      << mit->GetCurrentFlatIndex() << " null or unexpected.\n";
          }
        }
      }
      mit->Delete();
    }
  }

  SelectionAction action = this->modifierAction();

  if (!seln.empty())
  {
    // Always do a filtered-add to the selection; the behavior will have reset the selection
    // so we can rebuild it in its entirety from the VTK selection via successive operations
    // on each of the resources involved. Also, never notify observers since this is an
    // intermediate result.
    didModify = selnMgr->modifySelection(seln, m_smtkSelectionSource, m_smtkSelectionValue, action,
      /* bitwise */ true, /* notify */ false);
  }

  return didModify;
}

VTKModelInstancePlacementSelection::Result VTKModelInstancePlacementSelection::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  bool worked = this->transcribePlacementSelection(result);
  if (!worked)
  {
    result->findInt("outcome")->setValue(
      static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
  }
  return result;
}

const char* VTKModelInstancePlacementSelection::xmlDescription() const
{
  return VTKModelInstancePlacementSelection_xml;
}

} //namespace view
} // namespace smtk
