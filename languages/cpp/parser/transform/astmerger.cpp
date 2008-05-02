
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
 
    QString state;
    if (node.type() == NodeLookup::Node::UntransformedNode)
        state = "Untransformed";
    else if (node.type() == NodeLookup::Node::ChildrenChangedNode)
        state = "ChildrenChanged";
    else if (node.type() == NodeLookup::Node::NewNode)
        state = "New";

    qDebug() << "TransformedSourcePrinter visiting " << AST::kindNames[rootNode->kind] << 
    "state = " << state;

    switch (node.type())
    {
        case NodeLookup::Node::UntransformedNode:
            // write out original content
            writeOriginalContent(node.ast());
            break;
        case NodeLookup::Node::ChildrenChangedNode:
            writeChildrenChangedNode(node.ast());
            break;
        case NodeLookup::Node::NewNode:
            // visit with pretty printer
            PrettyPrintVisitor::visit(node.ast());
            break;
    }
}
void TransformedSourcePrinter::writeChildrenChangedNode(AST* node)
{
    PrettyPrintVisitor::visit(node);
}
void TransformedSourcePrinter::writeOriginalContent(AST* node)
{
    Q_ASSERT(tokenLookup());
    Q_ASSERT(node);
    Q_ASSERT(tokenLookup()->hasOriginalContent(node));

    QByteArray content = tokenLookup()->originalContent(node).trimmed(); 
    printer()->printRaw(content);
}


