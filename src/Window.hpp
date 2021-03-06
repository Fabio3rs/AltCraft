#pragma once

#include <queue>

#include "Packet.hpp"

struct Window {
    unsigned char WindowId = 0;
    std::string type;
    
    SlotDataType handSlot;
    const short HandSlotId = -1;
    std::vector<SlotDataType> slots;

    short actions = 1;

    void MakeClick(short ClickedSlot, bool Lmb, bool dropMode = false);

    std::queue<PacketClickWindow> pendingTransactions;
    std::vector<std::pair<short,std::pair<short,short>>> transactions;

    void ConfirmTransaction(PacketConfirmTransactionCB packet);
};