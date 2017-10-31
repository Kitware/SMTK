//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_source_vtkTracksAllInstances_h
#define smtk_extension_vtk_source_vtkTracksAllInstances_h
/// !file

#ifndef __VTK_WRAP__
/**\brief Declare that a class will track all instances of itself.
  *
  * Invoke this macro in a public section of your class declaration.
  *
  * This macro declares methods linkInstance() and unlinkInstance().
  * While linkInstance() is called in the automatically-generated New()
  * method, you must call this->unlinkInstance() in the class destructor.
  *
  * You may then call visitInstances() with a C++ lambda that takes
  * an instance as an argument and returns true or false; it will
  * be called once for each extant instance of the class until it
  * returns false, at which point the method will exit.
  */
#define smtkDeclareTracksAllInstances(cls)                                                         \
protected:                                                                                         \
  static cls* s_allInstances;                                                                      \
  cls* m_prevInstance;                                                                             \
  cls* m_nextInstance;                                                                             \
                                                                                                   \
public:                                                                                            \
  void linkInstance();                                                                             \
  void unlinkInstance();                                                                           \
  static void visitInstances(std::function<bool(cls*)>);
#else
#define smtkDeclareTracksAllInstances(cls)
#endif

/**\brief Implement methods for tracking all instances of a class.
  *
  * Invoke this macro in a single implementation file; it provides
  * implementations for methods plus a class-static variable
  * referring to the head of a doubly-linked list of instances.
  */
#define smtkImplementTracksAllInstances(cls)                                                       \
  cls* cls::s_allInstances = nullptr;                                                              \
  void cls::linkInstance()                                                                         \
  {                                                                                                \
    m_nextInstance = cls::s_allInstances;                                                          \
    cls::s_allInstances = this;                                                                    \
    if (m_nextInstance)                                                                            \
    {                                                                                              \
      m_nextInstance->m_prevInstance = this;                                                       \
    }                                                                                              \
    m_prevInstance = nullptr;                                                                      \
  }                                                                                                \
  void cls::unlinkInstance()                                                                       \
  {                                                                                                \
    if (m_prevInstance)                                                                            \
    {                                                                                              \
      m_prevInstance->m_nextInstance = m_nextInstance;                                             \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
      cls::s_allInstances = m_nextInstance;                                                        \
    }                                                                                              \
    if (m_nextInstance)                                                                            \
    {                                                                                              \
      m_nextInstance->m_prevInstance = m_prevInstance;                                             \
    }                                                                                              \
    m_nextInstance = nullptr;                                                                      \
    m_prevInstance = nullptr;                                                                      \
  }                                                                                                \
  void cls::visitInstances(std::function<bool(cls*)> visitor)                                      \
  {                                                                                                \
    for (cls* inst = cls::s_allInstances; inst; inst = inst->m_nextInstance)                       \
    {                                                                                              \
      if (!visitor(inst))                                                                          \
      {                                                                                            \
        break;                                                                                     \
      }                                                                                            \
    }                                                                                              \
  }

#endif
