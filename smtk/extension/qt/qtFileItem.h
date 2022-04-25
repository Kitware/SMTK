//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtFileItem_h
#define smtk_extension_qtFileItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

class qtFileItemInternals;
class QBoxLayout;
class QComboBox;
class QLineEdit;
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
  * ItemView Configuration Options:
  *
  * ShowRecentFiles="true/false" - if the file must already exist, this option, when true, will
  * display previous values using a combobox - default is false
  *
  * ShowFileExtensions="true/false" - if the file is not required to already exist, and there are
  * a set of suffixes associated with the item, this option, when true, will display the suffixes
  * using a combobox - default is false
  *
  * UseFileDirectory="true/false" - if true and if the value is set, the file browser will open in
  * the directory of the current value.  If the current value's directory does not exist the browser
  * will either use the DefaultDirectoryProperty value or revert to its default behavior - default
  * is true
  *
  * DefaultDirectoryProperty="nameOfResourceStringProperty" - if set and the string vector property
  * value on the item's attribute resource exists and refers to an existing directory.
  * the file browser will open in the directory refereed to by the property value
  *
  * See fileItemExample.sbt and fileItemExample.smtk as examples.  Note that to demonstrate
  * DefaultDirectoryProperty using these files you will need to create a string property called
  * testDir and set it to something valid.
  *
  * \sa qtItem
  */
class SMTKQTEXT_EXPORT qtFileItem : public qtItem
{
  Q_OBJECT

public:
  ///\brief Factory method that can be registered with smtk::extension::qtUIManager
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtFileItem(const qtAttributeItemInfo& info);
  ~qtFileItem() override;
  void setLabelVisible(bool) override;

  void enableFileBrowser(bool state = true);
  ///\brief Are the fileItem's values directories?
  bool isDirectory() const;
  ///\brief In the case of files that should exist, should we
  /// show values that have been used in the past?
  bool showRecentFiles() const;
  ///\brief In the case of files that may or may not exist
  /// should we show valid file extensions?
  bool showExtensions() const;
  ///\brief When changing an value should we try to use that
  /// file's directory as the file browser's starting point?
  ///
  ///  Note that if this not true or if the directory does not
  /// exist then the default directory (if one
  /// has been specified) or the browser's default behavior
  /// will be used
  bool useFileDirectory() const;
  ///\brief Is there a default directory for the file browser
  bool hasDefaultDirectory() const;
  ///\brief Return the default directory
  const std::string& defaultDirectory() const;
  virtual void setInputValue(int i, const QString&);
  void updateItemValue(int i);
  void updateEditorValue(int i);
public Q_SLOTS:
  virtual void onUpdateItemValue();
  void setOutputOptional(int);
  virtual bool onLaunchFileBrowser();
  void updateItemData() override;

Q_SIGNALS:
  bool launchFileBrowser();

protected Q_SLOTS:
  virtual void onAddNewValue();
  virtual void onRemoveValue();

protected:
  void createWidget() override;
  QWidget* createFileBrowseWidget(
    int elementIdx,
    const smtk::attribute::FileSystemItem& item,
    const smtk::attribute::FileSystemItemDefinition& itemDef);
  virtual void loadInputValues(
    const smtk::attribute::FileSystemItem& item,
    const smtk::attribute::FileSystemItemDefinition& itemDef);
  virtual void updateUI();
  virtual void addInputEditor(
    int i,
    const smtk::attribute::FileSystemItem& item,
    const smtk::attribute::FileSystemItemDefinition& itemDef);
  virtual void clearChildWidgets();
  bool updateRecentValues(const std::string& val);
  virtual void updateFileComboLists();
  void getEditor(int i, QComboBox** cbox, QLineEdit** lineEdit);

private:
  qtFileItemInternals* m_internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
