
// Own
#include "mainwindow.h"

// Qt
#include <QBuffer>
#include <QDockWidget>
#include <QListWidget>
#include <QTreeView>
#include <QHBoxLayout>
#include <QtDebug>
#include <QFile>
#include <QTimer>

// KDE
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>

// KDevelop
#include "ast.h"
#include "parser.h"
#include "parsesession.h"
#include "control.h"
#include "xmlwritervisitor.h"
#include "prettyprintvisitor.h"

// Local
#include "dommodel.h"

using namespace ASTView;
using namespace KDevelop;

MainWindow::MainWindow()
	: m_lastParseSession(0)
{
	QWidget* centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	setWindowTitle("AST Viewer");

	QHBoxLayout* layout = new QHBoxLayout(centralWidget);

	createSourceView();
	createASTView();
	createProblemView();
	
	layout->addWidget(m_sourceView);
	layout->addWidget(m_astView);
}
void MainWindow::createProblemView()
{
	QDockWidget* problemDock = new QDockWidget("Problems",this);
	m_parseProblemView = new QListWidget;
	problemDock->setWidget(m_parseProblemView);
	addDockWidget(Qt::BottomDockWidgetArea,problemDock);
}
void MainWindow::createSourceView()
{
	// timer to trigger AST view updates when source changes
	m_parseTimer = new QTimer();
	m_parseTimer->setInterval(500);
	m_parseTimer->setSingleShot(true);
	connect(m_parseTimer,SIGNAL(timeout()),this,SLOT(recreateAST()));

	// syntax-highlighted source editor
	KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
	Q_ASSERT(editor);
	m_sourceDocument = editor->createDocument(this);
	m_sourceDocument->setHighlightingMode("C++");
	connect(m_sourceDocument,SIGNAL(textChanged(KTextEditor::Document*)),m_parseTimer,
		SLOT(start()));
	m_sourceView = m_sourceDocument->createView(this);
	m_sourceView->setMinimumWidth(400);
}
void MainWindow::createASTView()
{
	m_astModel = new DomModel(this);
	m_astView = new QTreeView(this);
	m_astView->setModel(m_astModel);
}
void MainWindow::recreateAST()
{
	m_control = Control();
	XmlWriterVisitor writer;
	TranslationUnitAST* ast;
	TokenStream* tokenStream;

	if (parse(m_sourceDocument->text().toLocal8Bit(),ast,tokenStream))
	{
		// dump AST as XML to output
		QBuffer buffer;
		buffer.open(QIODevice::ReadWrite);
		writer.write(&buffer,ast,tokenStream);
		QDomDocument xmlDoc;
		xmlDoc.setContent(buffer.data());
		
		m_astModel->setDomNode(xmlDoc);

		updateProblemList();

		// recreate AST with pretty printer
		TokenStreamTokenLookup tokenLookup(tokenStream);
		SimplePrinter printer;
		PrettyPrintVisitor visitor;
		visitor.setTokenLookup(&tokenLookup);
		visitor.setPrinter(&printer);

		QFile outFile;
		outFile.open(stdout,QIODevice::WriteOnly);
		char newLine = '\n';
		outFile.write(&newLine,1);
		visitor.write(&outFile,ast);
	}
	m_astView->expandAll();
}
void MainWindow::updateProblemList()
{
	m_parseProblemView->clear();
	foreach(KDevelop::ProblemPointer problem,m_control.problems())
	{
		QString description;
		
		switch (problem->source())
		{
			case Problem::Lexer:
				description += "Lexer";
				break;
			case Problem::Parser:
				description += "Parser";
				break;
			case Problem::Preprocessor:
				description += "Pre-processor";
				break;
			default:
				description += "Unknown";
		}

		description += ": ";
		description += problem->description();

		if (!problem->explanation().isEmpty())
			description += '(' + problem->explanation() + ')';
		
		m_parseProblemView->addItem(description);
	}
}
bool MainWindow::parse(const QByteArray& source, TranslationUnitAST*& ast,
					   TokenStream*& tokenStream)
{
	Parser parser(&m_control);
	delete m_lastParseSession;
	m_lastParseSession = new ParseSession();
	m_lastParseSession->setContentsAndGenerateLocationTable(source);

	ast = parser.parse(m_lastParseSession);
	tokenStream = m_lastParseSession->token_stream;

	return ast != 0;
}


