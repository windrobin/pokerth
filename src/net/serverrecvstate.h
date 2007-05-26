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
/* State of network server. */

#ifndef _SERVERRECVSTATE_H_
#define _SERVERRECVSTATE_H_

#include <net/serverrecvthread.h>
#include <playerdata.h>

#define SERVER_INITIAL_STATE	ServerRecvStateInit
#define SERVER_START_GAME_STATE	ServerRecvStateStartGame

class Game;
class PlayerInterface;
class ServerCallback;

class ServerRecvState
{
public:
	virtual ~ServerRecvState();

	// Handling of a new TCP connection.
	virtual void HandleNewConnection(ServerRecvThread &server, boost::shared_ptr<ConnectData> connData) = 0;

	// Main processing function of the current state.
	virtual int Process(ServerRecvThread &server) = 0;
};

// Abstract State: Receiving.
class ServerRecvStateReceiving : public ServerRecvState
{
public:
	virtual ~ServerRecvStateReceiving();

	// Globally handle packets which are allowed in all running states.
	// Calls InternalProcess if packet has not been processed.
	virtual int Process(ServerRecvThread &server);

protected:

	ServerRecvStateReceiving();

	virtual int InternalProcess(ServerRecvThread &server, SessionWrapper session, boost::shared_ptr<NetPacket> packet) = 0;
};

// State: Initialization.
class ServerRecvStateInit : public ServerRecvStateReceiving
{
public:
	// Access the state singleton.
	static ServerRecvStateInit &Instance();

	virtual ~ServerRecvStateInit();

	// 
	virtual void HandleNewConnection(ServerRecvThread &server, boost::shared_ptr<ConnectData> data);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateInit();

	virtual int InternalProcess(ServerRecvThread &server, SessionWrapper session, boost::shared_ptr<NetPacket> packet);

private:

	u_int16_t		m_curUniquePlayerId;
};

// Abstract State: Game is running.
class ServerRecvStateRunning : public ServerRecvState
{
public:
	virtual ~ServerRecvStateRunning();

	// Reject new connections.
	virtual void HandleNewConnection(ServerRecvThread &server, boost::shared_ptr<ConnectData> data);

protected:

	ServerRecvStateRunning();
};

// State: Start server game.
class ServerRecvStateStartGame : public ServerRecvStateRunning
{
public:
	// Access the state singleton.
	static ServerRecvStateStartGame &Instance();

	virtual ~ServerRecvStateStartGame();

	// 
	virtual int Process(ServerRecvThread &server);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateStartGame();
};

// State: Start new hand.
class ServerRecvStateStartHand : public ServerRecvStateRunning
{
public:
	// Access the state singleton.
	static ServerRecvStateStartHand &Instance();

	virtual ~ServerRecvStateStartHand();

	// 
	virtual int Process(ServerRecvThread &server);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateStartHand();
};

// State: Start new round.
class ServerRecvStateStartRound : public ServerRecvStateRunning
{
public:
	// Access the state singleton.
	static ServerRecvStateStartRound &Instance();

	virtual ~ServerRecvStateStartRound();

	// 
	virtual int Process(ServerRecvThread &server);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateStartRound();

	static void GameRun(Game &curGame);
	static PlayerInterface *GetCurrentPlayer(Game &curGame);
	static void SendNewRoundCards(ServerRecvThread &server, Game &curGame, int state);
};

// State: Wait for a player action.
class ServerRecvStateWaitPlayerAction : public ServerRecvStateReceiving
{
public:
	// Access the state singleton.
	static ServerRecvStateWaitPlayerAction &Instance();

	virtual ~ServerRecvStateWaitPlayerAction();

	virtual void HandleNewConnection(ServerRecvThread &server, boost::shared_ptr<ConnectData> data);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateWaitPlayerAction();

	virtual int InternalProcess(ServerRecvThread &server, SessionWrapper session, boost::shared_ptr<NetPacket> packet);

	static int GetHighestSet(Game &curGame);
	static void SetHighestSet(Game &curGame, int highestSet);
};

// State: Final.
class ServerRecvStateNextHand : public ServerRecvStateRunning
{
public:
	// Access the state singleton.
	static ServerRecvStateNextHand &Instance();

	virtual ~ServerRecvStateNextHand();

	void SetTimer(const boost::microsec_timer &timer);

	// 
	virtual int Process(ServerRecvThread &server);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateNextHand();

private:

	boost::microsec_timer m_delayTimer;
};


// State: Final.
class ServerRecvStateFinal : public ServerRecvStateRunning
{
public:
	// Access the state singleton.
	static ServerRecvStateFinal &Instance();

	virtual ~ServerRecvStateFinal();

	// 
	virtual int Process(ServerRecvThread &server);

protected:

	// Protected constructor - this is a singleton.
	ServerRecvStateFinal();
};

#endif
