
// Own
#include "astmerger.h"

// Qt
#include <QtCore/QIODevice>

TransformedSourcePrinter::TransformedSourcePrinter()
{
}
void TransformedSourcePrinter::visit(AST* rootNode)
{
    if (!rootNode)
        return;

    Q_ASSERT(nodeLookup());

    NodeLookup::Node node = nodeLookup()->lookup(rootNode);
    
    switch (node.type())
    {
        case NodeLookup::Node::UntransformedNode:
            // write out original content
            writeOriginalContent(node.ast());
            break;
        case NodeLookup::Node::ChildrenChangedNode:
        case NodeLookup::Node::NewNode:
            // visit with pretty printer
            PrettyPrintVisitor::visit(node.ast());
            break;
    }
}
void TransformedSourcePrinter::writeOriginalContent(AST* node)
{
    Q_ASSERT(tokenLookup());
    Q_ASSERT(node);
    Q_ASSERT(tokenLookup()->hasOriginalContent(node));

    QByteArray content = tokenLookup()->originalContent(node); 
    device()->write(content);
}


