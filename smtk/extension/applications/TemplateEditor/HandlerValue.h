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
public:
  HandlerValue();
  ~HandlerValue();

protected:
  QWidget* getSubclassWidget();

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override = 0;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) override;

private:
  HandlerValue(const HandlerValue&) = delete;
  void operator=(const HandlerValue&) = delete;

  std::unique_ptr<Ui::ItemDefValueForm> Ui;
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
  bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) override;

  std::unique_ptr<Ui::ItemDefStringForm> Ui;
};

///////////////////////////////////////////////////////////////////////////
namespace Ui
{
class ItemDefDoubleForm;
}

/**
 * \brief Generates a custom UI for a DoubleItemDefinition instance.
 */
class HandlerDouble : public HandlerValue
{
public:
  HandlerDouble();
  ~HandlerDouble();

private:
  HandlerDouble(const HandlerDouble&) = delete;
  void operator=(const HandlerDouble&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) override;

  std::unique_ptr<Ui::ItemDefDoubleForm> Ui;
};

///////////////////////////////////////////////////////////////////////////
namespace Ui
{
class ItemDefIntForm;
}

/**
 * \brief Generates a custom UI for a DoubleItemDefinition instance.
 */
class HandlerInt : public HandlerValue
{
public:
  HandlerInt();
  ~HandlerInt();

private:
  HandlerInt(const HandlerInt&) = delete;
  void operator=(const HandlerInt&) = delete;

  smtk::attribute::ItemDefinitionPtr createItemDef_impl(const std::string& name) override;
  smtk::attribute::ItemDefinitionPtr updateItemDef_impl() override;
  bool initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent) override;

  std::unique_ptr<Ui::ItemDefIntForm> Ui;
};
#endif // __HandlerValue_h
