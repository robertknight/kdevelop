
#include "mergedvisitor.h"
#include "parsesession.h"

MergedVisitor::MergedVisitor()
: m_nodeLookup(0),
  m_tokenLookup(0)
{
}
void MergedVisitor::visit(AST* node)
{
    Q_ASSERT(m_nodeLookup);
	DefaultVisitor::visit(m_nodeLookup->lookup(node).ast());
}

TokenStreamTokenLookup::TokenStreamTokenLookup(TokenStream* stream)
: m_tokens(stream)
{}
QString TokenLookup::tokenString(AST* node, std::size_t index) const
{
	return token(node,index)->symbol();
}
const Token* TokenStreamTokenLookup::token(AST*, std::size_t index) const
{
	return &m_tokens->token(index); 
}
bool TokenStreamTokenLookup::hasOriginalContent(AST*) const
{ return true; }
QByteArray TokenStreamTokenLookup::originalContent(AST* node) const
{
    const Token* startToken = token(node,node->start_token);
    const Token* endToken = token(node,node->end_token);

    Q_ASSERT(startToken);
    Q_ASSERT(endToken);
    Q_ASSERT(startToken->session == endToken->session);
   
    const char* contents = startToken->session->contents();
  
    Q_ASSERT(endToken->position <= startToken->session->size());

    QByteArray result = QByteArray(contents + startToken->position,
                      endToken->position - startToken->position);

    qDebug() << "Original content for node" << AST::kindNames[node->kind] << result;

    return result;
}
MergedTokenLookup::MergedTokenLookup(TokenStream* sourceStream)
: TokenStreamTokenLookup(sourceStream)
{
    m_newTokenIndex = sourceStream->size(); 
}
int MergedTokenLookup::createNewToken(int kind, const QString& text)
{
    int index = m_newTokenIndex++;

    Token newToken;
    newToken.kind = kind;
    newToken.position = m_contentBuffer.count(); 
    newToken.size = text.count();
    newToken.session = 0;

    m_contentBuffer.append(text);

    m_newTokens.insert(index,newToken);

    return index; 
}
QString MergedTokenLookup::tokenString(AST* node, std::size_t index) const
{
    if (m_newTokens.contains(index))
    {
        Token t = m_newTokens[index];
        qDebug() << "Looking up new token, value is " << m_contentBuffer.mid(t.position,t.size);
        return m_contentBuffer.mid(t.position,t.size);
    }
    else
        return TokenStreamTokenLookup::tokenString(node,index); 
}
const Token* MergedTokenLookup::token(AST* node, std::size_t index) const
{
    if (m_newTokens.contains(index))
        return &const_cast<MergedTokenLookup*>(this)->m_newTokens[index];
    else
        return TokenStreamTokenLookup::token(node,index);
}
bool MergedTokenLookup::hasOriginalContent(AST* node) const
{
    return node->start_token < m_tokens->size();
}

void NodeLookup::setReplacement(AST* node, Node replacement)
{
    Q_ASSERT(node);
    Q_ASSERT(replacement.ast());

    m_replacements.insert(node,replacement);
}
QHash<AST*,NodeLookup::Node> NodeLookup::replacements() const
{ return m_replacements; }
NodeLookup::Node::Node(Type type, AST* node)
: m_type(type), m_ast(node)
{
}
void NodeLookup::setChildrenChanged(AST* node, bool changed)
{
    if (changed)
        m_childrenChangedSet.insert(node);
    else
        m_childrenChangedSet.remove(node);
}
NodeLookup::Node::Node() : m_type(UntransformedNode) , m_ast(0) {}
NodeLookup::Node NodeLookup::lookup(AST* node) const
{
	if (m_replacements.contains(node))
    {
        qDebug() << "Looking up replacement for node" << node;
		return m_replacements[node];
    }
	else if (m_childrenChangedSet.contains(node))
        return Node(Node::ChildrenChangedNode,node);
    else
    {
		return Node(Node::UntransformedNode,node);
	}
}

