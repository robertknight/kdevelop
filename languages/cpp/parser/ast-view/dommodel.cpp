
#include "dommodel.h"

DomModel::DomItem::DomItem(const QDomNode& node, DomItem* parent)
	: node(node) , parent(parent)
{
	children = (DomItem**)(new void*[childCount()]);
	memset(children,0,childCount()*sizeof(void*));
}

DomModel::DomItem::~DomItem()
{
	int count = childCount();
	for (int i=0;i<count;i++)
		delete children[i];
}
DomModel::DomModel(QObject* parent)
 : QAbstractItemModel(parent) , m_rootItem(new DomItem(QDomNode(),0))
{
}
void DomModel::setDomNode(const QDomNode& node)
{
	delete m_rootItem;
	m_rootItem = new DomItem(node,0);

	reset();
}
DomModel::~DomModel()
{
	delete m_rootItem;
}
QDomNode DomModel::domNode() const
{
	if (m_rootItem)
		return m_rootItem->node;
	else 
		return QDomNode();
}
DomModel::DomItem* DomModel::domItem(const QModelIndex& index) const
{
	if (index.isValid())
		return (DomItem*)(index.internalPointer());
	else
		return m_rootItem;
}
int DomModel::columnCount(const QModelIndex&) const
{
	return 1;	
}
QVariant DomModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return "Element";
	else
		return QAbstractItemModel::headerData(section,orientation,role);
}
QVariant DomModel::data(const QModelIndex& index, int role) const
{
	Q_ASSERT(index.isValid());

	if (role == Qt::DisplayRole)
	{
		QDomNode node = domItem(index)->node;
		QString name = node.nodeName();
		QString value = node.nodeValue();
	
		QString attributeString;

		if (node.hasAttributes())
		{
			QDomNamedNodeMap attributes = node.attributes();
			attributeString += " (";
			char* commaStr = 0;
			for (int i=0;i<attributes.count();i++)
			{
				QDomNode item = attributes.item(i);
				attributeString += commaStr + item.nodeName() + "=\"" + item.nodeValue() + '\"';
				commaStr = ", ";
			}
			attributeString += ") ";
		}

		switch (node.nodeType())
		{
		default:
			if (value.isEmpty())
				return name + attributeString;
			else
				return name + attributeString + " - \"" + value + '\"';
		}
	}
	else 
		return QVariant();
}
QModelIndex DomModel::index(int row, int column, const QModelIndex& parent) const
{
	DomItem* parentItem = domItem(parent);

	if (row >= parentItem->childCount())
		return QModelIndex();

	if (parentItem->children[row])
		return createIndex(row,column,parentItem->children[row]);
	else
	{
		DomItem* item = new DomItem(parentItem->node.childNodes().at(row),parentItem);
		parentItem->children[row] = item;

		return createIndex(row,column,item);
	}
}
QModelIndex DomModel::parent(const QModelIndex& index) const
{
	DomItem* parentItem = domItem(index)->parent;
	
	if (!parentItem)
		return QModelIndex();
	else
	{
		DomItem* parentParentItem = parentItem->parent;
		if (!parentParentItem)
			return QModelIndex();

		int parentRow = -1;
		for (int i=0;i<parentParentItem->childCount();i++)
			if (parentParentItem->children[++parentRow] == parentItem)
				break;

		return createIndex(parentRow,0,parentItem);
	}
}
int DomModel::rowCount(const QModelIndex& parent) const
{
	return domItem(parent)->childCount();
}





