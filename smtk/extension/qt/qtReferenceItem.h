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
#include "smtk/extension/qt/MembershipBadge.h"
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
  qtReferenceItem(const qtAttributeItemInfo& info);
  ~qtReferenceItem() override;
  void markForDeletion() override;
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);

  enum AcceptsTypes
  {
    NONE,
    RESOURCES,
    COMPONENTS,
    BOTH
  };

  AcceptsTypes acceptableTypes() const;

  void setLabelVisible(bool) override;

  /**\brief Set the paths (into qrc files or the local filesystem) of icons
    *       used to indicate which items are selected and which are not.
    *
    * This returns true if the values were modified and false otherwise.
    *
    * Note that calling this method will not result in re-renders of any
    * views displaying the model; you are responsible for calling this
    * method before rendering takes place or for causing a render if
    * calling after the initial render.
    */
  bool setSelectionIconPaths(
    const std::string& selectedIconPath,
    const std::string& unselectedIconPath);

  /// Return the paths to icons used to display the membership in the item.
  std::pair<std::string, std::string> selectionIconPaths() const;

public Q_SLOTS:
  void updateItemData() override;

protected Q_SLOTS:
  void removeObservers();

  virtual void selectionLinkToggled(bool linked);
  virtual void setOutputOptional(int state);

  /**\brief Called whenever the edit button's QMenu is about to hide.
    *
    * This may or may not be initiated by synchronizeAndHide().
    * If not, then it is initiated directly by the user clicking outside
    * the popup, so it will call synchronizeAndHide() with
    * m_p->m_alreadyClosingPopup set so that it will only synchronize and
    * not hide.
    */
  void popupClosing();

  /**\brief Link this item to the "hovered" selection bit.
    *
    * This should set up the initial state, observe changes to the item, and update
    * the selection, and add some indication to the item showing the "hovered"
    * status (e.g., a color swatch).
    */
  virtual void linkHover(bool link);
  virtual void linkHoverTrue();
  virtual void linkHoverFalse();

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

  virtual void copyFromSelection();
  virtual void copyToSelection();
  virtual void clearItem();
  virtual void sneakilyHideButtons();
  virtual void cleverlyShowButtons();

  /// Update our labels when the membership badge has changed.
  void membershipChanged(int val);

protected:
  /**\brief Subclasses override this to create a model of the appropriate type.
    *
    * The model should be configured using information the item (this->configuration())
    * and be ready for use.
    */
  virtual smtk::view::PhraseModelPtr createPhraseModel() const;

  void createWidget() override;

  virtual void clearWidgets();
  virtual void updateUI();

  virtual std::string synopsis(bool& membershipValid) const;

  virtual void updateSynopsisLabels() const;

  bool eventFilter(QObject* src, QEvent* event) override;

  /// Called by eventFilter() when user hits space/enter in popup.
  virtual void toggleCurrentItem();

  /// Indicate whether the GUI should be updated from the item it presents or vice versa.
  enum class UpdateSource
  {
    ITEM_FROM_GUI, //!< Update the attribute-system's reference-item to match the GUI
    GUI_FROM_ITEM, //!< Update the GUI to match the attribute-system's reference-item
  };

  /// Children must implement this.
  virtual bool synchronize(UpdateSource src);

  /// retrieve membership from the phraseModel's badge
  smtk::extension::qt::MembershipBadge::MemberMap& members() const;

  void checkRemovedComponents(
    smtk::view::DescriptivePhrasePtr,
    smtk::view::PhraseModelEvent,
    const std::vector<int>&,
    const std::vector<int>&,
    const std::vector<int>&);

  qtReferenceItemData* m_p;
};
} // namespace extension
} // namespace smtk

#endif
