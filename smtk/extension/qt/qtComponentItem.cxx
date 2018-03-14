//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtComponentItem.h"
#include "smtk/extension/qt/qtReferenceItemData.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/model/Manager.h"

namespace smtk
{
namespace extension
{

class qtComponentItem::Internal
{
public:
  std::map<smtk::resource::ComponentPtr, int> m_members;
};

qtComponentItem::qtComponentItem(
  smtk::attribute::ComponentItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation enumOrient)
  : Superclass(std::static_pointer_cast<smtk::attribute::Item>(item), p, bview)
{
  (void)enumOrient;
  m_p = new Internal;
  this->createWidget();
}

qtComponentItem::~qtComponentItem()
{
  delete m_p;
}

void qtComponentItem::setLabelVisible(bool visible)
{
  qtReferenceItem::m_p->m_label->setVisible(visible);
}

smtk::attribute::ComponentItemPtr qtComponentItem::componentItem() const
{
  return std::static_pointer_cast<smtk::attribute::ComponentItem>(this->getObject());
}

void qtComponentItem::updateItemData()
{
}

smtk::view::PhraseModelPtr qtComponentItem::createPhraseModel() const
{
  // Constructing the PhraseModel with a View properly initializes the SubphraseGenerator
  // to point back to the model (thus ensuring subphrases are decorated). This is required
  // since we need to decorate phrases to show+edit "visibility" as set membership:
  auto view = smtk::view::View::New("ComponentItem", "stuff");
  auto phraseModel = smtk::view::ComponentPhraseModel::create(view);
  phraseModel->root()->findDelegate()->setModel(phraseModel);
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ComponentItemDefinition>(
    this->getObject()->definition());
  std::static_pointer_cast<smtk::view::ComponentPhraseModel>(phraseModel)
    ->setComponentFilters(def->acceptableEntries());

  return phraseModel;
}

void qtComponentItem::createWidget()
{
  // Let our subclass do the UI work.
  this->Superclass::createWidget();

  // Now add in ComponentItem specifics.
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if (!dataObj || !this->passAdvancedCheck() ||
    (this->baseView() &&
      !this->baseView()->uiManager()->passItemCategoryCheck(dataObj->definition())))
  {
    return;
  }

  this->updateItemData();
}

std::string qtComponentItem::synopsis(bool& ok) const
{
  auto item = this->componentItem();
  if (!item)
  {
    ok = false;
    return "uninitialized item";
  }

  static const std::size_t numRequired = item->numberOfRequiredValues();
  static const std::size_t maxAllowed =
    item->isExtensible() ? item->maxNumberOfValues() : numRequired;
  std::ostringstream label;
  std::size_t numSel = 0;
  for (auto entry : m_p->m_members)
  {
    if (entry.second > 0)
    {
      ++numSel;
    }
  }
  ok = true;
  if (numRequired < 2 && maxAllowed == 1)
  {
    auto ment = std::dynamic_pointer_cast<smtk::model::Entity>(
      m_p->m_members.empty() ? smtk::resource::ComponentPtr() : m_p->m_members.begin()->first);
    label << (numSel == 1
        ? (ment ? ment->referenceAs<smtk::model::EntityRef>().name() : "TODO (report item name)")
        : (numSel > 0 ? "too many" : "(none)"));
    ok = numSel >= numRequired && numSel <= maxAllowed;
  }
  else
  {
    label << numSel;
    if (numRequired > 0)
    {
      label << " of ";
      if (numRequired == maxAllowed)
      { // Exactly N values are allowed and required.
        label << numRequired;
      }
      else if (maxAllowed > 0)
      { // There is a minimum required, but a limited additional number are acceptable
        label << numRequired << "—" << maxAllowed;
      }
      else
      { // Any number are allowed, but there is a minimum.
        label << numRequired << "+";
      }
      ok &= (numSel >= numRequired);
    }
    else
    { // no values are required, but there may be a cap on the maximum number.
      if (maxAllowed > 0)
      {
        label << " of 0–" << maxAllowed;
      }
      else
      {
        label << " chosen";
      }
    }
  }
  ok &= (maxAllowed == 0 || numSel <= maxAllowed);
  return label.str();
}

int qtComponentItem::decorateWithMembership(smtk::view::DescriptivePhrasePtr phr)
{
  smtk::view::VisibilityContent::decoratePhrase(phr, [this](smtk::view::VisibilityContent::Query qq,
                                                       int val,
                                                       smtk::view::ConstPhraseContentPtr data) {
    smtk::model::EntityPtr ent =
      data ? std::dynamic_pointer_cast<smtk::model::Entity>(data->relatedComponent()) : nullptr;
    smtk::model::ManagerPtr mmgr = ent
      ? ent->modelResource()
      : (data ? std::dynamic_pointer_cast<smtk::model::Manager>(data->relatedResource()) : nullptr);

    switch (qq)
    {
      case smtk::view::VisibilityContent::DISPLAYABLE:
        return (ent || (!ent && mmgr)) ? 1 : 0;
      case smtk::view::VisibilityContent::EDITABLE:
        return (ent || (!ent && mmgr)) ? 1 : 0;
      case smtk::view::VisibilityContent::GET_VALUE:
        if (ent)
        {
          auto valIt = m_p->m_members.find(ent);
          if (valIt != m_p->m_members.end())
          {
            return valIt->second;
          }
          return 0; // visibility is assumed if there is no entry.
        }
        return 0; // visibility is false if the component is not a model entity or NULL.
      case smtk::view::VisibilityContent::SET_VALUE:
        if (ent)
        {
          if (val && !m_p->m_members.empty())
          {
            auto item = this->componentItem();
            if (item->numberOfRequiredValues() <= 1 && item->maxNumberOfValues() == 1)
            { // Clear all other members since only 1 is allowed and the user just chose it.
              m_p->m_members.clear();
              qtReferenceItem::m_p->m_phraseModel->triggerDataChanged();
            }
          }
          m_p->m_members[ent] = val ? 1 : 0;
          this->updateSynopsisLabels();
          return 1;
        }
    }
    return 0;
  });
  return 0;
}

void qtComponentItem::toggleCurrentItem()
{
  auto cphr =
    qtReferenceItem::m_p->m_qtModel->getItem(qtReferenceItem::m_p->m_popupList->currentIndex());
  if (cphr)
  {
    auto currentMembership = cphr->relatedVisibility();
    // Selecting a new item when only 1 is allowed should reset all other membership.
    if (!currentMembership && !m_p->m_members.empty())
    {
      auto item = this->componentItem();
      if (item->numberOfRequiredValues() <= 1 && item->maxNumberOfValues() == 1)
      {
        m_p->m_members.clear();
        qtReferenceItem::m_p->m_phraseModel->triggerDataChanged();
      }
    }
    cphr->setRelatedVisibility(!currentMembership);
    this->updateSynopsisLabels();
  }
}
}
}
