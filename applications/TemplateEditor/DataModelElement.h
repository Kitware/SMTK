//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef DataModelElement_h
#define DataModelElement_h

#include <QTreeWidgetItem>

/**
 * \brief Qt Data model item holding a reference to custom model-data.
 *
 * The class and instances are referred to as 'elements' instead of 'items'
 * to avoid any confusion with the term 'item' in the context of the SMTK
 * attribute system (ItemDefinition, Item, etc.).
 *
 * ///TODO Implement the rest of the virtual interface of QTreeWidgetItem
 * to customize how data is set and queried from.
 */
template <typename T>
class DataModelElement : public QTreeWidgetItem
{
public:
  DataModelElement(QTreeWidgetItem* parent = nullptr);
  ~DataModelElement() = default;

  void setReferencedData(const T& data);

  /**
 * Get the actual underlying data referenced by this element in the
 * data model.
 */
  const T& getReferencedDataConst() const;

private:
  DataModelElement(const DataModelElement&) = delete;
  void operator=(const DataModelElement&) = delete;

  /**
 * Copy of the underlying referenced data.
 */
  T m_data;
};

template <typename T>
DataModelElement<T>::DataModelElement(QTreeWidgetItem* parent)
  : QTreeWidgetItem(parent){};

template <typename T>
void DataModelElement<T>::setReferencedData(const T& data)
{
  m_data = data;
};

template <typename T>
const T& DataModelElement<T>::getReferencedDataConst() const
{
  return m_data;
};
#endif //DataModelElement_h
