
/** 
 * Base class for manipulators which transform an AST 
 * according to various rules.
 */
class ASTManipulator
{
public:
	/** 
	 * Processes a translation unit and creates a map of 
	 */
	virtual ASTChanges createChanges(AST* node) = 0;
};

class RenameManipulator : public ASTManipulator
{
public:
	virtual ASTChanges createChanges(AST* node);

private:
	void rewriteUnqualifiedName(UnqualifiedNameAST* name);
};

class FunctionParameterManipulator : public ASTManipulator
{
public:
	virtual ASTChanges createChanges(AST* node);

private:
	void rewriteDeclaration(SimpleDeclarationAST* declaration);
	void rewriteDefinition(FunctionDefinitionAST* definition);
	void rewriteCall(PostfixExpressionAST* callNode);
};

class ExtractMethodManipulator : public ASTManipulator
{
public:
	virtual ASTChanges createChanges(AST* node);

private:
	void extractExpression();
	void extractStatements(const QList<Statement
};

#endif // ASTMANIPULATOR_H

