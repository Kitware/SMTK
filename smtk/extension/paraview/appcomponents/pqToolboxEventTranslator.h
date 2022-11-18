//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef pqToolboxEventTranslator_h
#define pqToolboxEventTranslator_h

#include "pqWidgetEventTranslator.h"
#include "smtkPQComponentsExtModule.h"

/** \brief Translate low-level Qt events into high-level SMTK events
  *        that denote intent when recording tests.
  *
  * \sa pqEventTranslator
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqToolboxEventTranslator : public pqWidgetEventTranslator
{
  Q_OBJECT
  typedef pqWidgetEventTranslator Superclass;

public:
  pqToolboxEventTranslator(QObject* p = nullptr);
  ~pqToolboxEventTranslator() override;

  using Superclass::translateEvent;
  bool translateEvent(QObject* Object, QEvent* Event, int eventType, bool& Error) override;

private:
  pqToolboxEventTranslator(const pqToolboxEventTranslator&);
  pqToolboxEventTranslator& operator=(const pqToolboxEventTranslator&);
};

#endif // !pqToolboxEventTranslator_h
