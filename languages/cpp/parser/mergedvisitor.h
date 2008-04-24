
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
	virtual QString tokenString(AST* node, std::size_t index) const;

    virtual bool hasOriginalContent(AST* node) const = 0;
    virtual QByteArray originalContent(AST* node) const = 0;
};

class KDEVCPPPARSER_EXPORT TokenStreamTokenLookup : public TokenLookup
{
public:
	TokenStreamTokenLookup(TokenStream* stream);
	virtual const Token* token(AST* node, std::size_t index) const;

    virtual bool hasOriginalContent(AST* node) const;
    virtual QByteArray originalContent(AST* node) const;

protected:
	TokenStream* m_tokens;
};

class KDEVCPPPARSER_EXPORT MergedTokenLookup : public TokenStreamTokenLookup
{
public:
    MergedTokenLookup(TokenStream* sourceStream);
    int createNewToken(int kind, const QString& text);

    // reimplemented
    virtual const Token* token(AST* node, std::size_t index) const;
    virtual QString tokenString(AST* node, std::size_t index) const;
    virtual bool hasOriginalContent(AST* node) const;

private:
    QHash<int,Token> m_newTokens;
    int m_newTokenIndex;
    QString m_contentBuffer;
};

class KDEVCPPPARSER_EXPORT NodeLookup
{
public:
	virtual ~NodeLookup() {}

	class KDEVCPPPARSER_EXPORT Node
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
    
    void setReplacement(AST* node, Node replacement);
    QHash<AST*,Node> replacements() const;

    void setChildrenChanged(AST* node, bool changed);

private:
	QHash<AST*,Node> m_replacements;
    QSet<AST*> m_childrenChangedSet;
};

inline NodeLookup::Node::Type NodeLookup::Node::type() const
{ return m_type; }
inline AST* NodeLookup::Node::ast() const
{ return m_ast; }

class KDEVCPPPARSER_EXPORT MergedVisitor : public DefaultVisitor
{
public:
    MergedVisitor();

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

