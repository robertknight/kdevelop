/*
 * KDevelop C++ Highlighting Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVCPPHIGHLIGHTING_H
#define KDEVCPPHIGHLIGHTING_H

#include <QObject>
#include <QHash>
#include <QModelIndex>
#include <vector>
#include <duchain/duchainpointer.h>

#include <ktexteditor/attribute.h>

#include <icodehighlighting.h>

namespace KTextEditor { class SmartRange; }

namespace KDevelop 
{
class DUContext;
class Declaration;
}

///@todo make the colorized highlighting work while smart-conversion

typedef QVector<KDevelop::Declaration*> ColorMap;

class CppHighlighting : public QObject, public KDevelop::ICodeHighlighting
{
  Q_OBJECT
  Q_INTERFACES(KDevelop::ICodeHighlighting)

  public:
    enum Types {
      UnknownType,
      //Primary highlighting:
      LocalClassMemberType,
      InheritedClassMemberType,
      LocalVariableType,

      //Other highlighting:
      ClassType,
      FunctionType,
      ForwardDeclarationType,
      EnumType,
      EnumeratorType,
      TypeAliasType,

      //If none of the above match:
      MemberVariableType,
      NamespaceVariableType,
      GlobalVariableType,
      
      //Most of these are currently not used:
      ArgumentType,
      CodeType,
      FileType,
      NamespaceType,
      ScopeType,
      TemplateType,
      TemplateParameterType,
      FunctionVariableType,
      ErrorVariableType
    };

    enum Contexts {
      DefinitionContext,
      DeclarationContext,
      ReferenceContext
    };

    CppHighlighting(QObject* parent);
    virtual ~CppHighlighting();

    void highlightTree(KTextEditor::SmartRange* topRange) const;
    void highlightDUChain(KDevelop::TopDUContext* context) const;

    void deleteHighlighting(KDevelop::DUContext* context) const;
  
    virtual void highlightDeclaration(KDevelop::Declaration* declaration, uint color) const;
    virtual void highlightUse(KDevelop::DUContext* context, int index, uint color) const;
    virtual void highlightDeclaration(KDevelop::Declaration* declaration) const;
    virtual void highlightUses(KDevelop::DUContext* context) const;

    //color should be zero when undecided
    KTextEditor::Attribute::Ptr attributeForType(Types type, Contexts context, uint color ) const;
    KTextEditor::Attribute::Ptr attributeForDepth(int depth) const;

  private:
    void highlightDUChainSimple(KDevelop::DUContext* context) const;
    void highlightDUChain(KDevelop::DUContext* context, QHash<KDevelop::Declaration*, uint> colorsForDeclarations, ColorMap) const;
    void outputRange( KTextEditor::SmartRange * range ) const;

    /**
     * @param context Should be the context from where the declaration is used, if a use is highlighted.
     * */
    Types typeForDeclaration(KDevelop::Declaration* dec, KDevelop::DUContext* context) const;

    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_definitionAttributes;
    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_declarationAttributes;
    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_referenceAttributes;

    mutable QList<KTextEditor::Attribute::Ptr> m_depthAttributes;

    //Here the colors of function context are stored until they are merged into the function body
    mutable QMap<KDevelop::DUContextPointer, QHash<KDevelop::Declaration*, uint> > m_functionColorsForDeclarations;
    mutable QMap<KDevelop::DUContextPointer, ColorMap> m_functionDeclarationsForColors;

  //Should be used to enable/disable the colorization of local variables and their uses
  bool m_localColorization;
};

#endif
