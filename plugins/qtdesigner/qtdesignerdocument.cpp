/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
/****************************************************************************
**
** Copyright 1992-2006 Trolltech AS. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtdesignerdocument.h"

#include "qtdesignerplugin.h"
#include <icore.h>
#include <iuicontroller.h>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtCore/QFile>
#include <sublime/view.h>

QtDesignerDocument::QtDesignerDocument( const KUrl& url , KDevelop::ICore* core )
    : Sublime::UrlDocument(core->uiController()->controller(), url),
      KDevelop::IDocument(core), m_url(url), m_state(KDevelop::IDocument::Clean)
{

}

KUrl QtDesignerDocument::url() const
{
    return m_url;
}

KSharedPtr<KMimeType> QtDesignerDocument::mimeType() const
{
    return KMimeType::mimeType("application/x-designer");
}

KParts::Part* QtDesignerDocument::partForView(QWidget*) const
{
    return 0;
}

KTextEditor::Document* QtDesignerDocument::textDocument() const
{
    return 0;
}

bool QtDesignerDocument::save(KDevelop::IDocument::DocumentSaveMode mode)
{
    Q_UNUSED(mode);
    kDebug(9038) << "Going to Save";
    if( m_state == KDevelop::IDocument::Clean )
        return false;
    if( m_forms.isEmpty() )
        return false;
    QFile f(m_url.toLocalFile());
    if( !f.open( QIODevice::WriteOnly ) )
    {
        kDebug(9038) << "Couldn't open file:" << f.error();
        return false;
    }
    QTextStream s(&f);
    s << m_forms.first()->contents();
    s.flush();
    f.close();
    m_state = KDevelop::IDocument::Clean;
    notifySaved();
    return true;
}

void QtDesignerDocument::reload()
{
    QFile uiFile(m_url.toLocalFile());
    foreach(QDesignerFormWindowInterface* form, m_forms)
    {
        form->setContents(&uiFile);
    }
    m_state = KDevelop::IDocument::Clean;
    notifyStateChanged();
}

void QtDesignerDocument::close()
{
    foreach(QDesignerFormWindowInterface* form, m_forms)
    {
        m_designerPlugin->designer()->formWindowManager()->removeFormWindow(form);
        QMdiArea* area = m_areas.at(m_forms.indexOf(form));
        m_areas.removeAll(area);
        m_forms.removeAll(form);
        delete area;
    }
}

bool QtDesignerDocument::isActive() const
{
    QDesignerFormWindowInterface* activeWin =
            m_designerPlugin->designer()->formWindowManager()->activeFormWindow();
    foreach( QDesignerFormWindowInterface* form, m_forms )
    {
        if( activeWin == form )
            return true;
    }
    return false;
}

KDevelop::IDocument::DocumentState QtDesignerDocument::state() const
{
    return m_state;
}

void QtDesignerDocument::setCursorPosition(const KTextEditor::Cursor&)
{
    return;
}

void QtDesignerDocument::activate(Sublime::View* view)
{
    QMdiArea* a = dynamic_cast<QMdiArea*>(view->widget());
    if(a)
    {
        int num = m_areas.indexOf(a);
        kDebug(9038) << "Area found at" << num;
        if( num >= 0 )
        {
            m_designerPlugin->designer()->formWindowManager()->setActiveFormWindow( m_forms.at(num) );
        }
    }
    notifyActivated();
}

void QtDesignerDocument::setDesignerPlugin(QtDesignerPlugin* plugin)
{
    m_designerPlugin = plugin;
}

QWidget *QtDesignerDocument::createViewWidget(QWidget *parent)
{
    kDebug(9038) << "Creating new area for form:" << m_url;
    QMdiArea* area = new QMdiArea(parent);
//     area->setScrollBarsEnabled( true ); //FIXME commented just to make it compile with the new qt-copy
//     area->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
//     area->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    QFile uiFile(m_url.toLocalFile());
    QDesignerFormWindowManagerInterface* manager = m_designerPlugin->designer()->formWindowManager();

    QDesignerFormWindowInterface* form = manager->createFormWindow();
    kDebug(9038) << "now we have" << manager->formWindowCount() << "formwindows";
    form->setFileName(m_url.toLocalFile());
    form->setContents(&uiFile);
    manager->setActiveFormWindow(form);
    QMdiSubWindow* window = area->addSubWindow(form, Qt::Window | Qt::WindowShadeButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    const QSize containerSize = form->mainContainer()->size();
    const QSize containerMinimumSize = form->mainContainer()->minimumSize();
    const QSize containerMaximumSize = form->mainContainer()->maximumSize();
    const QSize decorationSize = window->geometry().size() - window->contentsRect().size();
    window->resize(containerSize+decorationSize);
    window->setMinimumSize(containerMinimumSize+decorationSize);
    if( containerMaximumSize == QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX) )
        window->setMaximumSize(containerMaximumSize);
    else
        window->setMaximumSize(containerMaximumSize+decorationSize);
    window->setWindowTitle( form->mainContainer()->windowTitle() );
    m_areas << area;
    connect( form, SIGNAL(changed()), this, SLOT(formChanged()));
    m_forms << form;
    return area;
}

void QtDesignerDocument::formChanged()
{
    kDebug(9038) << "Form changed";
    QDesignerFormWindowInterface* activeForm = m_designerPlugin->designer()->formWindowManager()->activeFormWindow();
    foreach(QDesignerFormWindowInterface* form, m_forms)
    {
        if( form != activeForm )
        {
            form->disconnect( this );
            form->setContents(activeForm->contents());
            connect( form, SIGNAL(changed()), this, SLOT(formChanged()));
        }
    }
    m_state = KDevelop::IDocument::Modified;
    notifyStateChanged();
}

KTextEditor::Cursor QtDesignerDocument::cursorPosition( ) const
{
    return KTextEditor::Cursor();
}

#include "qtdesignerdocument.moc"

