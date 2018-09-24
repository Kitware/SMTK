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
#include "smtk/view/PhraseModel.h"

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
  qtReferenceItem(const AttributeItemInfo& info);
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

  /**\brief Called when the user asks to close the popup.
    *
    * If \a escaping is false and if the widget's state is valid,
    * then update the attribute-system item's state.
    * Otherwise, revert the widget's state to the attribute-system item's state.
    *
    * In the future we may provide applications with more options (either via
    * signals/slots to resolve problems, or via preferences that can be set).
    *
    * When \a escaping is true, this method is being called becuase the user clicked
    * the Cancel or Escape keys. Otherwise, this method is being called because the
    * popup's Done button has been pressed.
    */
  virtual void synchronizeAndHide(bool escaping = false);

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

  /// Indicate whether the GUI should be updated from the item it presents or vice versa.
  enum class UpdateSource
  {
    ITEM_FROM_GUI, //!< Update the attribute-system's reference-item to match the GUI
    GUI_FROM_ITEM, //!< Update the GUI to match the attribute-system's reference-item
  };

  /// Children must implement this.
  virtual bool synchronize(UpdateSource src) = 0;

  void checkRemovedComponents(smtk::view::DescriptivePhrasePtr, smtk::view::PhraseModelEvent,
    const std::vector<int>&, const std::vector<int>&, const std::vector<int>&);

  qtReferenceItemData* m_p;
};
}
}

#endif
