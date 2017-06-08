//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __HandlerString_h
#define __HandlerString_h
#include "HandlerItemDef.h"

#include "smtk/attribute/ValueItemDefinitionTemplate.h"

namespace Ui
{
class ItemDefValueForm;
}

class QWidget;

/**
 * \brief Generates a custom UI for a ValueItemDefinition instance.
 *
 * This handler should not be instanced directly, instance concrete Value
 * ItemDefinitions (Double, Int, String).
 */
class HandlerValue : public HandlerItemDef
{
  //Q_OBJECT
public:
  HandlerValue();
  ~HandlerValue();

protected:
  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override = 0;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;

  std::shared_ptr<Ui::ItemDefValueForm> Ui;

private:
  HandlerValue(const HandlerValue&) = delete;
  void operator=(const HandlerValue&) = delete;
};

///////////////////////////////////////////////////////////////////////////
namespace Ui
{
class ItemDefStringForm;
}

/**
 * \brief Generates a custom UI for a StringItemDefinition instance.
 */
class HandlerString : public HandlerValue
{
public:
  HandlerString();
  ~HandlerString();

private:
  HandlerString(const HandlerString&) = delete;
  void operator=(const HandlerString&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;

  std::unique_ptr<Ui::ItemDefStringForm> Ui;
};

///////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a DoubleItemDefinition instance.
 */
class HandlerDouble : public HandlerValue
{
public:
  HandlerDouble() = default;
  ~HandlerDouble() = default;

private:
  HandlerDouble(const HandlerDouble&) = delete;
  void operator=(const HandlerDouble&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};

///////////////////////////////////////////////////////////////////////////
/**
 * \brief Generates a custom UI for a DoubleItemDefinition instance.
 */
class HandlerInt : public HandlerValue
{
public:
  HandlerInt() = default;
  ~HandlerInt() = default;

private:
  HandlerInt(const HandlerInt&) = delete;
  void operator=(const HandlerInt&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(QWidget* parent) override;
};
#endif // __HandlerValue_h
