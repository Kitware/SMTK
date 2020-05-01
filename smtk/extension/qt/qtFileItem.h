//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtFileItem_h
#define __smtk_extension_qtFileItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

class qtFileItemInternals;
class QBoxLayout;
class QWidget;

namespace smtk
{
namespace extension
{

/**\brief Provides the QT UI for a smtk::attribute::FileSystemItem.
  *
  * qtFileSystemItem generates a UI for interacting with a smtk::attribute::FileSystemItem.
  * In the case of a non-extensiable item, this qtItem generates an editable combobox along
  * with a file browser for each value stored within the item.  The combobox also stores
  * the previous values entered allowing the user to choose from those along with entering
  * an new value or selecting one using the file browser.  If the item is extensible the
  * generated GUI is similar except that instead of a combobox a line edit widget is created.
  *
  * In either case, the line edit /combobox widget is colored based on whether the value is
  * the default (if one exists), valid, or invalid.  Note that the validity takes into
  * consideration not only the validity imposed by the item definition's validity method
  * but whether the file should exist or not.
  *
  * This qtItem is the default type used by smtk::extension::qtUIManager.
  *
  * \sa qtItem
  */
class SMTKQTEXT_EXPORT qtFileItem : public qtItem
{
  Q_OBJECT

public:
  /**\brief Factor method that can be registered with smtk::extension::qtUIManager */
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtFileItem(const qtAttributeItemInfo& info);
  virtual ~qtFileItem();
  void setLabelVisible(bool) override;

  void enableFileBrowser(bool state = true);
  bool isDirectory();
  virtual void setInputValue(const QString&);

public slots:
  virtual void onInputValueChanged();
  void setOutputOptional(int);
  virtual bool onLaunchFileBrowser();
  virtual void updateFileComboList(const QString&);
  void updateItemData() override;

signals:
  bool launchFileBrowser();

protected slots:
  virtual void onAddNewValue();
  virtual void onRemoveValue();
  virtual void setActiveField(QWidget*);

protected:
  void createWidget() override;
  QWidget* createFileBrowseWidget(int elementIdx, const smtk::attribute::FileSystemItem& item,
    const smtk::attribute::FileSystemItemDefinition& itemDef);
  virtual void loadInputValues(const smtk::attribute::FileSystemItem& item,
    const smtk::attribute::FileSystemItemDefinition& itemDef);
  virtual void updateUI();
  virtual void addInputEditor(int i, const smtk::attribute::FileSystemItem& item,
    const smtk::attribute::FileSystemItemDefinition& itemDef);
  virtual void updateExtensibleState();
  virtual void clearChildWidgets();

private:
  qtFileItemInternals* m_internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
