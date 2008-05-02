
#ifndef ASTMERGER_H
#define ASTMERGER_H

#include "ast.h"
#include "prettyprintvisitor.h"

class QIODevice;

/** 
 * Prints out transformed source code.
 */
class KDEVCPPPARSER_EXPORT TransformedSourcePrinter : public PrettyPrintVisitor 
{
public:
    TransformedSourcePrinter();

    virtual void visit(AST* node);

private:
    void writeOriginalContent(AST* node);
    void writeChildrenChangedNode(AST* node);
};

#endif // ASTMERGER_H
