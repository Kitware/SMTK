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

/**
 * \brief Generates a custom UI for a RefItemDefinition instance.
 */
class HandlerRef : public HandlerItemDef
{
public:
  HandlerRef();
  ~HandlerRef();

private:
  HandlerRef(const HandlerRef&) = delete;
  void operator=(const HandlerRef&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;

  std::unique_ptr<Ui::ItemDefRefForm> Ui;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a FileItemDefinition instance.
 * TODO Derive from HandlerFileSystem
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
 * TODO Derive from HandlerFileSystem
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
 * \brief Generates a custom UI for a ModelEntityItemDef instance.
 */
class HandlerModelEntity : public HandlerItemDef
{
public:
  HandlerModelEntity() = default;
  ~HandlerModelEntity() = default;

private:
  HandlerModelEntity(const HandlerModelEntity&) = delete;
  void operator=(const HandlerModelEntity&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a MeshSelectionItemDef instance.
 */
class HandlerMeshSelection : public HandlerItemDef
{
public:
  HandlerMeshSelection() = default;
  ~HandlerMeshSelection() = default;

private:
  HandlerMeshSelection(const HandlerMeshSelection&) = delete;
  void operator=(const HandlerMeshSelection&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a MeshEntityItemDef instance.
 */
class HandlerMeshEntity : public HandlerItemDef
{
public:
  HandlerMeshEntity() = default;
  ~HandlerMeshEntity() = default;

private:
  HandlerMeshEntity(const HandlerMeshEntity&) = delete;
  void operator=(const HandlerMeshEntity&) = delete;

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
