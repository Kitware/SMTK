//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtReferenceTree_h
#define smtk_extension_qt_qtReferenceTree_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/view/PhraseModel.h"

namespace smtk
{
namespace extension
{

class qtReferenceTreeData;

/**\brief A tree-view UI for attribute associations and reference-items.
  *
  * The qtReferenceTree class provides a uniform GUI for selecting entries that
  * populate an attribute's item while abstracting away the types of those
  * entries. The organization of the tree can be configured on a per-instance
  * basis by choosing different phrase models and subphrase generators.
  * A subset of the tree identified by a MembershipCriteria and a filter string
  * is made available for selection (and thus inclusion in the associated
  * or referenced components/resources/objects).
  */
class SMTKQTEXT_EXPORT qtReferenceTree : public qtItem
{
  Q_OBJECT
  using Superclass = qtItem;

public:
  qtReferenceTree(const qtAttributeItemInfo& info);
  ~qtReferenceTree() override;
  void markForDeletion() override;
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);

  qt::MembershipCriteria membershipCriteria() const;
  std::string membershipFilter() const;

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

  /// Return the phrase model controlling the tree's contents.
  std::shared_ptr<smtk::view::PhraseModel> phraseModel() const;

public Q_SLOTS:
  void updateItemData() override;
  virtual void synchronizeMembers();

protected Q_SLOTS:
  void removeObservers();

  virtual void setOutputOptional(int state);

  /**\brief Link this item to the "hovered" selection bit.
    *
    * This should set up the initial state, observe changes to the item, and update
    * the selection, and add some indication to the item showing the "hovered"
    * status (e.g., a color swatch).
    */
  virtual void linkHover(bool link);
  virtual void linkHoverTrue();
  virtual void linkHoverFalse();

  virtual void copyFromSelection();
  virtual void copyToSelection();
  virtual void clearItem();

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

  bool eventFilter(QObject* src, QEvent* event) override;

  /// Toggle the currently-selected item(s)'s membership (when \a membership is true)
  /// or visibility (when \a membership is false).
  ///
  /// Called by eventFilter() when user hits space (membership) or enter (visibility).
  virtual void toggleCurrentItem(bool membership);

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

  qtReferenceTreeData* m_p;
};
} // namespace extension
} // namespace smtk

#endif
