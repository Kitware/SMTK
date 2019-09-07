//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/RespondToVTKSelection.h"
#include "smtk/extension/paraview/server/RespondToVTKSelection_xml.h"

#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/view/Selection.h"

#include "smtk/io/Logger.h"

#include "vtkCompositeDataIterator.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

// Change "undef" to "define" to debug the selection operation
#undef SMTK_DEBUG_SELECTION

// FIXME: We cannot #include "pqView.h" because we don't want to link to Qt
//        but we need the enum values declared there. Copy them from there
//        for now:
class pqView
{
public:
  enum SelectionModifier
  {
    PV_SELECTION_DEFAULT = 0,
    PV_SELECTION_ADDITION,
    PV_SELECTION_SUBTRACTION,
    PV_SELECTION_TOGGLE
  };
};

namespace smtk
{
namespace view
{

RespondToVTKSelection::RespondToVTKSelection()
  : m_vtkSelection(nullptr)
  , m_vtkData(nullptr)
  , m_smtkSelectionSource("paraview")
  , m_smtkSelectionValue(1)
  , m_modifier(/* replace current selection */ 0)
  , m_selectingBlocks(false)
{
}

RespondToVTKSelection::~RespondToVTKSelection()
{
  if (m_vtkSelection)
  {
    m_vtkSelection->UnRegister(nullptr);
  }
  if (m_vtkData)
  {
    m_vtkData->UnRegister(nullptr);
  }
}

bool RespondToVTKSelection::setVTKData(vtkMultiBlockDataSet* mbds)
{
  if (m_vtkData == mbds)
  {
    return false;
  }

  if (m_vtkData)
  {
    m_vtkData->UnRegister(nullptr);
  }
  m_vtkData = mbds;
  if (m_vtkData)
  {
    m_vtkData->Register(nullptr);
  }
  return true;
}

bool RespondToVTKSelection::setVTKSelection(::vtkSelection* seln)
{
  if (m_vtkSelection == seln)
  {
    return false;
  }

  if (m_vtkSelection)
  {
    m_vtkSelection->UnRegister(nullptr);
  }
  m_vtkSelection = seln;
  if (m_vtkSelection)
  {
    m_vtkSelection->Register(nullptr);
  }
  return true;
}

bool RespondToVTKSelection::setSMTKSelection(const smtk::view::SelectionPtr& seln)
{
  auto curr = m_smtkSelection.lock();
  if (curr == seln)
  {
    return false;
  }

  m_smtkSelection = seln;
  return true;
}

smtk::view::SelectionPtr RespondToVTKSelection::smtkSelection() const
{
  return m_smtkSelection.lock();
}

bool RespondToVTKSelection::setSMTKSelectionSource(const std::string& sourceName)
{
  if (sourceName == m_smtkSelectionSource || sourceName.empty())
  {
    return false;
  }
  m_smtkSelectionSource = sourceName;
  return true;
}

bool RespondToVTKSelection::setSMTKSelectionValue(int value)
{
  if (value == m_smtkSelectionValue)
  {
    return false;
  }
  if (value == 0)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "A selection value of 0 is not allowed.");
    return false;
  }
  m_smtkSelectionValue = value;
  return true;
}

bool RespondToVTKSelection::setModifier(int modifier)
{
  if (m_modifier == modifier)
  {
    return false;
  }
  m_modifier = modifier;
  return true;
}

smtk::view::SelectionAction RespondToVTKSelection::modifierAction() const
{
  SelectionAction action;
  switch (m_modifier)
  {
    case pqView::SelectionModifier::PV_SELECTION_DEFAULT:
    case pqView::SelectionModifier::PV_SELECTION_ADDITION:
      action = SelectionAction::FILTERED_ADD;
      break;
    case pqView::SelectionModifier::PV_SELECTION_SUBTRACTION:
      action = SelectionAction::FILTERED_SUBTRACT;
      break;
    case pqView::SelectionModifier::PV_SELECTION_TOGGLE:
      // TODO: Handle PV_SELECTION_TOGGLE
      action = SelectionAction::FILTERED_ADD;
      smtkWarningMacro(
        smtk::io::Logger::instance(), "Toggle selections are not supported by SMTK yet.");
      break;
    default:
      action = SelectionAction::FILTERED_ADD;
      smtkWarningMacro(
        smtk::io::Logger::instance(), "Unknown ParaView selection modifier " << m_modifier << ".");
      break;
  }
  return action;
}

bool RespondToVTKSelection::setSelectingBlocks(bool shouldSelectBlocks)
{
  if (m_selectingBlocks == shouldSelectBlocks)
  {
    return false;
  }
  m_selectingBlocks = shouldSelectBlocks;
  return true;
}

bool RespondToVTKSelection::transcribeBlockSelection()
{
  bool didModify = false;
  if (!this->selectingBlocks())
  {
    return didModify;
  }

  auto selnMgr = this->smtkSelection();
  auto selnBlock = this->vtkSelection();
  unsigned nn = selnBlock ? selnBlock->GetNumberOfNodes() : 0;
  auto mbdsThing = this->vtkData();
  auto assoc = this->parameters()->associations();
  smtk::resource::ResourcePtr resource = assoc->valueAs<smtk::resource::Resource>();
  if (!resource)
  {
    return false;
  }
#ifdef SMTK_DEBUG_SELECTION
  if (mbdsThing)
  {
    vtkResourceMultiBlockSource::DumpBlockStructureWithUUIDs(mbdsThing);
  }
  if (selnBlock)
  {
    vtkIndent ind;
    selnBlock->PrintSelf(std::cout, ind);
  }
#endif
  std::set<vtkIdType> tessBlocks;
  std::set<vtkIdType> glyphBlocks;
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
    if (propId == source->GetEntitiesActorPickId() ||
      propId == source->GetSelectedEntitiesActorPickId())
    {
      tessBlocks.insert(compositeIndex);
    }
    else if (propId == source->GetGlyphEntitiesActorPickId() ||
      propId == source->GetSelectedGlyphEntitiesActorPickId())
    {
      glyphBlocks.insert(compositeIndex);
    }
    else
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Unknown prop ID " << propId << " when picking.");
    }
  }
  std::set<smtk::resource::ComponentPtr> seln;
  // Were any blocks selected?
  if (!tessBlocks.empty())
  {
    auto mit = mbdsThing->NewIterator();
    for (mit->InitTraversal(); !mit->IsDoneWithTraversal(); mit->GoToNextItem())
    {
      if (tessBlocks.find(mit->GetCurrentFlatIndex()) != tessBlocks.end())
      {
        auto cmp =
          resource->find(vtkResourceMultiBlockSource::GetDataObjectUUID(mit->GetCurrentMetaData()));
        if (cmp)
        {
          seln.insert(seln.end(), cmp);
        }
        // std::cout << "    tess  blk sel " << cmp->id() << " " << cmp->name() << "\n";
      }
    }
    mit->Delete();
  }
  if (!glyphBlocks.empty())
  {
    auto instances = vtkMultiBlockDataSet::SafeDownCast(
      mbdsThing->GetBlock(vtkResourceMultiBlockSource::BlockId::Value::Instances));
    if (instances)
    {
      auto mit = instances->NewIterator();
      for (mit->InitTraversal(); !mit->IsDoneWithTraversal(); mit->GoToNextItem())
      {
        if (glyphBlocks.find(mit->GetCurrentFlatIndex()) != glyphBlocks.end())
        {
          auto cmp = resource->find(
            vtkResourceMultiBlockSource::GetDataObjectUUID(mit->GetCurrentMetaData()));
          if (cmp)
          {
            seln.insert(seln.end(), cmp);
          }
          // std::cout << "    glyph blk sel " << cmp->id() << " " << cmp->name() << "\n";
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

RespondToVTKSelection::Result RespondToVTKSelection::operateInternal()
{
  bool worked = this->transcribeBlockSelection();
  auto result = this->createResult(worked ? smtk::operation::Operation::Outcome::SUCCEEDED
                                          : smtk::operation::Operation::Outcome::FAILED);
  return result;
}

const char* RespondToVTKSelection::xmlDescription() const
{
  return RespondToVTKSelection_xml;
}

} //namespace view
} // namespace smtk
