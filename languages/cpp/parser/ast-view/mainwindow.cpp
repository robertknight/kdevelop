
// Own
#include "mainwindow.h"

// Qt
#include <QBuffer>
#include <QDockWidget>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <QtDebug>
#include <QPushButton>

// KDE
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>

// KDevelop
#include "ast.h"
#include "control.h"
#include "parser.h"
#include "parsesession.h"
#include "prettyprintvisitor.h"
#include "xmlwritervisitor.h"

#include "transform/astmerger.h"
#include "transform/astmanipulator.h"

// Local
#include "dommodel.h"

using namespace ASTView;
using namespace KDevelop;

MainWindow::MainWindow()
	: m_lastParseSession(0)
{
	QSplitter* centralWidget = new QSplitter;
	setCentralWidget(centralWidget);
	setWindowTitle("AST Viewer");

	QWidget* sourceWidget = createSourceView();
	QWidget* astViews = createASTViews();
	createProblemView();
	
	centralWidget->addWidget(sourceWidget);
	centralWidget->addWidget(astViews);
}
void MainWindow::createProblemView()
{
	QDockWidget* problemDock = new QDockWidget("Problems",this);
	m_parseProblemView = new QListWidget;
	problemDock->setWidget(m_parseProblemView);
	addDockWidget(Qt::BottomDockWidgetArea,problemDock);
}
QWidget* MainWindow::createSourceView()
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

	return m_sourceView;
}
QWidget* MainWindow::createASTViews()
{
	QSplitter* astSplitter = new QSplitter(Qt::Vertical);
	
	QWidget* originalAst= new QWidget;
	m_astModel = new DomModel(this);
	QWidget* astLabel = new QLabel("Original AST",originalAst);
	m_astView = new QTreeView(originalAst);
	m_astView->setModel(m_astModel);
	
	QVBoxLayout* originalAstLayout = new QVBoxLayout(originalAst);
	originalAstLayout->addWidget(astLabel);
	originalAstLayout->addWidget(m_astView);

	QWidget* transformedAst = new QWidget;
	m_transformedAstModel = new DomModel(this);
	QWidget* transformLabel = new QLabel("Transformed AST",transformedAst);
	m_transformedAstView = new QTreeView(transformedAst);
	m_transformedAstView->setModel(m_transformedAstModel);
	QLabel* transformEditLabel = new QLabel("Transform:");
	m_transformEdit = new QLineEdit();
	QPushButton* transformApply = new QPushButton("Apply");
	QVBoxLayout* transformedAstLayout = new QVBoxLayout(transformedAst);
	transformedAstLayout->addWidget(transformLabel);
	transformedAstLayout->addWidget(m_transformedAstView);
	QHBoxLayout* transformEditLayout = new QHBoxLayout();
	transformedAstLayout->addLayout(transformEditLayout);
	transformEditLayout->addWidget(transformEditLabel);
	transformEditLayout->addWidget(m_transformEdit);
	transformEditLayout->addWidget(transformApply);

	astSplitter->addWidget(originalAst);
	astSplitter->addWidget(transformedAst);

	return astSplitter;
}
void MainWindow::applyTransform()
{
}
#if 0
void MainWindow::applyTransform()
{
	TransformScriptParser transformParser;
	XmlWriterVisitor writer;

	AST* rootAst = 0;

	ASTManipulator* manipulator = transformParser.createManipulator(m_transformEdit->text());
	if (manipulator)
	{
		ASTChanges changes = manipulator->createChanges(node);
		NodeLookup nodeLookup(rootAst);
		nodeLookup.addChanges(changes);

		TransformedSourcePrinter printer;
		printer.setNodeLookup(&nodeLookup);
		printer.
		

		// dump transformed AST as XML
		QBuffer buffer;
		buffer.open(QIODevice::ReadWrite);
		writer.write(&buffer,ast,tokenStream);
	}
}
#endif
void MainWindow::recreateAST()
{
	m_control = Control();
	
    XmlWriterVisitor writer;
	TranslationUnitAST* ast;
	TokenStream* tokenStream;

	if (parse(m_sourceDocument->text().toLocal8Bit(),ast,tokenStream))
	{
		// dump AST as XML to output
        TokenStreamTokenLookup plainTokenLookup(tokenStream);
		NodeLookup nodeLookup;
        writer.setTokenLookup(&plainTokenLookup);
		writer.setNodeLookup(&nodeLookup);
        QBuffer buffer;
		buffer.open(QIODevice::ReadWrite);
		writer.write(&buffer,ast);
		QDomDocument xmlDoc;
		xmlDoc.setContent(buffer.data());
		
		m_astModel->setDomNode(xmlDoc);

		updateProblemList();

		// recreate AST with pretty printer
		MergedTokenLookup tokenLookup(tokenStream);
		SimplePrinter printer;
        
        RenameManipulator manipulator;
        manipulator.setTokenLookup(&tokenLookup);
        manipulator.setName("foo");
        manipulator.setNewName("bar");
        ASTChanges changes = manipulator.createChanges(ast);
        changes.addToLookup(nodeLookup);

        XmlWriterVisitor transformWriter;
        transformWriter.setTokenLookup(&tokenLookup);
        transformWriter.setNodeLookup(&nodeLookup);
        QBuffer trBuffer;
        trBuffer.open(QIODevice::ReadWrite);
        transformWriter.write(&trBuffer,ast);
        
        QDomDocument trXmlDoc;
        trXmlDoc.setContent(trBuffer.data());
        m_transformedAstModel->setDomNode(trXmlDoc);

        TransformedSourcePrinter visitor;
		visitor.setTokenLookup(&tokenLookup);
        visitor.setNodeLookup(&nodeLookup);
		visitor.setPrinter(&printer);
		QFile outFile;
		outFile.open(stdout,QIODevice::WriteOnly);
		char newLine = '\n';
		outFile.write(&newLine,1);
        visitor.setDevice(&outFile);
		visitor.visit(ast);
	}
	m_astView->expandAll();
    m_transformedAstView->expandAll();
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


