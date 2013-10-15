/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME PublicPointerDefs.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_PublicPointerDefs_h
#define __smtk_PublicPointerDefs_h

#include "smtk/SharedPtr.h"
#include <set>

namespace smtk
{
  template <typename T, typename U >
  inline smtk::shared_ptr< T > dynamicCastPointer(const smtk::shared_ptr< U > &r)
  {
    return smtk::dynamic_pointer_cast< T >(r);
  }

  namespace model
  {
    class Model;
    class Item;
    class GridInfo;
    class GroupItem;
    class ModelDomainItem;
  }

  namespace attribute
  {
    class Attribute;
    class AttributeRefItem;
    class AttributeRefItemDefinition;
    class AttributeDefinition;
    class DirectoryItem;
    class DirectoryItemDefinition;
    class DoubleItem;
    class DoubleItemDefinition;
    class FileItem;
    class FileItemDefinition;
    class GroupItem;
    class GroupItemDefinition;
    class IntItem;
    class IntItemDefinition;
    class Item;
    class ItemDefinition;
    class Manager;
    class StringItem;
    class StringItemDefinition;
    class ValueItem;
    class ValueItemDefinition;
    class VoidItem;
    class VoidItemDefinition;
  }

  namespace util
  {
    class UserData;
  }

  namespace view
  {
    class Attribute;
    class Base;
    class Group;
    class Instanced;
    class ModelEntity;
    class Root;
    class SimpleExpression;
  };

  //Shiboken requires that we use fully qualified namespaces for all
  //types that these shared_ptr and weak_ptr are holding
  typedef smtk::shared_ptr< smtk::model::Model >      ModelPtr;
  typedef smtk::weak_ptr< smtk::model::Model >        WeakModelPtr;
  typedef smtk::shared_ptr< smtk::model::Item >       ModelItemPtr;
  typedef smtk::weak_ptr< smtk::model::Item >         WeakModelItemPtr;
  typedef smtk::shared_ptr< smtk::model::GroupItem >  ModelGroupItemPtr;

  typedef smtk::shared_ptr< smtk::attribute::Attribute >        AttributePtr;
  typedef smtk::weak_ptr< smtk::attribute::Attribute >          WeakAttributePtr;
  typedef smtk::shared_ptr< smtk::attribute::AttributeDefinition >       AttributeDefinitionPtr;
  typedef smtk::shared_ptr< const smtk::attribute::AttributeDefinition > ConstAttributeDefinitionPtr;
  typedef smtk::weak_ptr< smtk::attribute::AttributeDefinition >         WeakAttributeDefinitionPtr;

  typedef smtk::shared_ptr< smtk::attribute::AttributeRefItem >           AttributeRefItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::AttributeRefItemDefinition > AttributeRefItemDefinitionPtr;

  typedef smtk::shared_ptr< smtk::attribute::Item >                 AttributeItemPtr;
  typedef smtk::shared_ptr< const smtk::attribute::Item >           ConstAttributeItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::ItemDefinition >       AttributeItemDefinitionPtr;
  typedef smtk::shared_ptr< const smtk::attribute::ItemDefinition > ConstAttributeItemDefinitionPtr;
  typedef smtk::weak_ptr< smtk::attribute::Item >                   WeakAttributeItemPtr;
  typedef smtk::weak_ptr< smtk::attribute::ItemDefinition >         WeakAttributeItemDefinitionPtr;

  typedef smtk::shared_ptr< smtk::attribute::ValueItem >            ValueItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::ValueItemDefinition >  ValueItemDefinitionPtr;

  typedef smtk::shared_ptr< smtk::attribute::DirectoryItem >            DirectoryItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::DirectoryItemDefinition >  DirectoryItemDefinitionPtr;
  typedef smtk::shared_ptr< smtk::attribute::DoubleItem >               DoubleItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::DoubleItemDefinition >     DoubleItemDefinitionPtr;
  typedef smtk::shared_ptr< smtk::attribute::FileItem >                 FileItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::FileItemDefinition >       FileItemDefinitionPtr;
  typedef smtk::shared_ptr< smtk::attribute::GroupItem >                GroupItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::GroupItemDefinition >      GroupItemDefinitionPtr;
  typedef smtk::shared_ptr< smtk::attribute::IntItem >                  IntItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::IntItemDefinition >        IntItemDefinitionPtr;
  typedef smtk::shared_ptr< smtk::attribute::StringItem >               StringItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::StringItemDefinition >     StringItemDefinitionPtr;
  typedef smtk::shared_ptr< smtk::attribute::VoidItem >                 VoidItemPtr;
  typedef smtk::shared_ptr< smtk::attribute::VoidItemDefinition >       VoidItemDefinitionPtr;

  typedef smtk::shared_ptr< smtk::attribute::Manager >                  AttributeManagerPtr;

  namespace util
  {
    //custom user data classes
    typedef smtk::shared_ptr< smtk::util::UserData > UserDataPtr;
  };

  namespace view
  {
  // View Related Pointer Classes
    typedef smtk::shared_ptr< smtk::view::Base >             BasePtr;
    typedef smtk::weak_ptr< smtk::view::Base >               WeakBasePtr;
    typedef smtk::shared_ptr< smtk::view::Attribute>         AttributePtr;
    typedef smtk::shared_ptr< smtk::view::Group>             GroupPtr;
    typedef smtk::shared_ptr< smtk::view::Instanced>         InstancedPtr;
    typedef smtk::shared_ptr< smtk::view::ModelEntity>       ModelEntityPtr;
    typedef smtk::shared_ptr< smtk::view::Root>              RootPtr;
    typedef smtk::shared_ptr< smtk::view::SimpleExpression>  SimpleExpressionPtr;
  };


  // class for making the analysis grid information available in SMTK
  typedef smtk::shared_ptr< smtk::model::GridInfo >                     GridInfoPtr;

#if defined(_WIN32) && defined(_MSC_VER) && _MSC_VER  >= 1600
  //special map and set typedefs to work around VS removing less
  //than from weak pointer in 2010 and greater
  typedef std::set< WeakAttributeDefinitionPtr,
          std::owner_less< WeakAttributeDefinitionPtr > >
                                                      WeakAttributeDefinitionPtrSet;

  typedef std::set< WeakAttributeItemDefinitionPtr,
          std::owner_less< WeakAttributeItemDefinitionPtr > >
                                                      WeakAttributeItemDefinitionPtrSet;

  typedef std::set< WeakAttributeItemPtr,
          std::owner_less< WeakAttributeItemPtr > >   WeakAttributeItemPtrSet;
  typedef std::set< WeakAttributePtr,
          std::owner_less<WeakAttributePtr > >        WeakAttributePtrSet;
  typedef std::set< WeakModelItemPtr,
          std::owner_less<WeakModelItemPtr > >        WeakModelItemPtrSet;
  typedef std::set< WeakModelPtr,
          std::owner_less<WeakModelPtr > >            WeakModelPtrSet;
  namespace view
  {
    typedef std::set< view::WeakBasePtr,
      std::owner_less<view::WeakBasePtr > >            WeakViewPtrSet;
  };

#else
  //we can use less than operator
  typedef std::set< WeakAttributeDefinitionPtr  >     WeakAttributeDefinitionPtrSet;
  typedef std::set< WeakAttributeItemDefinitionPtr  > WeakAttributeItemDefinitionPtrSet;
  typedef std::set< WeakAttributeItemPtr  >           WeakAttributeItemPtrSet;
  typedef std::set< WeakAttributePtr  >               WeakAttributePtrSet;
  typedef std::set< WeakModelItemPtr  >               WeakModelItemPtrSet;
  typedef std::set< WeakModelPtr  >                   WeakModelPtrSet;
  namespace view
  {
    typedef std::set< view::WeakBasePtr  >              WeakBasePtrSet;
  };
#endif

  // These are used internally by SMTK
  namespace internal
  {
    template <typename T >
    struct is_shared_ptr
    {
      enum {type=false};
    };
    template<typename T >
    struct is_shared_ptr< smtk::shared_ptr< T >  >
    {
      enum{type=true};
    };

    template<typename T, int Enabled = is_shared_ptr< T >::type  >
    struct shared_ptr_type
    {
      typedef smtk::shared_ptr< T > SharedPointerType;
      typedef T RawPointerType;
    };

    template<typename T >
    struct shared_ptr_type<T,true >
    {
      typedef T SharedPointerType;
      typedef typename T::element_type RawPointerType;
    };

  }
}
#endif /* __smtk_PublicPointerDefs_h */
