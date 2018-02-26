//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtReferenceItem_h
#define smtk_extension_qt_qtReferenceItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtItem.h"

namespace smtk
{
namespace extension
{

class qtReferenceItemData;

/**\brief A base class for component and resource items.
  *
  * The qtReferenceItem class provides a uniform GUI for selecting entries that
  * populate an attribute's item while abstracting away the types of those
  * entries.
  */
class SMTKQTEXT_EXPORT qtReferenceItem : public qtItem
{
  Q_OBJECT
  using Superclass = qtItem;

public:
  qtReferenceItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* bview);
  virtual ~qtReferenceItem();

protected slots:
  virtual void selectionLinkToggled(bool linked);
  virtual void setOutputOptional(int state);

  /**\brief Link this item to the "hovered" selection bit.
    *
    * This should set up the initial state, observe changes to the item and update
    * the selection, and add some indication to the item showing the "hovered"
    * status (e.g., a color swatch).
    */
  virtual void linkHover(bool link);

protected:
  /**\brief Subclasses override this to create a model of the appropriate type.
    *
    * The model should be configured using information the item (this->getObject())
    * and be ready for use.
    */
  virtual smtk::view::PhraseModelPtr createPhraseModel() const = 0;

  virtual void createWidget() override;

  virtual void clearWidgets();
  virtual void updateUI();

  virtual std::string synopsis(bool& membershipValid) const = 0;

  virtual void updateSynopsisLabels() const;

  bool eventFilter(QObject* src, QEvent* event) override;

  /// Called by eventFilter() when user hits space/enter in popup.
  virtual void toggleCurrentItem() = 0;

  /**\brief This method is called by the m_p->m_phraseModel to decorate each phrase.
    *
    * This method ensures that each phrase's visibility corresponds to whether
    * or not it is a member of the underlying attribute-system's reference item.
    */
  virtual int decorateWithMembership(smtk::view::DescriptivePhrasePtr phr) = 0;

  qtReferenceItemData* m_p;
};
}
}

#endif
