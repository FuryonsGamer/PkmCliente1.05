/*
 * Copyright (c) 2010-2017 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <framework/core/application.h>

#include "protocolgame.h"
#include "game.h"
#include "player.h"
#include "item.h"
#include "localplayer.h"

void ProtocolGame::login(const std::string& accountName, const std::string& accountPassword, const std::string& host, uint16 port, const std::string& characterName, const std::string& authenticatorToken, const std::string& sessionKey, const std::string& worldName)
{
    m_accountName = accountName;
    m_accountPassword = accountPassword;
    m_authenticatorToken = authenticatorToken;
    m_sessionKey = sessionKey;
    m_characterName = characterName;
    m_worldName = worldName;

    connect(host, port);
}

void ProtocolGame::onConnect()
{
    m_firstRecv = true;
    Protocol::onConnect();

    m_localPlayer = g_game.getLocalPlayer();

    if (g_game.getFeature(Otc::GameSendWorldName))
        sendWorldName();

    if (g_game.getFeature(Otc::GamePacketSizeU32))
        enableBigPackets();

    if(g_game.getFeature(Otc::GameProtocolChecksum))
        enableChecksum();

    if(!g_game.getFeature(Otc::GameChallengeOnLogin))
        sendLoginPacket(0, 0);

    recv();
}

void ProtocolGame::onRecv(const InputMessagePtr& inputMessage)
{
    m_recivedPackeds += 1;
    m_recivedPackedsSize += inputMessage->getMessageSize();
    if(m_firstRecv) {
        m_firstRecv = false;

        if(g_game.getFeature(Otc::GameMessageSizeCheck)) {
            int size = g_game.getFeature(Otc::GamePacketSizeU32) ? inputMessage->getU32() : inputMessage->getU16();
            if(size != inputMessage->getUnreadSize()) {
                g_logger.traceError("invalid message size");
                return;
            }
        }
    }

    parseMessage(inputMessage);
    recv();
}

void ProtocolGame::onError(const boost::system::error_code& error)
{
    g_game.processConnectionError(error);
    disconnect();
}

void ProtocolGame::addItemInfo(const InputMessagePtr& msg, const ItemPtr& item)
{
    bool hasItem = msg->getU8();

    if (hasItem) {

        ItemInfo info;
        info.ItemID = msg->getU64();
        info.name = msg->getString();
        uint8 ball = msg->getU8();
        if (ball) {
            info.pokeballInfo = msg->getString();
        }

        info.desc = msg->getString();

        item->setItemInfo(info);
    }
}