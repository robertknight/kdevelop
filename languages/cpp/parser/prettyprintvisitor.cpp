/* This file is part of KDevelop
    Copyright 2008 Robert Knight <robertknight@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation
   or, at your option, any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tokens.h"
#include "prettyprintvisitor.h"

QString TokenLookup::tokenString(AST* node, std::size_t index) const
{
	return token(node,index)->symbol();
}

TokenStreamTokenLookup::TokenStreamTokenLookup(TokenStream* stream)
: m_tokens(stream)
{}

const Token* TokenStreamTokenLookup::token(AST*, std::size_t index) const
{
	return &m_tokens->token(index); 
}

PrettyPrintWriter& PrettyPrintWriter::operator<<(int tokenType)
{
	print(tokenType,token_string(tokenType));		
	return *this;
}
PrettyPrintWriter& PrettyPrintWriter::operator<<(const QString& text)
{
	print(Token_identifier,text);
	return *this;
}
void SimplePrinter::reset(QTextStream* output)
{
	m_output = output;
	m_lastWasAlphaNum = false;
	m_inTemplate = false;
	m_lastTokenType = 0;
	m_indentation = 0;
	m_currentLine.clear();
	m_lineOutput.setString(&m_currentLine);
	m_topNode = 0;
}
void SimplePrinter::enter(AST* node)
{
	if (!m_topNode)
		m_topNode = node;

	switch (node->kind)
	{
		case AST::Kind_TemplateDeclaration:
			m_inTemplate = true;
			break;
		case AST::Kind_SimpleDeclaration:
		case AST::Kind_FunctionDefinition:
			if (m_inTemplate)
				newLine();
			break;
		case AST::Kind_CtorInitializer:
			newLine();
			break;
		case AST::Kind_AccessSpecifier:
			if (m_lastTokenType != '{')
				m_indentation--;
			break;
	};
}
void SimplePrinter::leave(AST* node)
{
	switch (node->kind)
	{
		case AST::Kind_AccessSpecifier:
			newLine();
			m_indentation++;
			break;
		case AST::Kind_TemplateDeclaration:
			m_inTemplate = false;
			break;
	};

	if (node == m_topNode)
		newLine();
}
void SimplePrinter::print(int tokenType, const QString& text)
{
	bool alphaNum = false;
	for (int i=0;i<text.count();i++)
		if (text[i].isLetterOrNumber())
		{
			alphaNum = true;
			break;
		}

	// whitespace based on previous token
	if (m_lastTokenType == '{')
	{
		newLine();
		m_indentation++;
	}
	else if (m_lastTokenType == '}')
	{
		if (tokenType != ';')
			newLine();
	}
	else if (m_lastTokenType == ';')
	{
		if (tokenType != '}')
			newLine();
	}
	else if (m_lastTokenType == ',')
	{
		m_lineOutput << ' ';
	}

	bool 	requiresSpace = alphaNum && m_lastWasAlphaNum;
			requiresSpace = requiresSpace || (m_lastTokenType == '>' && tokenType == '>');
			requiresSpace = requiresSpace || (m_lastTokenType == '>' && tokenType == '=');


	// whitespace based on current token 
	if (requiresSpace)
		m_lineOutput << ' ';
	else if (tokenType == '{')
	{
		newLine();
	}
	else if (tokenType == '}')
	{
		newLine();
		m_indentation--;
	}

	m_lineOutput << text;

	m_lastWasAlphaNum = alphaNum;
	m_lastTokenType = tokenType;
}
void SimplePrinter::newLine()
{
	if (m_indentation > 0)
		*m_output << QString(m_indentation,'\t');
	*m_output << m_currentLine;
	m_currentLine.clear();
	*m_output << '\n';
}

PrettyPrintVisitor::PrettyPrintVisitor()
: m_tokenLookup(0), m_printer(0) , m_currentIndentation(0)
{}
void PrettyPrintVisitor::write(QIODevice* device, AST* node)
{
	Q_ASSERT(tokenLookup());
	Q_ASSERT(printer());

	m_outputStream.setDevice(device);
	m_printer->reset(&m_outputStream);
	visit(node);
}
void PrettyPrintVisitor::visit(AST* node)
{
	if (!node)
		return;
	m_nodeStack.push(node);
	m_printer->enter(node);
	DefaultVisitor::visit(node);
	m_printer->leave(node);
	m_nodeStack.pop();
}
void PrettyPrintVisitor::visitCompoundStatement(CompoundStatementAST* node)
{
	*m_printer << '{';
	DefaultVisitor::visitCompoundStatement(node);
	*m_printer << '}';
}
void PrettyPrintVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
	visitTypeSpecifier(node);
	writeTokenList(node,node->integrals);
	DefaultVisitor::visitSimpleTypeSpecifier(node);
}
void PrettyPrintVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)
{
	if (node->tilde != 0)
		*m_printer << '~';
	writeToken(node,node->id);
	DefaultVisitor::visitUnqualifiedName(node);
}
void PrettyPrintVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
	if (node->typeKnown)
	{
		if (node->expressionChosen)
			visit(node->expression);
		else 
			visit(node->declaration);
	}
	else
	{
		qWarning() << 	"Pretty-printing ambiguous statement which may be an expression"
						"(a function call) or a declaration (of the form T(initializer))\n"
						"Treating statement as an expression for now.";
		visit(node->expression);
	}
}
void PrettyPrintVisitor::visitParameterDeclarationClause(ParameterDeclarationClauseAST* node)
{
	*m_printer << '(';
	
	bool hasDeclarations = node->parameter_declarations != 0;
	const ListNode<ParameterDeclarationAST*>* declaration = 0;
	
	if (hasDeclarations)
		declaration = node->parameter_declarations->toFront();

	while (declaration != 0)
	{
		visitParameterDeclaration(declaration->element);

		if (declaration->hasNext())
		{
			declaration = declaration->next;
			*m_printer << ',';
		} else
			break;
	}
	
	if (node->ellipsis != 0)
	{
		if (hasDeclarations)
			*m_printer << ',';
		*m_printer << Token_ellipsis; 
	}
	*m_printer << ')';
}
int PrettyPrintVisitor::opPrecedence(int opKind)
{
	static const int precedence[] = 
		{ 	Token_throw, 
			Token_assign,
			Token_or,
			Token_and,
			'|',
			'^',
			'&',
			Token_not_eq,
			Token_eq,
			Token_geq,
			Token_leq,
			'>',
			'<',
			Token_shift, // '<<' has precedence over '>>'
			'-',
			'+',
			'%',
			'/',
			'*',
			Token_ptrmem, // '.*' has precedence over '->*'
			
			// cast expressions of the form (typeid)cast-expression go here
			
			Token_delete,
			Token_new,
			Token_sizeof,
			Token_decr,
			Token_incr,

			// postfix expressions go here
			
			0
		};
		
		for (int i=0;precedence[i] != 0;i++)
			if (opKind == precedence[i])
				return i;
		return -1;
}
bool PrettyPrintVisitor::higherPrecedenceInStack(int opKind) const
{
	int precedence = opPrecedence(opKind);
	for (int i = m_opStack.count()-1 ; i >= 0 ; i--)
		if (opPrecedence(m_opStack[i]) > precedence)
			return true;
	return false;
}
void PrettyPrintVisitor::visitBinaryExpression(BinaryExpressionAST* node)
{
	int opKind = m_tokenLookup->token(node,node->op)->kind;
	bool addParens = higherPrecedenceInStack(opKind);
	if (addParens)
		*m_printer << '(';

	m_opStack.push(opKind);

	visit(node->left_expression);
	writeToken(node,node->op);
	visit(node->right_expression);

	m_opStack.pop();

	if (addParens)
		*m_printer << ')';
}
void PrettyPrintVisitor::visitUnaryExpression(UnaryExpressionAST* node)
{
	int opKind = m_tokenLookup->token(node,node->op)->kind;
	bool addParens = higherPrecedenceInStack(opKind);
	if (addParens)
		*m_printer << '(';
	m_opStack.push(opKind);

	writeToken(node,node->op);
	visit(node->expression);

	m_opStack.pop();
	if (addParens)
		*m_printer << ')';
}
void PrettyPrintVisitor::visitTypeSpecifier(TypeSpecifierAST* node)
{
	writeTokenList(node,node->cv);
}
void PrettyPrintVisitor::visitAccessSpecifier(AccessSpecifierAST* node)
{
	writeTokenList(node,node->specs);
	*m_printer << ':';
}
void PrettyPrintVisitor::visitBaseClause(BaseClauseAST* node)
{
	*m_printer << ':';
	DefaultVisitor::visitBaseClause(node);
}
void PrettyPrintVisitor::visitBaseSpecifier(BaseSpecifierAST* node)
{
	if (node->virt);
		*m_printer << Token_virtual;
	writeToken(node,node->access_specifier);
	visit(node->name);
}
void PrettyPrintVisitor::visitCastExpression(CastExpressionAST* node)
{
	*m_printer << '(';
	visit(node->type_id);
	*m_printer << ')';
	*m_printer << '(';
	visit(node->expression);
	*m_printer << ')';
}
void PrettyPrintVisitor::visitCondition(ConditionAST* node)
{
	visit(node->expression);
}
void PrettyPrintVisitor::visitHandler(HandlerAST* node)
{
	*m_printer << Token_catch;
	*m_printer << '(';
	visit(node->declaration);
	*m_printer << ')';
	visit(node->body);
}
void PrettyPrintVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
	writeToken(node,node->op);
	visit(node->name);
}
void PrettyPrintVisitor::visitClassSpecifier(ClassSpecifierAST* node)
{
	visitTypeSpecifier(node);
	visit(node->win_decl_specifiers);
	writeToken(node,node->class_key);
	visit(node->name);
	visit(node->base_clause);
	*m_printer << '{';
	visitNodes(this,node->member_specs);
	*m_printer << '}';
}
void PrettyPrintVisitor::visitCppCastExpression(CppCastExpressionAST* node)
{
	writeToken(node,node->op);
	*m_printer << '<';
	visit(node->type_id);
	*m_printer << '>';
	*m_printer << '(';
	*m_printer << ')';

	visitListWithSeparator(node->sub_expressions,',','(',')');
}
void PrettyPrintVisitor::visitCtorInitializer(CtorInitializerAST* node)
{
	*m_printer << ':';
	visitListWithSeparator(node->member_initializers,',');
}
void PrettyPrintVisitor::visitDeclarationStatement(DeclarationStatementAST* node)
{
	visit(node->declaration);
}
void PrettyPrintVisitor::visitDeclarator(DeclaratorAST* node)
{
	visitNodes(this,node->ptr_ops);
	visit(node->sub_declarator);
	visit(node->id);
	visit(node->bit_expression);
	visitNodes(this,node->array_dimensions);
	visit(node->parameter_declaration_clause);
	writeTokenList(node,node->fun_cv);
	visit(node->exception_spec);
}
void PrettyPrintVisitor::visitDeleteExpression(DeleteExpressionAST* node)
{
	// ISSUE - Make use of node->scope_token
	*m_printer << Token_delete;
	
	if (node->lbracket_token)
		*m_printer << '[' << ']';
	
	visit(node->expression);
}
void PrettyPrintVisitor::visitDoStatement(DoStatementAST* node)
{
	*m_printer << Token_do;
	visit(node->statement);
	*m_printer << Token_while;
	*m_printer << '(';
	visit(node->expression);
	*m_printer << ')';
	*m_printer << ';';
}
void PrettyPrintVisitor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
	visitTypeSpecifier(node);
	writeToken(node,node->type);
	visit(node->name);
}
void PrettyPrintVisitor::visitEnumSpecifier(EnumSpecifierAST* node)
{
	visitTypeSpecifier(node);
	*m_printer << Token_enum;
	visit(node->name);
	*m_printer << '{';
	visitListWithSeparator(node->enumerators,',');
	*m_printer << '}';
	*m_printer << ';';
}
void PrettyPrintVisitor::visitEnumerator(EnumeratorAST* node)
{
	writeToken(node,node->id);
	if (node->expression)
	{
		*m_printer << '=';
		visit(node->expression);
	}
}
void PrettyPrintVisitor::visitExceptionDeclaration(ExceptionDeclarationAST* node)
{
	if (node->ellipsis)
		*m_printer << Token_ellipsis;
	else
	{
		visit(node->type_specifier);
		visit(node->declarator);
	}
}
void PrettyPrintVisitor::visitExceptionSpecification(ExceptionSpecificationAST* node)
{
	*m_printer << Token_throw;
	*m_printer << '(';
	if (node->ellipsis)
		*m_printer << Token_ellipsis;
	else
		visitListWithSeparator(node->type_ids,',');
	*m_printer << ')';
}
void PrettyPrintVisitor::visitExpressionStatement(ExpressionStatementAST* node)
{
	visit(node->expression);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitFunctionCall(FunctionCallAST* node)
{
	*m_printer << '(';
	visit(node->arguments);
	*m_printer << ')';
}
void PrettyPrintVisitor::visitFunctionDefinition(FunctionDefinitionAST* node)
{
	writeTokenList(node,node->storage_specifiers);
	writeTokenList(node,node->function_specifiers);

	visit(node->type_specifier);
	visit(node->init_declarator);
	
	if (node->function_body->kind == AST::Kind_FunctionTryBlock)
		*m_printer << Token_try;

	visit(node->constructor_initializers);
	visit(node->function_body);
	visit(node->win_decl_specifiers);
}
void PrettyPrintVisitor::visitFunctionTryBlock(FunctionTryBlockAST* node)
{
	visit(node->body);
	visitNodes(this,node->handlers);
}
void PrettyPrintVisitor::visitForStatement(ForStatementAST* node)
{
	*m_printer << Token_for;
	*m_printer << '(';
	visit(node->init_statement);
	visit(node->condition);
	*m_printer << ';';
	visit(node->expression);
	*m_printer << ')';
	visit(node->statement);
}
void PrettyPrintVisitor::visitIfStatement(IfStatementAST* node)
{
	*m_printer << Token_if;
	*m_printer << '(';
	visit(node->condition);
	*m_printer << ')';
	visit(node->statement);

	if (node->else_statement)
	{
		*m_printer << Token_else;
		visit(node->else_statement);
	}
}
void PrettyPrintVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)
{
	writeToken(node,node->op);
}
void PrettyPrintVisitor::visitInitDeclarator(InitDeclaratorAST* node)
{
	visit(node->declarator);

	if (node->initializer)
	{
		*m_printer << '=';
		visit(node->initializer);
	}
}
void PrettyPrintVisitor::visitInitializer(InitializerAST* node)
{
	DefaultVisitor::visitInitializer(node);
}
void PrettyPrintVisitor::visitInitializerClause(InitializerClauseAST* node)
{
	if (node->expression)
		visit(node->expression);
	else
	{
		*m_printer << '{';
		visitListWithSeparator(node->initializer_list,',');
		*m_printer << '}';
	}
}
void PrettyPrintVisitor::visitLabeledStatement(LabeledStatementAST* )
{
	Q_ASSERT(0); // Not implemented yet
}
void PrettyPrintVisitor::visitLinkageBody(LinkageBodyAST* )
{
	Q_ASSERT(0); // Not implemented yet
}
void PrettyPrintVisitor::visitLinkageSpecification(LinkageSpecificationAST* )
{
	Q_ASSERT(0); // Not implemented yet
}
void PrettyPrintVisitor::visitMemInitializer(MemInitializerAST* node)
{
	visit(node->initializer_id);
	*m_printer << '(';
	visit(node->expression);
	*m_printer << ')';
}
void PrettyPrintVisitor::visitName(NameAST* node)
{
	visitListWithSeparator(node->qualified_names,Token_scope,0,Token_scope);
	visit(node->unqualified_name);
}
void PrettyPrintVisitor::visitNamespace(NamespaceAST* node)
{
	*m_printer << Token_namespace;
	writeToken(node,node->namespace_name);
	visit(node->linkage_body);
}
void PrettyPrintVisitor::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
	*m_printer << Token_namespace;
	writeToken(node,node->namespace_name);
	*m_printer << '=';
	visit(node->alias_name);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitNewDeclarator(NewDeclaratorAST* node)
{
	visit(node->ptr_op);
	visit(node->sub_declarator);
	
	const ListNode<ExpressionAST*>* expression = node->expressions ? 
												 node->expressions->toFront() : 0;
	while (expression)
	{
		*m_printer << '[';
		visit(expression->element);
		*m_printer << ']';
		if (expression->hasNext())
			expression = expression->next;
		else
			break;
	}
}
void PrettyPrintVisitor::visitNewExpression(NewExpressionAST* node)
{
	if (node->scope_token)
		*m_printer << Token_scope;
	if (node->new_token)
		*m_printer << Token_new;

	if (node->expression)
	{
		*m_printer << '(';
		visit(node->expression);
		*m_printer << ')';
	}
	
	if (node->type_id)
	{
		*m_printer << '(';
		visit(node->type_id);
		*m_printer << ')';
	}
	else
		visit(node->new_type_id);

	if (node->new_initializer)
	{
		*m_printer << '(';
		visit(node->new_initializer);
		*m_printer << ')';
	}
}
void PrettyPrintVisitor::visitNewInitializer(NewInitializerAST* node)
{
	visit(node->expression);
}
void PrettyPrintVisitor::visitOperator(OperatorAST* node)
{
	writeToken(node,node->open);
	if (node->open != node->op)
		writeToken(node,node->op);
	writeToken(node,node->close);
}
void PrettyPrintVisitor::visitOperatorFunctionId(OperatorFunctionIdAST* node)
{
	*m_printer << Token_operator;
	visit(node->op);
	visit(node->type_specifier);
	visitNodes(this,node->ptr_ops);
}
void PrettyPrintVisitor::visitParameterDeclaration(ParameterDeclarationAST* node)
{
	DefaultVisitor::visitParameterDeclaration(node);
}
void PrettyPrintVisitor::visitPostfixExpression(PostfixExpressionAST* node)
{
	DefaultVisitor::visitPostfixExpression(node);
}
void PrettyPrintVisitor::visitPrimaryExpression(PrimaryExpressionAST* node)
{
	DefaultVisitor::visitPrimaryExpression(node);
	writeToken(node,node->token);
}
void PrettyPrintVisitor::visitPtrOperator(PtrOperatorAST* node)
{
	writeToken(node,node->op);
	writeTokenList(node,node->cv);
}
void PrettyPrintVisitor::visitPtrToMember(PtrToMemberAST* )
{
	Q_ASSERT(0); // Not implemented yet
}	
void PrettyPrintVisitor::visitJumpStatement(JumpStatementAST* node)
{
	writeToken(node,node->op);
	writeToken(node,node->identifier);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitReturnStatement(ReturnStatementAST* node)
{
	*m_printer << Token_return;
	visit(node->expression);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
	writeTokenList(node,node->storage_specifiers);
	writeTokenList(node,node->function_specifiers);
	visit(node->type_specifier);
	visitNodes(this,node->init_declarators);
	visit(node->win_decl_specifiers);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitSizeofExpression(SizeofExpressionAST* node)
{
	*m_printer << Token_sizeof;
	*m_printer << '(';
	if (node->type_id)
		visit(node->type_id);
	else
		visit(node->expression);
	*m_printer << ')';
}
void PrettyPrintVisitor::visitStringLiteral(StringLiteralAST* node)
{
	writeTokenList(node,node->literals);
}	
void PrettyPrintVisitor::visitSubscriptExpression(SubscriptExpressionAST* node)
{
	*m_printer << '[';
	visit(node->subscript);
	*m_printer << ']';
}
void PrettyPrintVisitor::visitSwitchStatement(SwitchStatementAST* node)
{
	*m_printer << Token_switch;
	*m_printer << '(';
	visit(node->condition);
	*m_printer << ')';
	visit(node->statement);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitTemplateArgument(TemplateArgumentAST* node)
{
	*m_printer << '<';
	visit(node->type_id);
	visit(node->expression);
	*m_printer << '>';
}
void PrettyPrintVisitor::visitTemplateDeclaration(TemplateDeclarationAST* node)
{
	writeToken(node,node->exported);
	*m_printer << Token_template;
	if (node->template_parameters)
	{
		*m_printer << '<';
		visitListWithSeparator(node->template_parameters,',');	
		*m_printer << '>';
	}
	visit(node->declaration);
}
void PrettyPrintVisitor::visitTemplateParameter(TemplateParameterAST* node)
{
	DefaultVisitor::visitTemplateParameter(node);	
}
void PrettyPrintVisitor::visitThrowExpression(ThrowExpressionAST* node)
{
	*m_printer << Token_throw;
	visit(node->expression);
}
void PrettyPrintVisitor::visitTryBlockStatement(TryBlockStatementAST* node)
{
	*m_printer << Token_try;
	visit(node->body);
	visitNodes(this,node->handlers);
}
void PrettyPrintVisitor::visitTypeId(TypeIdAST* node)
{
	DefaultVisitor::visitTypeId(node);
}
void PrettyPrintVisitor::visitTypeParameter(TypeParameterAST* node)
{
	int kind = m_tokenLookup->token(node,node->type)->kind;
	*m_printer << kind;

	if (kind == Token_class || kind == Token_typename)
	{
		visit(node->name);
		if (node->type_id)
		{
			*m_printer << '=';
			visit(node->type_id);
		}
	} 
	else if (kind == Token_template)
	{
		*m_printer << '<';
		visitListWithSeparator(node->template_parameters,',');
		*m_printer << '>';
		if (node->name)
		{
			visit(node->name);
			if (node->type_id)
			{
				*m_printer << '=';
				visit(node->type_id);
			}
		}
		if (node->template_name)
		{
			*m_printer << '=';
			visit(node->template_name);
		}
	}
}
void PrettyPrintVisitor::visitTypedef(TypedefAST* node)
{
	*m_printer << Token_typedef;
	visit(node->type_specifier);
	visitNodes(this,node->init_declarators);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitUsing(UsingAST* node)
{
	*m_printer << Token_using;
	writeToken(node,node->type_name);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitUsingDirective(UsingDirectiveAST* node)
{
	*m_printer << Token_namespace;
	visit(node->name);
	*m_printer << ';';
}
void PrettyPrintVisitor::visitWhileStatement(WhileStatementAST* node)
{
	*m_printer << Token_while;
	*m_printer << '(';
	visit(node->condition);
	*m_printer << ')';
	visit(node->statement);
}
void PrettyPrintVisitor::visitWinDeclSpec(WinDeclSpecAST* node)
{
	writeToken(node,node->specifier);
	writeToken(node,node->modifier);
}
template <class T>
void PrettyPrintVisitor::visitListWithSeparator(const ListNode<T>* nodes,int separatorToken,
												int startToken, int endToken)
{
	if (!nodes)
		return;

	if (startToken)
		*m_printer << startToken;

	const ListNode<T>* node = nodes->toFront();
	while (node != 0)
	{
		visit(node->element);
		if (node->hasNext())
		{
			*m_printer << separatorToken;
			node = node->next;
		}
		else
			break;
	}

	if (endToken)
		*m_printer << endToken;
}
void PrettyPrintVisitor::writeToken(AST* node, std::size_t index)
{
	const Token* token = m_tokenLookup->token(node,index);

	if (!token)
		return;

	switch (token->kind)
	{
	case Token_identifier:
	case Token_char_literal:
	case Token_string_literal:
	case Token_number_literal:
	case Token_shift:
		*m_printer << token->symbol();
		break;
	default:		
		*m_printer << token->kind;
	}
}
void PrettyPrintVisitor::writeTokenList(AST* node,const ListNode<std::size_t>* list) 
{
	if (list != 0)
		list = list->toFront();

	while (list != 0)
	{
		writeToken(node,list->element);
		if (list->hasNext())
			list = list->next;
		else
			break;
	}
}
