
#include <QtCore/QAbstractItemModel>
#include <QtXml/QDomNode>

class DomModel : public QAbstractItemModel
{
Q_OBJECT

public:
	DomModel(QObject* parent = 0);
	virtual ~DomModel();

	void setDomNode(const QDomNode& node);
	QDomNode domNode() const;

	virtual int columnCount(const QModelIndex& parent) const;
	virtual QVariant data(const QModelIndex& index,int role) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex&) const;
	virtual int rowCount(const QModelIndex& parent) const;

private:
	class DomItem
	{
	public:
		DomItem(const QDomNode& node, DomItem* parent);
		~DomItem();

		QDomNode node;
		DomItem* parent;
		DomItem** children;
		
		int childCount() const
		{
			return node.childNodes().count();
		}
	};

	DomItem* domItem(const QModelIndex& index) const;
	QDomNode node(const QModelIndex& index) const;

	DomItem* m_rootItem;
};


