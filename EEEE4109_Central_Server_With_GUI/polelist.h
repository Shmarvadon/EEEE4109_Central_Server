#pragma once

#include "pole.h"


class PoleDataModel;
class Pole;
class Gate;

class PoleDataModel : public QAbstractItemModel {

    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(PoleDataModel)

    /*			Constructors			*/
    PoleDataModel();
	~PoleDataModel();

    /*			QT methods			*/
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
        const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    TreeViewHeader* getRootItem();

    /*			My Methods :)			*/
public slots:

    // From UDP listner thread.
    void appendNewPole(sockaddr_in poleAddress, int port, uint64_t HWID, uint8_t type);

    // From UDP listner thread.
    Pole* getPoleByHWID(uint64_t HWID);
    

    // From Pole class.
    void updateVisual(){ dataChanged(index(0, 0), index(rowCount(), columnCount()), { Qt::DecorationRole }); }

    bool findPartnerToPole(Pole* pPole);

signals:

private:


    /*			Stuff for QT UI			*/
    TreeViewHeader* _rootItem = nullptr;

    /*			My Stuff :)			*/

    std::vector<Pole*> _poles;
    std::vector<Gate*> _Gates;
};

class TreeViewHeader : public QObject {
	Q_OBJECT

public:
	Q_DISABLE_COPY_MOVE(TreeViewHeader)

	/*			Constructors			*/
	explicit TreeViewHeader(PoleDataModel* model, QVariantList data, Pole* parentItem = nullptr);	// I do need this for the header.

    ~TreeViewHeader() {};

	/*			QT methods			*/
	void appendChild(Pole* child);

	Pole* child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	TreeViewHeader* parentItem();

    // My stuff.
    std::vector<Pole*>& getChildItems() { return _childItems; }

private:

	/*			Stuff for QT UI			*/
	std::vector<Pole*> _childItems;
	QVariantList _itemData;
	Pole* _parentItem;

    // My stuff.
    PoleDataModel* _Model;
};