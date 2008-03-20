/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPTYPES_H
#define CPPTYPES_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QPair>

#include <duchain/identifier.h>
#include <typesystem.h>
#include <declaration.h>
#include <identifiedtype.h>
#include "cppduchainexport.h"

namespace KDevelop
{
class DUContext;
class Declaration;
}

template<typename T>
T constant_value(const qint64* realval)
{
  T value;
  memcpy(&value, realval, sizeof(T));
  return value;
}

class TemplateParameterDeclaration;

class KDEVCPPDUCHAIN_EXPORT CppCVType
{
  friend class TypeRepository;
  friend class TypeBuilder;

public:
  CppCVType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  inline bool isConstant() const { return m_constant; }

  inline bool isVolatile() const { return m_volatile; }

  void clear();
  
  QString cvString() const;
  QString cvMangled() const;

  uint cvHash(uint input) const { return input + (m_constant ? 7 : 0) + (m_volatile ? 3 : 0); }

  KDevelop::Declaration::CVSpecs cv() const { return static_cast<KDevelop::Declaration::CVSpecs>((m_constant & KDevelop::Declaration::Const) | (m_volatile & KDevelop::Declaration::Volatile)); }

  bool equals(const CppCVType* rhs) const;
  
  inline void setCV(KDevelop::Declaration::CVSpecs spec) { m_constant = spec & KDevelop::Declaration::Const; m_volatile = spec & KDevelop::Declaration::Volatile; }
protected:
  inline void setConstant(bool is) { m_constant = is; }
  inline void setVolatile(bool is) { m_volatile = is; }

private:
  bool m_constant : 1;
  bool m_volatile : 1;
};

class KDEVCPPDUCHAIN_EXPORT CppIntegralType : public KDevelop::IntegralType, public CppCVType
{
  friend class TypeRepository;

public:
  typedef KSharedPtr<CppIntegralType> Ptr;

  enum IntegralTypes {
    TypeNone,
    TypeChar,
    TypeWchar_t,  // C++ only
    TypeBool, // C++ only
    TypeInt,
    TypeFloat,
    TypeDouble,
    TypeVoid
  };

  IntegralTypes integralType() const;

  enum TypeModifier {
    ModifierNone      = 0x0,
    ModifierShort     = 0x1,
    ModifierLong      = 0x2,
    ModifierLongLong  = 0x4,
    ModifierSigned    = 0x8,
    ModifierUnsigned  = 0x10
  };
  Q_DECLARE_FLAGS(TypeModifiers, TypeModifier)

  TypeModifiers typeModifiers() const;

  virtual QString toString() const;

  virtual QString mangled() const;

  virtual uint hash() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;
  
protected:
  CppIntegralType(IntegralTypes type, CppIntegralType::TypeModifiers modifiers = ModifierNone);

private:
  IntegralTypes m_type;
  CppIntegralType::TypeModifiers m_modifiers;
};

/**
 * An integral type that additionally has a constant value
 * */
class KDEVCPPDUCHAIN_EXPORT CppConstantIntegralType : public CppIntegralType
{
  friend class TypeRepository;

public:
  typedef KSharedPtr<CppConstantIntegralType> Ptr;

  /**The types and modifiers are not changed!
   
  * Implemented versions are:
   * setValue<qint64>(),setValue<quint64>(), setValue<float>(), setValue<double>()
   *
   * */
  template<class ValueType>
  void setValue(ValueType value);

  /**
   * For booleans, the value is 1 for true, and 0 for false.
   * All signed values should be retrieved and set through value(),
   * unsigned ones through unsignedValue().
   *
   * Implemented versions are:
   * value<qint64>(),value<quint64>(), value<float>(), value<double>()
   * 
   * */
  template<class Type>
  Type value() const {
    return constant_value<Type>(&m_value);
  }

  virtual QString toString() const;

  virtual AbstractType* clone() const;

  virtual uint hash() const;
  
  CppConstantIntegralType(IntegralTypes type, CppIntegralType::TypeModifiers modifiers = ModifierNone);
private:
  qint64 m_value;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CppIntegralType::TypeModifiers)

class KDEVCPPDUCHAIN_EXPORT CppFunctionType : public KDevelop::FunctionType, public KDevelop::IdentifiedType, public CppCVType
{
public:
  typedef KSharedPtr<CppFunctionType> Ptr;

  ///Declarations of this class(@see KDevelop::IdentifiedType::declaration()) are guaranteed to be based on AbstractFunctionDeclaration
  
  virtual QString toString() const;

  virtual uint hash() const;

  virtual QString mangled() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;
};

class KDEVCPPDUCHAIN_EXPORT CppPointerType : public KDevelop::PointerType, public CppCVType
{
public:
  typedef KSharedPtr<CppPointerType> Ptr;

  CppPointerType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  virtual QString toString() const;

  virtual QString mangled() const;

  virtual AbstractType* clone() const;
  
  virtual bool equals(const AbstractType* rhs) const;

  virtual uint hash() const;
};

class KDEVCPPDUCHAIN_EXPORT CppReferenceType : public KDevelop::ReferenceType, public CppCVType
{
public:
  typedef KSharedPtr<CppReferenceType> Ptr;

  CppReferenceType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  virtual QString toString() const;

  virtual QString mangled() const;

  virtual AbstractType* clone() const;
  
  virtual bool equals(const AbstractType* rhs) const;

  virtual uint hash() const;
};

class KDEVCPPDUCHAIN_EXPORT CppClassType : public KDevelop::StructureType, public KDevelop::IdentifiedType, public CppCVType
{
public:
  typedef KSharedPtr<CppClassType> Ptr;

  CppClassType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  struct BaseClassInstance
  {
    AbstractType::Ptr baseClass; //May either be CppClassType, or CppDelayedType
    KDevelop::Declaration::AccessPolicy access;
    bool virtualInheritance;
  };

  

  const QList<BaseClassInstance>& baseClasses() const;
  void addBaseClass(const BaseClassInstance& baseClass);
  void removeBaseClass(AbstractType::Ptr baseClass);

  enum ClassType
  {
    Class,
    Struct,
    Union
  };

  void setClassType(ClassType type);
  ClassType classType() const;

  /// C++ classes are closed types, once they are defined, they can't be changed.
  bool isClosed() const { return m_closed; }
  void close() { m_closed = true; }

  ///After clearing, a class-type is open again.
  void clear(); 

  /// Error if the type is closed.
  virtual void addElement(KDevelop::AbstractType::Ptr element);

  virtual uint hash() const;

  virtual QString toString() const;

  virtual QString mangled() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;
  
  virtual void accept0 (KDevelop::TypeVisitor *v) const;

  virtual void exchangeTypes(KDevelop::TypeExchanger*);
  
private:
  QList<BaseClassInstance> m_baseClasses;
  ClassType m_classType;
  bool m_closed;
};

/*class CppArrayType : public ArrayType, public CppCVType
{
public:
  typedef KSharedPtr<CppArrayType> Ptr;

  CppArrayType(Declaration::CVSpecs spec = Declaration::CVNone);

  virtual QString toString() const;
};*/

class KDEVCPPDUCHAIN_EXPORT CppTypeAliasType : public KDevelop::AbstractType, public KDevelop::IdentifiedType, public CppCVType
{
public:
  typedef KSharedPtr<CppTypeAliasType> Ptr;

  CppTypeAliasType();

  KDevelop::AbstractType::Ptr type() const;
  void setType(KDevelop::AbstractType::Ptr type);

  virtual uint hash() const;

  virtual QString toString() const;

  virtual QString mangled() const;

  virtual AbstractType* clone() const;
  
  virtual bool equals(const AbstractType* rhs) const;
  
protected:
  virtual void accept0 (KDevelop::TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (m_type, v);

    //v->endVisit (this);
  }

private:
  KDevelop::AbstractType::Ptr m_type;
};

class KDEVCPPDUCHAIN_EXPORT CppEnumeratorType : public CppConstantIntegralType
{
public:
  typedef KSharedPtr<CppEnumeratorType> Ptr;

  CppEnumeratorType();
};

class KDEVCPPDUCHAIN_EXPORT CppEnumerationType : public CppIntegralType, public KDevelop::IdentifiedType
{
public:
  typedef KSharedPtr<CppEnumerationType> Ptr;

  CppEnumerationType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  virtual uint hash() const;

  virtual AbstractType* clone() const;
  
  virtual bool equals(const AbstractType* rhs) const;

  virtual QString mangled() const;
  
  virtual QString toString() const;
};

class KDEVCPPDUCHAIN_EXPORT CppArrayType : public KDevelop::ArrayType
{
public:
  typedef KSharedPtr<CppArrayType> Ptr;

  virtual QString mangled() const;

  virtual AbstractType* clone() const;
  
  virtual bool equals(const AbstractType* rhs) const;
};

/**
 * This class represents a template-parameter on the type-level(it is strictly attached to a template-declaration)
 * This is only attached to unset template-parameters. Once the template-parameters are set, the TemplateDeclarations
 * will return the real set types as abstractType().
 * This means that when you encounter this type, it means that the template-parameter is not set.
 * */
class KDEVCPPDUCHAIN_EXPORT CppTemplateParameterType : public KDevelop::AbstractType, public KDevelop::IdentifiedType
{
public:
  typedef KSharedPtr<CppTemplateParameterType> Ptr;

  TemplateParameterDeclaration* declaration() const;

  virtual QString toString() const;
  virtual QString mangled() const;

  virtual AbstractType* clone() const;
  
  virtual bool equals(const AbstractType* rhs) const;

  virtual uint hash() const;
  
  protected:
  virtual void accept0 (KDevelop::TypeVisitor *v) const;
  private:
};

/*template <class _Target, class _Source>
_Target model_static_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;

  _Target ptr = static_cast<_Target_pointer>(item.data());
  return ptr;
}

template <class _Target, class _Source>
_Target model_safe_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && source->kind() == _Target_pointer(0)->__node_kind)
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}

template <typename _Target, typename _Source>
_Target model_dynamic_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && (source->kind() == _Target_pointer(0)->__node_kind
         || (_Target_pointer(0)->__node_kind <= int(_CodeModelItem::KindMask)
             && ((source->kind() & _Target_pointer(0)->__node_kind)
                  == _Target_pointer(0)->__node_kind))))
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}*/


#endif // CPPTYPES_H

