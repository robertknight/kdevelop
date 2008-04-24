
#ifndef MERGEDVISITOR_H
#define MERGEDVISITOR_H

#include <QtCore/QHash>
#include "default_visitor.h"
#include "lexer.h"
#include "ast.h"

class KDEVCPPPARSER_EXPORT TokenLookup
{
public:
	virtual ~TokenLookup() {}

	virtual const Token* token(AST* node, std::size_t index) const = 0; 
	QString tokenString(AST* node, std::size_t index) const;
};

class KDEVCPPPARSER_EXPORT TokenStreamTokenLookup : public TokenLookup
{
public:
	TokenStreamTokenLookup(TokenStream* stream);
	virtual const Token* token(AST* node, std::size_t index) const;
private:
	TokenStream* m_tokens;
};

class KDEVCPPPARSER_EXPORT NodeLookup
{
public:
	virtual ~NodeLookup() {}

	class Node
	{
	public:
		enum Type
		{
			UntransformedNode,
			ChildrenChangedNode,
			NewNode,
		};
		
		Node();
		Node(Type type, AST* node); 

		Type type() const;
		AST* ast() const;

	private:
		Type m_type;
		AST* m_ast;
	};

	virtual Node lookup(AST* node) const;

private:
	QHash<AST*,Node> m_replacements;
};

inline NodeLookup::Node::Type NodeLookup::Node::type() const
{ return m_type; }
inline AST* NodeLookup::Node::ast() const
{ return m_ast; }

class KDEVCPPPARSER_EXPORT MergedVisitor : public DefaultVisitor
{
public:
	void setNodeLookup(NodeLookup* lookup) 
	{ m_nodeLookup = lookup; }

	NodeLookup* nodeLookup() const
	{ return m_nodeLookup; }

	void setTokenLookup(TokenLookup* lookup)
	{ m_tokenLookup = lookup; }
	TokenLookup* tokenLookup() const
	{ return m_tokenLookup; }

	virtual void visit(AST* node);

protected:
	NodeLookup* m_nodeLookup;
	TokenLookup* m_tokenLookup;

};

#endif // MERGEDVISITOR_H

