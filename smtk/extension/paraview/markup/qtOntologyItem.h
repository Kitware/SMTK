//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtOntologyItem_h
#define smtk_extension_qtOntologyItem_h

#include "smtk/extension/paraview/markup/smtkPQMarkupExtModule.h"
#include "smtk/extension/qt/qtItem.h"

class qtOntologyModel;

class QAction;
class QBoxLayout;
class QComboBox;
class QLineEdit;
class QTableWidgetItem;
class QWidget;

namespace smtk
{
namespace extension
{

/**\brief Provides a QT UI for choosing an ontology identifier from an ontology.
  *
  * This item exists for the smtk::markup resource although there is nothing
  * specific to that resource in this widget. It is used in the context of
  * markup resources to create OntologyIdentifier nodes and connect them to
  * instances of the class the identifier represents.
  *
  * \sa qtItem
  */
class SMTKPQMARKUPEXT_EXPORT qtOntologyItem : public qtItem
{
  Q_OBJECT

public:
  ///\brief Factory method that can be registered with smtk::extension::qtUIManager
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtOntologyItem(const qtAttributeItemInfo& info);
  ~qtOntologyItem() override;

  /**\brief Set the ontology model used for completion to a pre-registered default.
    *
    * When you call qtOntologyModel::registerOntology() with XML and a URL,
    * you can refer to it here by name and an instance of qtOntologyModel
    * will be created for you.
    *
    * This method returns true on success.
    */
  virtual bool setOntologyModel(const std::string& modelName);

  /**\brief Set the ontology model used for completion.
    *
    */
  virtual bool setOntologyModel(qtOntologyModel* model);
  /**\brief Return the ontology model used for completion.
    *
    * When the model is accepted and not already current, true is returned.
    */
  qtOntologyModel* ontologyModel() const;

public Q_SLOTS:
  /// Display data from \a index in the item's view (but do not update the attribute).
  virtual void modelEntryHighlighted(const QModelIndex& index);
  /// Display data from \a index in the item's view (and also update the attribute).
  virtual void modelEntryChosen(const QModelIndex& index);
  /// Look up the model index specified by the given \a text and call modelEntryChosen() with it.
  virtual void textActivated(const QString& text);
  /// Empty the description when the search text becomes empty.
  virtual void emptyDescriptionOnEmptySearch(const QString& search);
  /// Call textActivated() with the current search text.
  /// This slot is invoked when the plus ("+") button is clicked.
  virtual void attemptToAddTag();
  /// Respond to a press of the minus ("-") button by removing the currently-selected tags
  /// from the list of tags to be added.
  virtual void removeSelectedTags();

  /// Update the user interface to match data in the attribute-item's value.
  ///
  /// This is invoked at initialization and also when the item is updated
  /// externally (i.e., by an operation such as Signal).
  void updateItemData() override;

  /// Invoked when the operation's associations change.
  /// This slot is connected by the parent view.
  void associationsChanged();

  /// Invoked when any item in the parent view is modified.
  /// This will call associationsChanged() if the item being modified
  /// is the operation's associations.
  void associationsMayHaveChanged(const std::shared_ptr<smtk::attribute::Item>& item);

protected:
  void createWidget() override;
  void updateUI();
  void setItemEnabled(int checkState);

private:
  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk

#endif
