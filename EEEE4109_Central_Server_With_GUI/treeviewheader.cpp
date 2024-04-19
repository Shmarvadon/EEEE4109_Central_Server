#include "polelist.h"

TreeViewHeader::TreeViewHeader(PoleDataModel* model, QVariantList data, Pole* parent) : _itemData(std::move(data)), _parentItem(parent), _Model(model) {}



void TreeViewHeader::appendChild(Pole* child) {
	_childItems.push_back(child);
}

Pole* TreeViewHeader::child(int row) {
	return row >= 0 && row < childCount() ? _childItems.at(row) : nullptr;
}

int TreeViewHeader::childCount() const {
	return int(_childItems.size());
}

int TreeViewHeader::row() const {
	return 0;
}

int TreeViewHeader::columnCount() const {
	return int(_itemData.count());
}

QVariant TreeViewHeader::data(int column) const {
	return _itemData.value(column);
}

TreeViewHeader* TreeViewHeader::parentItem() {
	return nullptr;
}