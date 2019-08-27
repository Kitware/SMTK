//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __HandlerVoid_h
#define __HandlerVoid_h
#include "HandlerItemDef.h"

/**
 * \brief Generates a custom UI for a VoidItemDefinition instance.
 */
class HandlerVoid : public HandlerItemDef
{
public:
  HandlerVoid() = default;
  ~HandlerVoid() = default;

private:
  HandlerVoid(const HandlerVoid&) = delete;
  void operator=(const HandlerVoid&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
class ItemDefRefForm;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a FileItemDefinition instance.
 * TODO Derive from HandlerFileCollection
 */
class HandlerFile : public HandlerItemDef
{
public:
  HandlerFile() = default;
  ~HandlerFile() = default;

private:
  HandlerFile(const HandlerFile&) = delete;
  void operator=(const HandlerFile&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a DirectoryItemDefinition instance.
 * TODO Derive from HandlerFileCollection
 */
class HandlerDirectory : public HandlerItemDef
{
public:
  HandlerDirectory() = default;
  ~HandlerDirectory() = default;

private:
  HandlerDirectory(const HandlerDirectory&) = delete;
  void operator=(const HandlerDirectory&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a ComponentItemDef instance.
 */
class HandlerComponent : public HandlerItemDef
{
public:
  HandlerComponent() = default;
  ~HandlerComponent() = default;

private:
  HandlerComponent(const HandlerComponent&) = delete;
  void operator=(const HandlerComponent&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a ResourceItemDef instance.
 */
class HandlerResource : public HandlerItemDef
{
public:
  HandlerResource() = default;
  ~HandlerResource() = default;

private:
  HandlerResource(const HandlerResource&) = delete;
  void operator=(const HandlerResource&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a DateTimeItemDef instance.
 */
class HandlerDateTime : public HandlerItemDef
{
public:
  HandlerDateTime() = default;
  ~HandlerDateTime() = default;

private:
  HandlerDateTime(const HandlerDateTime&) = delete;
  void operator=(const HandlerDateTime&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};
#endif // __HandlerVoid_h
