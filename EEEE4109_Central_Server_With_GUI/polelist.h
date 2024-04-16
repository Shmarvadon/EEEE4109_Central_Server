#pragma once

#include "pole.h"

#define UDP_BUFF_SIZE 512

class PoleDataModel;

class udplistnerthread : public QThread {
    Q_OBJECT
public:

        explicit udplistnerthread(PoleDataModel* model, QObject* parent, Pole* rootItem, int UDPListnerPort, std::pair<uint32_t, uint32_t> TCPPortRange) : QThread(parent), _RootItem(rootItem), _UDPListnerPort(UDPListnerPort), _TCPPortsRange(TCPPortRange), _NumberOfPolesConncted(0), _Model(model) {};
        void run() override;

signals:
    void appendNewPole(int port, int sessionId, uint64_t HWID, uint8_t type);

private:
    std::pair<uint32_t, uint32_t> _TCPPortsRange;
    SOCKET UDPListener;
    SOCKET UDPSender;
    int _UDPListnerPort;
    int _NumberOfPolesConncted;
    Pole* _RootItem;
    PoleDataModel* _Model;
};



class PoleDataModel : public QAbstractItemModel {

    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(PoleDataModel)

    /*			Constructors			*/
    PoleDataModel(int UDPListnerPort, std::pair<uint32_t, uint32_t> TCPPortRange);
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

    Pole* getRootItem();

    /*			My Methods :)			*/
public slots:

    // From UDP listner thread.
    void appendNewPole(int port, int sessionId, uint64_t HWID, uint8_t type);

    // From Pole class.
    void updateVisual(){ dataChanged(index(0, 0), index(rowCount(), columnCount()), { Qt::DecorationRole }); }

signals:

private:


    /*			Stuff for QT UI			*/
    Pole* _rootItem = nullptr;

    /*			My Stuff :)			*/

    std::vector<Pole*> _poles;
    udplistnerthread* _UDPListnerThread;
};