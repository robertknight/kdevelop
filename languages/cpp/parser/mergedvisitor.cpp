
#include "mergedvisitor.h"

void MergedVisitor::visit(AST* node)
{
	DefaultVisitor::visit(m_nodeLookup->lookup(node).ast());
}

NodeLookup::Node::Node(Type type, AST* node)
: m_type(type), m_ast(node)
{
}
NodeLookup::Node::Node() : m_type(UntransformedNode) , m_ast(0) {}
NodeLookup::Node NodeLookup::lookup(AST* node) const
{
	if (m_replacements.contains(node))
		return m_replacements[node];
	else
	{
		return Node(Node::UntransformedNode,node);
	}
}

