
#ifndef ASTMERGER_H
#define ASTMERGER_H

/** 
 * Prints out transformed source code.
 */
class TransformedSourcePrinter 
{
public:
    TransformedSourcePrinter();
	void setNodeLookup(NodeLookup* lookup);
	void write(QIODevice* device, AST* rootNode);

private:
	NodeLookup* m_lookup;
    PrettyPrintVisitor m_prettyPrinter;
};

#endif // ASTMERGER_H
