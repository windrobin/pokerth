/***************************************************************************
 *   Copyright (C) 2007 by Lothar May                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/* PokerTH network packet. */

#ifndef _NETPACKET_H_
#define _NETPACKET_H_

#include <playerdata.h>
#include <game_defs.h>
#include <gamedata.h>
#include <net/socket_helper.h>

#define NET_VERSION_MAJOR			1
#define NET_VERSION_MINOR			0

#define MIN_PACKET_SIZE				4
#define MAX_PACKET_SIZE				256
#define MAX_NAME_SIZE				64
#define MAX_PASSWORD_SIZE			64
#define MAX_CHAT_TEXT_SIZE			128
#define MAX_NUM_PLAYER_RESULTS		MAX_NUMBER_OF_PLAYERS


struct NetPacketHeader;

class NetPacketJoinGame;
class NetPacketJoinGameAck;
class NetPacketPlayerJoined;
class NetPacketPlayerLeft;
class NetPacketGameStart;
class NetPacketHandStart;
class NetPacketPlayersTurn;
class NetPacketPlayersAction;
class NetPacketPlayersActionDone;
class NetPacketPlayersActionRejected;
class NetPacketDealFlopCards;
class NetPacketDealTurnCard;
class NetPacketDealRiverCard;
class NetPacketEndOfHandShowCards;
class NetPacketEndOfHandHideCards;
class NetPacketSendChatText;
class NetPacketChatText;
class NetPacketError;

class NetPacket
{
public:
	static boost::shared_ptr<NetPacket> Create(char *data, unsigned &dataSize);

	NetPacket(u_int16_t type, u_int16_t initialLen);
	virtual ~NetPacket();

	virtual boost::shared_ptr<NetPacket> Clone() const = 0;

	const NetPacketHeader *GetRawData() const;
	NetPacketHeader *GetRawData();
	void SetRawData(const NetPacketHeader *p);

	u_int16_t GetType() const;
	u_int16_t GetLen() const;

	virtual const NetPacketJoinGame *ToNetPacketJoinGame() const;
	virtual const NetPacketJoinGameAck *ToNetPacketJoinGameAck() const;
	virtual const NetPacketPlayerJoined *ToNetPacketPlayerJoined() const;
	virtual const NetPacketPlayerLeft *ToNetPacketPlayerLeft() const;
	virtual const NetPacketGameStart *ToNetPacketGameStart() const;
	virtual const NetPacketHandStart *ToNetPacketHandStart() const;
	virtual const NetPacketPlayersTurn *ToNetPacketPlayersTurn() const;
	virtual const NetPacketPlayersAction *ToNetPacketPlayersAction() const;
	virtual const NetPacketPlayersActionDone *ToNetPacketPlayersActionDone() const;
	virtual const NetPacketPlayersActionRejected *ToNetPacketPlayersActionRejected() const;
	virtual const NetPacketDealFlopCards *ToNetPacketDealFlopCards() const;
	virtual const NetPacketDealTurnCard *ToNetPacketDealTurnCard() const;
	virtual const NetPacketDealRiverCard *ToNetPacketDealRiverCard() const;
	virtual const NetPacketEndOfHandShowCards *ToNetPacketEndOfHandShowCards() const;
	virtual const NetPacketEndOfHandHideCards *ToNetPacketEndOfHandHideCards() const;
	virtual const NetPacketSendChatText *ToNetPacketSendChatText() const;
	virtual const NetPacketChatText *ToNetPacketChatText() const;
	virtual const NetPacketError *ToNetPacketError() const;

protected:

	virtual void Check(const NetPacketHeader* data) const = 0;

	void Resize(u_int16_t newLen);

private:

	NetPacketHeader *m_data;
};

class NetPacketJoinGame : public NetPacket
{
public:
	struct Data
	{
		int versionMajor;
		int versionMinor;
		PlayerType ptype;
		std::string playerName;
		std::string password;
	};

	NetPacketJoinGame();
	virtual ~NetPacketJoinGame();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketJoinGame *ToNetPacketJoinGame() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketJoinGameAck : public NetPacket
{
public:
	struct Data
	{
		u_int32_t	sessionId;
		int16_t		yourPlayerNum;
		u_int16_t	yourPlayerUniqueId;
		GameData	gameData;
	};

	NetPacketJoinGameAck();
	virtual ~NetPacketJoinGameAck();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketJoinGameAck *ToNetPacketJoinGameAck() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketPlayerJoined : public NetPacket
{
public:
	struct Data
	{
		u_int16_t	playerId;
		u_int16_t	playerNumber;
		PlayerType ptype;
		std::string playerName;
	};

	NetPacketPlayerJoined();
	virtual ~NetPacketPlayerJoined();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketPlayerJoined *ToNetPacketPlayerJoined() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketPlayerLeft : public NetPacket
{
public:
	struct Data
	{
		u_int16_t	playerId;
	};

	NetPacketPlayerLeft();
	virtual ~NetPacketPlayerLeft();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketPlayerLeft *ToNetPacketPlayerLeft() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketGameStart : public NetPacket
{
public:
	struct Data
	{
		StartData	startData;
		u_int16_t	reserved;
	};

	NetPacketGameStart();
	virtual ~NetPacketGameStart();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketGameStart *ToNetPacketGameStart() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketHandStart : public NetPacket
{
public:
	struct Data
	{
		u_int16_t	yourCards[2];
	};

	NetPacketHandStart();
	virtual ~NetPacketHandStart();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketHandStart *ToNetPacketHandStart() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketPlayersTurn : public NetPacket
{
public:
	struct Data
	{
		GameState	gameState;
		u_int16_t	playerId;
	};

	NetPacketPlayersTurn();
	virtual ~NetPacketPlayersTurn();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketPlayersTurn *ToNetPacketPlayersTurn() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketPlayersAction : public NetPacket
{
public:
	struct Data
	{
		GameState		gameState;
		PlayerAction	playerAction;
		u_int32_t		playerBet;
	};

	NetPacketPlayersAction();
	virtual ~NetPacketPlayersAction();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketPlayersAction *ToNetPacketPlayersAction() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketPlayersActionDone : public NetPacket
{
public:
	struct Data
	{
		GameState		gameState;
		u_int16_t		playerId;
		PlayerAction	playerAction;
		u_int32_t		totalPlayerBet;
		u_int32_t		playerMoney;
		u_int32_t		potSize;
		u_int32_t		curHandBets;
	};

	NetPacketPlayersActionDone();
	virtual ~NetPacketPlayersActionDone();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketPlayersActionDone *ToNetPacketPlayersActionDone() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketPlayersActionRejected : public NetPacket
{
public:

	struct Data
	{
		GameState		gameState;
		PlayerAction	playerAction;
		u_int32_t		playerBet;
		int				rejectionReason;
	};

	NetPacketPlayersActionRejected();
	virtual ~NetPacketPlayersActionRejected();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketPlayersActionRejected *ToNetPacketPlayersActionRejected() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketDealFlopCards : public NetPacket
{
public:
	struct Data
	{
		u_int16_t	flopCards[3];
	};

	NetPacketDealFlopCards();
	virtual ~NetPacketDealFlopCards();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketDealFlopCards *ToNetPacketDealFlopCards() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketDealTurnCard : public NetPacket
{
public:
	struct Data
	{
		u_int16_t	turnCard;
	};

	NetPacketDealTurnCard();
	virtual ~NetPacketDealTurnCard();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketDealTurnCard *ToNetPacketDealTurnCard() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketDealRiverCard : public NetPacket
{
public:
	struct Data
	{
		u_int16_t	riverCard;
	};

	NetPacketDealRiverCard();
	virtual ~NetPacketDealRiverCard();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketDealRiverCard *ToNetPacketDealRiverCard() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketEndOfHandShowCards : public NetPacket
{
public:
	struct PlayerResult
	{
		u_int16_t		playerId;
		u_int16_t		cards[2];
		u_int16_t		bestHandPos[5];
		u_int32_t		valueOfCards;
		u_int32_t		moneyWon;
		u_int32_t		playerMoney;
	};

	typedef std::list<PlayerResult> PlayerResultList;

	struct Data
	{
		PlayerResultList playerResults;
	};

	NetPacketEndOfHandShowCards();
	virtual ~NetPacketEndOfHandShowCards();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketEndOfHandShowCards *ToNetPacketEndOfHandShowCards() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketEndOfHandHideCards : public NetPacket
{
public:
	struct Data
	{
		u_int16_t		playerId;
		u_int32_t		moneyWon;
		u_int32_t		playerMoney;
	};

	NetPacketEndOfHandHideCards();
	virtual ~NetPacketEndOfHandHideCards();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketEndOfHandHideCards *ToNetPacketEndOfHandHideCards() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketSendChatText : public NetPacket
{
public:
	struct Data
	{
		std::string		text;
	};

	NetPacketSendChatText();
	virtual ~NetPacketSendChatText();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketSendChatText *ToNetPacketSendChatText() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketChatText : public NetPacket
{
public:
	struct Data
	{
		u_int16_t		playerId;
		std::string		text;
	};

	NetPacketChatText();
	virtual ~NetPacketChatText();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketChatText *ToNetPacketChatText() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

class NetPacketError : public NetPacket
{
public:
	struct Data
	{
		int		errorCode;
	};

	NetPacketError();
	virtual ~NetPacketError();

	virtual boost::shared_ptr<NetPacket> Clone() const;

	void SetData(const Data &inData);
	void GetData(Data &outData) const;

	virtual const NetPacketError *ToNetPacketError() const;

protected:

	virtual void Check(const NetPacketHeader* data) const;
};

#endif

