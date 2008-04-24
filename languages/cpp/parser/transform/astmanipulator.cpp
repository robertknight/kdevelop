
// Own
#include "transform/astmanipulator.h"

// Local
#include "tokens.h"

ASTManipulator::ASTManipulator()
: m_tokenLookup(0)
, m_pool(new pool)
{
}
ASTManipulator::~ASTManipulator()
{
    delete m_pool;
}

ASTChanges ASTManipulator::createChanges(AST* node)
{
    m_changes = ASTChanges();
    visit(node);
    return m_changes;
}
void ASTManipulator::visit(AST* node)
{
    m_nodeStack.push(node);
    DefaultVisitor::visit(node);
    m_nodeStack.pop();
}
void ASTManipulator::addReplacement(AST* oldNode, AST* newNode)
{
    m_changes.setReplacement(oldNode,newNode,m_nodeStack);
}

void RenameManipulator::visitUnqualifiedName(UnqualifiedNameAST* name)
{
    if (tokenLookup()->tokenString(name,name->id) == m_oldName)
    {
        UnqualifiedNameAST* newNameNode = newNode<UnqualifiedNameAST>();
        newNameNode->start_token = tokenLookup()->createNewToken(Token_identifier,m_newName);
        newNameNode->end_token = newNameNode->start_token;
        newNameNode->id = newNameNode->start_token;
        newNameNode->tilde = name->tilde;
        newNameNode->operator_id = name->operator_id;
        newNameNode->template_arguments = name->template_arguments;

        addReplacement(name,newNameNode);

        qDebug() << "Replacing name node at token " << name->start_token;
    }
}


