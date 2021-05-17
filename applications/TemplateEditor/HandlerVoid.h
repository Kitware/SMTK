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
  ~HandlerVoid() override = default;

  HandlerVoid(const HandlerVoid&) = delete;
  HandlerVoid& operator=(const HandlerVoid&) = delete;

private:
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
  ~HandlerFile() override = default;

  HandlerFile(const HandlerFile&) = delete;
  HandlerFile& operator=(const HandlerFile&) = delete;

private:
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
  ~HandlerDirectory() override = default;

  HandlerDirectory(const HandlerDirectory&) = delete;
  HandlerDirectory& operator=(const HandlerDirectory&) = delete;

private:
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
  ~HandlerComponent() override = default;

  HandlerComponent(const HandlerComponent&) = delete;
  HandlerComponent& operator=(const HandlerComponent&) = delete;

private:
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
  ~HandlerResource() override = default;

  HandlerResource(const HandlerResource&) = delete;
  HandlerResource& operator=(const HandlerResource&) = delete;

private:
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
  ~HandlerDateTime() override = default;

  HandlerDateTime(const HandlerDateTime&) = delete;
  HandlerDateTime& operator=(const HandlerDateTime&) = delete;

private:
  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};
#endif // __HandlerVoid_h
