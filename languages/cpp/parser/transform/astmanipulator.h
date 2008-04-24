
#ifndef ASTMANIPULATOR_H
#define ASTMANIPULATOR_H

#include "ast.h"
#include "default_visitor.h"
#include "mergedvisitor.h"

class KDEVCPPPARSER_EXPORT ASTChanges
{
public:
    void setReplacement(AST* oldNode, AST* newNode, const QStack<AST*> nodeStack)
    {
        Q_ASSERT(oldNode);
        Q_ASSERT(newNode);

        m_replacements[oldNode] = newNode; 
        for (int i=nodeStack.count()-1;i>=0;i--)
            m_childrenChanged.insert(nodeStack[i]);
    }
    QHash<AST*,AST*> replacements() const
    { return m_replacements; }
    QSet<AST*> childrenChangedNodes() const
    { return m_childrenChanged; }

    void addToLookup(NodeLookup& lookup) const
    {      
        foreach(AST* key, m_replacements.keys()) 
        {
            NodeLookup::Node replacement(NodeLookup::Node::NewNode,
                                         m_replacements.value(key));
            lookup.setReplacement(key,replacement); 
        }
        foreach(AST* key, m_childrenChanged)
            lookup.setChildrenChanged(key,true);
    }   

private:
    QHash<AST*,AST*> m_replacements;
    QSet<AST*> m_childrenChanged;
};

/** 
 * Base class for manipulators which transform an AST 
 * according to various rules.
 */
class KDEVCPPPARSER_EXPORT ASTManipulator : protected DefaultVisitor
{
public:
    ~ASTManipulator();

    /** 
	 * Processes a translation unit and creates a map of 
	 */
	ASTChanges createChanges(AST* node);

    void setTokenLookup(MergedTokenLookup* lookup)
    { m_tokenLookup = lookup; }
    MergedTokenLookup* tokenLookup() const
    { return m_tokenLookup; }

protected:
    ASTManipulator();
    
    void addReplacement(AST* oldNode, AST* newNode);
    template <class T>
    T* newNode()
    { 
        T* node = CreateNode<T>(m_pool);
        node->start_token = -1;
        node->end_token = -1;
        return node;
    }

    // reimplemented 
    virtual void visit(AST* node);

private:
    ASTChanges m_changes;
    QStack<AST*> m_nodeStack;
    MergedTokenLookup* m_tokenLookup;
    pool* m_pool;
};

class KDEVCPPPARSER_EXPORT RenameManipulator : public ASTManipulator
{
public:
    void setName(const QString& oldName)
    { m_oldName = oldName; }
    void setNewName(const QString& newName)
    { m_newName = newName; }

protected:
	void visitUnqualifiedName(UnqualifiedNameAST* name);

    QString m_oldName;
    QString m_newName;
};

class KDEVCPPPARSER_EXPORT FunctionParameterManipulator : public ASTManipulator
{
private:
	void rewriteDeclaration(SimpleDeclarationAST* declaration);
	void rewriteDefinition(FunctionDefinitionAST* definition);
	void rewriteCall(PostfixExpressionAST* callNode);
};

class KDEVCPPPARSER_EXPORT ExtractMethodManipulator : public ASTManipulator
{
private:
	void extractExpression();
};

#endif // ASTMANIPULATOR_H

