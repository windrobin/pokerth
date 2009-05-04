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

#include <boost/asio.hpp>
#include <net/socket_helper.h>
#include <net/clientthread.h>
#include <net/clientstate.h>
#include <net/clientcontext.h>
#include <net/senderthread.h>
#include <net/receiverhelper.h>
#include <net/downloaderthread.h>
#include <net/clientexception.h>
#include <net/socket_msg.h>
#include <core/avatarmanager.h>
#include <core/loghelper.h>
#include <clientenginefactory.h>
#include <game.h>
#include <qttoolsinterface.h>

#include <boost/lambda/lambda.hpp>
#include <sstream>
#include <memory>
#include <cassert>

#define TEMP_AVATAR_FILENAME "avatar.tmp"

using namespace std;
using boost::asio::ip::tcp;


class ClientSenderCallback : public SenderCallback, public SessionDataCallback
{
public:
	ClientSenderCallback() {}
	virtual ~ClientSenderCallback() {}

	virtual void SignalNetError(SessionId /*session*/, int /*errorID*/, int /*osErrorID*/)
	{
	}

	virtual void SignalSessionTerminated(unsigned /*session*/)
	{
	}

private:
};

ClientThread::ClientThread(GuiInterface &gui, AvatarManager &avatarManager)
: m_curState(NULL), m_gui(gui), m_avatarManager(avatarManager), m_isServerSelected(false),
  m_curGameId(0), m_curGameNum(1), m_guiPlayerId(0), m_sessionEstablished(false)
{
	m_ioService.reset(new boost::asio::io_service());
	m_context.reset(new ClientContext);
	m_receiver.reset(new ReceiverHelper);
	myQtToolsInterface.reset(CreateQtToolsWrapper());
	m_senderCallback.reset(new ClientSenderCallback());
	m_senderThread.reset(new SenderThread(*m_senderCallback, m_ioService));
}

ClientThread::~ClientThread()
{
}

void
ClientThread::Init(
	const string &serverAddress, const string &serverListUrl,
	bool useServerList, unsigned serverPort, bool ipv6, bool sctp,
	const string &avatarServerAddress, const string &pwd,
	const string &playerName, const string &avatarFile,
	const string &cacheDir)
{
	if (IsRunning())
	{
		assert(false);
		return;
	}

	ClientContext &context = GetContext();

	context.SetProtocol(sctp ? SOCKET_IPPROTO_SCTP : 0);
	context.SetAddrFamily(ipv6 ? AF_INET6 : AF_INET);
	context.SetServerAddr(serverAddress);
	context.SetServerListUrl(serverListUrl);
	context.SetUseServerList(useServerList);
	context.SetServerPort(serverPort);
	context.SetAvatarServerAddr(avatarServerAddress);
	context.SetPassword(pwd);
	context.SetPlayerName(playerName);
	context.SetAvatarFile(avatarFile);
	context.SetCacheDir(cacheDir);
}

void
ClientThread::SendKickPlayer(unsigned playerId)
{
	boost::shared_ptr<NetPacket> request(new NetPacketKickPlayer);
	NetPacketKickPlayer::Data requestData;
	requestData.playerId = playerId;
	static_cast<NetPacketKickPlayer *>(request.get())->SetData(requestData);
	boost::mutex::scoped_lock lock(m_outPacketListMutex);
	m_outPacketList.push_back(request);
}

void
ClientThread::SendLeaveCurrentGame()
{
	boost::shared_ptr<NetPacket> request(new NetPacketLeaveCurrentGame);
	boost::mutex::scoped_lock lock(m_outPacketListMutex);
	m_outPacketList.push_back(request);
}

void
ClientThread::SendStartEvent(bool fillUpWithCpuPlayers)
{
	// Warning: This function is called in the context of the GUI thread.
	// Create a network packet for the server start event.
	boost::shared_ptr<NetPacket> start(new NetPacketStartEvent);
	NetPacketStartEvent::Data startData;
	startData.fillUpWithCpuPlayers = fillUpWithCpuPlayers;
	try
	{
		static_cast<NetPacketStartEvent *>(start.get())->SetData(startData);
		boost::mutex::scoped_lock lock(m_outPacketListMutex);
		m_outPacketList.push_back(start);
	} catch (const NetException &e)
	{
		LOG_ERROR("ClientThread::SendStartEvent: " << e.what());
	}
}

void
ClientThread::SendPlayerAction()
{
	// Warning: This function is called in the context of the GUI thread.
	// Create a network packet containing the current player action.
	boost::shared_ptr<NetPacket> action(new NetPacketPlayersAction);
	NetPacketPlayersAction::Data actionData;
	boost::shared_ptr<PlayerInterface> myPlayer = GetGame()->getSeatsList()->front();
	actionData.gameState = static_cast<GameState>(GetGame()->getCurrentHand()->getCurrentRound());
	actionData.playerAction = static_cast<PlayerAction>(myPlayer->getMyAction());
	// Only send last bet if not fold/checked.
	if (actionData.playerAction != PLAYER_ACTION_FOLD && actionData.playerAction != PLAYER_ACTION_CHECK)
		actionData.playerBet = myPlayer->getMyLastRelativeSet();
	else
		actionData.playerBet = 0;
	try
	{
		static_cast<NetPacketPlayersAction *>(action.get())->SetData(actionData);
		// Just dump the packet.
		boost::mutex::scoped_lock lock(m_outPacketListMutex);
		m_outPacketList.push_back(action);
	} catch (const NetException &e)
	{
		LOG_ERROR("ClientThread::SendPlayerAction: " << e.what());
	}
}

void
ClientThread::SendChatMessage(const std::string &msg)
{
	// Warning: This function is called in the context of the GUI thread.
	// Create a network packet containing the chat message.
	boost::shared_ptr<NetPacket> chat(new NetPacketSendChatText);
	NetPacketSendChatText::Data chatData;
	chatData.text = msg;
	try
	{
		static_cast<NetPacketSendChatText *>(chat.get())->SetData(chatData);
		// Just dump the packet.
		boost::mutex::scoped_lock lock(m_outPacketListMutex);
		m_outPacketList.push_back(chat);
	} catch (const NetException &e)
	{
		LOG_ERROR("ClientThread::SendChatMessage: " << e.what());
	}
}

void
ClientThread::SendJoinFirstGame(const std::string &password)
{
	// Warning: This function is called in the context of the GUI thread.
	// Create a network packet to request joining a game.
	boost::shared_ptr<NetPacket> join(new NetPacketJoinGame);
	NetPacketJoinGame::Data joinData;
	joinData.gameId = 1;
	joinData.password = password;
	try
	{
		static_cast<NetPacketJoinGame *>(join.get())->SetData(joinData);
		boost::mutex::scoped_lock lock(m_outPacketListMutex);
		m_outPacketList.push_back(join);
	} catch (const NetException &e)
	{
		LOG_ERROR("ClientThread::SendJoinFirstGame: " << e.what());
	}
}

void
ClientThread::SendJoinGame(unsigned gameId, const std::string &password)
{
	// Warning: This function is called in the context of the GUI thread.
	// Create a network packet to request joining a game.
	boost::shared_ptr<NetPacket> join(new NetPacketJoinGame);
	NetPacketJoinGame::Data joinData;
	joinData.password = password;
	joinData.gameId = gameId;
	try
	{
		static_cast<NetPacketJoinGame *>(join.get())->SetData(joinData);
		boost::mutex::scoped_lock lock(m_outPacketListMutex);
		m_outPacketList.push_back(join);
	} catch (const NetException &e)
	{
		LOG_ERROR("ClientThread::SendJoinGame: " << e.what());
	}
}

void
ClientThread::SendCreateGame(const GameData &gameData, const std::string &name, const std::string &password)
{
	// Warning: This function is called in the context of the GUI thread.
	// Create a network packet to request creating a new game.
	boost::shared_ptr<NetPacket> create(new NetPacketCreateGame);
	NetPacketCreateGame::Data createData;
	createData.gameData = gameData;
	createData.gameName = name;
	createData.password = password;
	try
	{
		static_cast<NetPacketCreateGame *>(create.get())->SetData(createData);
		boost::mutex::scoped_lock lock(m_outPacketListMutex);
		m_outPacketList.push_back(create);
	} catch (const NetException &e)
	{
		LOG_ERROR("ClientThread::SendCreateGame: " << e.what());
	}
}

void
ClientThread::SendResetTimeout()
{
	boost::shared_ptr<NetPacket> reset(new NetPacketResetTimeout);
	boost::mutex::scoped_lock lock(m_outPacketListMutex);
	m_outPacketList.push_back(reset);
}

void
ClientThread::SendAskKickPlayer(unsigned playerId)
{
	boost::shared_ptr<NetPacket> ask(new NetPacketAskKickPlayer);
	NetPacketAskKickPlayer::Data askData;
	askData.playerId = playerId;
	static_cast<NetPacketAskKickPlayer *>(ask.get())->SetData(askData);
	boost::mutex::scoped_lock lock(m_outPacketListMutex);
	m_outPacketList.push_back(ask);
}

void
ClientThread::SendVoteKick(bool doKick)
{
	boost::shared_ptr<NetPacket> vote(new NetPacketVoteKickPlayer);
	NetPacketVoteKickPlayer::Data voteData;
	{
		boost::mutex::scoped_lock lock(m_curPetitionIdMutex);
		voteData.petitionId = m_curPetitionId;
	}
	voteData.vote = doKick ? KICK_VOTE_IN_FAVOUR : KICK_VOTE_AGAINST;
	static_cast<NetPacketVoteKickPlayer *>(vote.get())->SetData(voteData);
	boost::mutex::scoped_lock lock(m_outPacketListMutex);
	m_outPacketList.push_back(vote);
}

void
ClientThread::SelectServer(unsigned serverId)
{
	boost::mutex::scoped_lock lock(m_selectServerMutex);
	m_isServerSelected = true;
	m_selectedServerId = serverId;
}

ServerInfo
ClientThread::GetServerInfo(unsigned serverId) const
{
	ServerInfo tmpInfo;
	boost::mutex::scoped_lock lock(m_serverInfoMapMutex);
	ServerInfoMap::const_iterator pos = m_serverInfoMap.find(serverId);
	if (pos != m_serverInfoMap.end())
	{
		tmpInfo = pos->second;
	}
	return tmpInfo;
}

GameInfo
ClientThread::GetGameInfo(unsigned gameId) const
{
	GameInfo tmpInfo;
	boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
	GameInfoMap::const_iterator pos = m_gameInfoMap.find(gameId);
	if (pos != m_gameInfoMap.end())
	{
		tmpInfo = pos->second;
	}
	return tmpInfo;
}

PlayerInfo
ClientThread::GetPlayerInfo(unsigned playerId) const
{
	PlayerInfo info;
	if (!GetCachedPlayerInfo(playerId, info))
	{
		ostringstream name;
		name << "#" << playerId;

		info.playerName = name.str();
	}
	return info;
}

bool
ClientThread::GetPlayerIdFromName(const string &playerName, unsigned &playerId) const
{
	bool retVal = false;

	boost::mutex::scoped_lock lock(m_playerInfoMapMutex);
	PlayerInfoMap::const_reverse_iterator i = m_playerInfoMap.rbegin();
	PlayerInfoMap::const_reverse_iterator end = m_playerInfoMap.rend();

	while (i != end)
	{
		if (i->second.playerName == playerName)
		{
			playerId = i->first;
			retVal = true;
			break;
		}
		++i;
	}
	return retVal;
}

ClientCallback &
ClientThread::GetCallback()
{
	return m_gui;
}

GuiInterface &
ClientThread::GetGui()
{
	return m_gui;
}

AvatarManager &
ClientThread::GetAvatarManager()
{
	return m_avatarManager;
}

void
ClientThread::Main()
{
	// Start sub-threads.
	m_senderThread->Start();
	m_avatarDownloader.reset(new DownloaderThread);
	m_avatarDownloader->Run();
	SetState(CLIENT_INITIAL_STATE::Instance());

	// Main loop.
	try
	{
		while (!ShouldTerminate())
		{
			int msg = GetState().Process(*this);
			if (msg != MSG_SOCK_INTERNAL_PENDING)
			{
				if (msg <= MSG_SOCK_LIMIT_CONNECT)
					GetCallback().SignalNetClientConnect(msg);
				else
					GetCallback().SignalNetClientGameInfo(msg);

				// Additionally signal the start of the game.
				if (msg == MSG_NET_GAME_CLIENT_START)
				{
					// EngineFactory erstellen
					boost::shared_ptr<EngineFactory> factory(new ClientEngineFactory); // LocalEngine erstellen

					MapPlayerDataList();
					if (GetPlayerDataList().size() != (unsigned)GetStartData().numberOfPlayers)
						throw ClientException(__FILE__, __LINE__, ERR_NET_INVALID_PLAYER_COUNT, 0);
					m_game.reset(new Game(&m_gui, factory, GetPlayerDataList(), GetGameData(), GetStartData(), m_curGameNum++));
					// Initialize GUI speed.
					GetGui().initGui(GetGameData().guiSpeed);
					// Signal start of game to GUI.
					GetCallback().SignalNetClientGameStart(m_game);
				}
			}
			if (IsSessionEstablished())
				SendPacketLoop();
		}
	} catch (const PokerTHException &e)
	{
		GetCallback().SignalNetClientError(e.GetErrorId(), e.GetOsErrorCode());
	}
	// Terminate sub-threads.
	m_avatarDownloader->SignalTermination();
	m_avatarDownloader->Join(DOWNLOADER_THREAD_TERMINATE_TIMEOUT);
	m_senderThread->SignalStop();
	m_senderThread->WaitStop();
}

void
ClientThread::AddPacket(boost::shared_ptr<NetPacket> packet)
{
	boost::mutex::scoped_lock lock(m_outPacketListMutex);
	m_outPacketList.push_back(packet);
}

void
ClientThread::SendPacketLoop()
{
	boost::mutex::scoped_lock lock(m_outPacketListMutex);

	if (!m_outPacketList.empty())
	{
		NetPacketList::iterator i = m_outPacketList.begin();
		NetPacketList::iterator end = m_outPacketList.end();

		while (i != end)
		{
			GetContext().GetSessionData()->GetSender().Send(GetContext().GetSessionData(), *i);
			++i;
		}
		m_outPacketList.clear();
	}
}

bool
ClientThread::GetCachedPlayerInfo(unsigned id, PlayerInfo &info) const
{
	bool retVal = false;

	boost::mutex::scoped_lock lock(m_playerInfoMapMutex);
	PlayerInfoMap::const_iterator pos = m_playerInfoMap.find(id);
	if (pos != m_playerInfoMap.end())
	{
		info = pos->second;
		retVal = true;
	}
	return retVal;
}

void
ClientThread::RequestPlayerInfo(unsigned id, bool requestAvatar)
{
	if (find(m_playerInfoRequestList.begin(), m_playerInfoRequestList.end(), id) == m_playerInfoRequestList.end())
	{
		boost::shared_ptr<NetPacket> req(new NetPacketRetrievePlayerInfo);
		NetPacketRetrievePlayerInfo::Data reqData;
		reqData.playerId = id;
		static_cast<NetPacketRetrievePlayerInfo *>(req.get())->SetData(reqData);
		GetContext().GetSessionData()->GetSender().Send(GetContext().GetSessionData(), req);

		m_playerInfoRequestList.push_back(id);

	}
	// Remember that we have to request an avatar.
	if (requestAvatar)
	{
		m_avatarShouldRequestList.push_back(id);
	}
}

void
ClientThread::SetPlayerInfo(unsigned id, const PlayerInfo &info)
{
	{
		boost::mutex::scoped_lock lock(m_playerInfoMapMutex);
		// Remove previous player entry with different id
		// for the same player name if it exists.
		// This can only be one entry, since every time a duplicate
		// name is added one is removed.
		// Only erase non computer player entries.
		if (info.playerName.substr(0, sizeof(SERVER_COMPUTER_PLAYER_NAME) - 1) != SERVER_COMPUTER_PLAYER_NAME)
		{
			PlayerInfoMap::iterator i = m_playerInfoMap.begin();
			PlayerInfoMap::iterator end = m_playerInfoMap.end();
			while (i != end)
			{
				if (i->first != id && i->second.playerName == info.playerName)
				{
					m_playerInfoMap.erase(i);
					break;
				}
				++i;
			}
		}
		m_playerInfoMap[id] = info;
	}

	// Update player data for current game.
	boost::shared_ptr<PlayerData> playerData = GetPlayerDataByUniqueId(id);
	if (playerData.get())
	{
		playerData->SetName(info.playerName);
		playerData->SetType(info.ptype);
		if (info.hasAvatar)
		{
			string avatarFile;
			if (GetAvatarManager().GetAvatarFileName(info.avatar, avatarFile))
			{
				playerData->SetAvatarFile(GetQtToolsInterface().stringToUtf8(avatarFile));
			}
		}
	}

	if (find(m_avatarShouldRequestList.begin(), m_avatarShouldRequestList.end(), id) != m_avatarShouldRequestList.end())
	{
		m_avatarShouldRequestList.remove(id);
		// Retrieve avatar if needed.
		RetrieveAvatarIfNeeded(id, info);
	}

	// Remove it from the request list.
	m_playerInfoRequestList.remove(id);

	// Notify GUI
	GetCallback().SignalNetClientPlayerChanged(id, info.playerName);

}

void
ClientThread::SetUnknownPlayer(unsigned id)
{
	// Just remove it from the request list.
	m_playerInfoRequestList.remove(id);
	m_avatarShouldRequestList.remove(id);
	LOG_ERROR("Server reported unknown player id: " << id);
}

void
ClientThread::SetNewGameAdmin(unsigned id)
{
	// Update player data for current game.
	boost::shared_ptr<PlayerData> playerData = GetPlayerDataByUniqueId(id);
	if (playerData.get())
	{
		playerData->SetRights(PLAYER_RIGHTS_ADMIN);
		GetCallback().SignalNetClientNewGameAdmin(id, playerData->GetName());
	}
}

void
ClientThread::RetrieveAvatarIfNeeded(unsigned id, const PlayerInfo &info)
{
	if (find(m_avatarHasRequestedList.begin(), m_avatarHasRequestedList.end(), id) == m_avatarHasRequestedList.end())
	{
		if (info.hasAvatar && !info.avatar.IsZero() && !GetAvatarManager().HasAvatar(info.avatar))
		{
			m_avatarHasRequestedList.push_back(id); // Never remove from this list. Only request once.

			// Download from avatar server if applicable.
			string avatarServerAddress(GetContext().GetAvatarServerAddr());
			if (!avatarServerAddress.empty() && m_avatarDownloader)
			{
				string serverFileName(info.avatar.ToString() + AvatarManager::GetAvatarFileExtension(info.avatarType));
				m_avatarDownloader->QueueDownload(
					id, avatarServerAddress + serverFileName, GetContext().GetCacheDir() + TEMP_AVATAR_FILENAME);
			}
			else
			{
				boost::shared_ptr<NetPacket> retrieveAvatar(new NetPacketRetrieveAvatar);
				NetPacketRetrieveAvatar::Data retrieveAvatarData;
				retrieveAvatarData.requestId = id;
				retrieveAvatarData.avatar = info.avatar;
				static_cast<NetPacketRetrieveAvatar *>(retrieveAvatar.get())->SetData(retrieveAvatarData);
				GetContext().GetSessionData()->GetSender().Send(GetContext().GetSessionData(), retrieveAvatar);
			}
		}
	}
}

void
ClientThread::AddTempAvatarData(unsigned playerId, unsigned avatarSize, AvatarFileType type)
{
	boost::shared_ptr<AvatarData> tmpAvatar(new AvatarData);
	tmpAvatar->fileData.reserve(avatarSize);
	tmpAvatar->fileType = type;
	tmpAvatar->reportedSize = avatarSize;

	m_tempAvatarMap[playerId] = tmpAvatar;
}

void
ClientThread::StoreInTempAvatarData(unsigned playerId, const vector<unsigned char> &data)
{
	AvatarDataMap::iterator pos = m_tempAvatarMap.find(playerId);
	if (pos == m_tempAvatarMap.end())
		throw ClientException(__FILE__, __LINE__, ERR_NET_INVALID_REQUEST_ID, 0);
	// We trust the server (concerning size of the data).
	std::copy(data.begin(), data.end(), back_inserter(pos->second->fileData));
}

void
ClientThread::CompleteTempAvatarData(unsigned playerId)
{
	AvatarDataMap::iterator pos = m_tempAvatarMap.find(playerId);
	if (pos == m_tempAvatarMap.end())
		throw ClientException(__FILE__, __LINE__, ERR_NET_INVALID_REQUEST_ID, 0);
	boost::shared_ptr<AvatarData> tmpAvatar = pos->second;
	unsigned avatarSize = (unsigned)tmpAvatar->fileData.size();
	if (avatarSize != tmpAvatar->reportedSize)
		LOG_ERROR("Client received invalid avatar file size!");
	else
		PassAvatarDataToManager(playerId, tmpAvatar);

	// Free memory.
	m_tempAvatarMap.erase(pos);
}

void
ClientThread::PassAvatarDataToManager(unsigned playerId, boost::shared_ptr<AvatarData> avatarData)
{
	PlayerInfo tmpPlayerInfo;
	if (!GetCachedPlayerInfo(playerId, tmpPlayerInfo))
		LOG_ERROR("Client received invalid player id!");
	else
	{
		if (avatarData->fileType == AVATAR_FILE_TYPE_UNKNOWN)
			avatarData->fileType = tmpPlayerInfo.avatarType;
		if (!GetAvatarManager().StoreAvatarInCache(tmpPlayerInfo.avatar, avatarData->fileType, &avatarData->fileData[0], avatarData->reportedSize, false))
			LOG_ERROR("Failed to store avatar in cache directory.");

		// Update player info, but never re-request avatar.
		SetPlayerInfo(playerId, tmpPlayerInfo);

		string fileName;
		if (GetAvatarManager().GetAvatarFileName(tmpPlayerInfo.avatar, fileName))
		{
			// Dynamically update avatar in GUI.
			GetGui().setPlayerAvatar(playerId, GetQtToolsInterface().stringToUtf8(fileName));
		}
	}
}

void
ClientThread::SetUnknownAvatar(unsigned playerId)
{
	m_tempAvatarMap.erase(playerId);
	LOG_ERROR("Server reported unknown avatar for player: " << playerId);
}

void
ClientThread::CheckAvatarDownloads()
{
	if (m_avatarDownloader && m_avatarDownloader->HasDownloadResult())
	{
		unsigned playerId;
		boost::shared_ptr<AvatarData> tmpAvatar(new AvatarData);
		m_avatarDownloader->GetDownloadResult(playerId, tmpAvatar->fileData);
		tmpAvatar->reportedSize = tmpAvatar->fileData.size();
		PassAvatarDataToManager(playerId, tmpAvatar);
	}
}

void
ClientThread::UnsubscribeLobbyMsg()
{
	if (GetContext().GetSubscribeLobbyMsg())
	{
		// Send unsubscribe request.
		boost::shared_ptr<NetPacket> unsubscr(new NetPacketUnsubscribeGameList);
		GetContext().GetSessionData()->GetSender().Send(GetContext().GetSessionData(), unsubscr);
		GetContext().SetSubscribeLobbyMsg(false);
	}
}

void
ClientThread::ResubscribeLobbyMsg()
{
	if (!GetContext().GetSubscribeLobbyMsg())
	{
		// Clear game info map as it is outdated.
		ClearGameInfoMap();
		// Send resubscribe request.
		boost::shared_ptr<NetPacket> resubscr(new NetPacketResubscribeGameList);
		GetContext().GetSessionData()->GetSender().Send(GetContext().GetSessionData(), resubscr);
		GetContext().SetSubscribeLobbyMsg(true);
	}
}

const ClientContext &
ClientThread::GetContext() const
{
	assert(m_context.get());
	return *m_context;
}

ClientContext &
ClientThread::GetContext()
{
	assert(m_context.get());
	return *m_context;
}

void
ClientThread::CreateContextSession()
{
	bool validSocket = false;
	// TODO ipv6
	// TODO sctp
	try {
		boost::shared_ptr<tcp::socket> newSock(new boost::asio::ip::tcp::socket(*m_ioService, tcp::v4()));
		boost::asio::socket_base::non_blocking_io command(true);
		newSock->io_control(command);
		newSock->set_option(tcp::no_delay(true));
		newSock->set_option(boost::asio::socket_base::keep_alive(true));

		GetContext().SetSessionData(boost::shared_ptr<SessionData>(new SessionData(
			newSock,
			SESSION_ID_GENERIC,
			m_senderThread,
			*m_senderCallback)));
		validSocket = true;
	} catch (...)
	{
	}
	if (!validSocket)
		throw ClientException(__FILE__, __LINE__, ERR_SOCK_CREATION_FAILED, SOCKET_ERRNO());
}

ClientState &
ClientThread::GetState()
{
	assert(m_curState);
	return *m_curState;
}

void
ClientThread::SetState(ClientState &newState)
{
	m_curState = &newState;
}

ReceiverHelper &
ClientThread::GetReceiver()
{
	assert(m_receiver.get());
	return *m_receiver;
}

unsigned
ClientThread::GetGameId() const
{
	boost::mutex::scoped_lock lock(m_curGameIdMutex);
	return m_curGameId;
}

void
ClientThread::SetGameId(unsigned id)
{
	boost::mutex::scoped_lock lock(m_curGameIdMutex);
	m_curGameId = id;
}

const GameData &
ClientThread::GetGameData() const
{
	return m_gameData;
}

void
ClientThread::SetGameData(const GameData &gameData)
{
	m_gameData = gameData;
}

const StartData &
ClientThread::GetStartData() const
{
	return m_startData;
}

void
ClientThread::SetStartData(const StartData &startData)
{
	m_startData = startData;
}

unsigned
ClientThread::GetGuiPlayerId() const
{
	return m_guiPlayerId;
}

void
ClientThread::SetGuiPlayerId(unsigned guiPlayerId)
{
	m_guiPlayerId = guiPlayerId;
}

boost::shared_ptr<Game>
ClientThread::GetGame()
{
	return m_game;
}

QtToolsInterface &
ClientThread::GetQtToolsInterface()
{
	assert(myQtToolsInterface.get());
	return *myQtToolsInterface;
}

void
ClientThread::AddPlayerData(boost::shared_ptr<PlayerData> playerData)
{
	if (playerData.get() && !playerData->GetName().empty())
	{
		m_playerDataList.push_back(playerData);
		if (playerData->GetUniqueId() == GetGuiPlayerId())
			GetCallback().SignalNetClientSelfJoined(playerData->GetUniqueId(), playerData->GetName(), playerData->GetRights());
		else
			GetCallback().SignalNetClientPlayerJoined(playerData->GetUniqueId(), playerData->GetName(), playerData->GetRights());
	}
}

void
ClientThread::RemovePlayerData(unsigned playerId, int removeReason)
{
	boost::shared_ptr<PlayerData> tmpData;

	PlayerDataList::iterator i = m_playerDataList.begin();
	PlayerDataList::iterator end = m_playerDataList.end();
	while (i != end)
	{
		if ((*i)->GetUniqueId() == playerId)
		{
			tmpData = *i;
			m_playerDataList.erase(i);
			break;
		}
		++i;
	}

	if (tmpData.get())
	{
		// Remove player from gui.
		GetCallback().SignalNetClientPlayerLeft(tmpData->GetUniqueId(), tmpData->GetName(), removeReason);
	}
}

void
ClientThread::ClearPlayerDataList()
{
	m_playerDataList.clear();
}

void
ClientThread::MapPlayerDataList()
{
	// Retrieve the GUI player.
	boost::shared_ptr<PlayerData> guiPlayer = GetPlayerDataByUniqueId(GetGuiPlayerId());
	assert(guiPlayer.get());
	int guiPlayerNum = guiPlayer->GetNumber();

	// Create a copy of the player list so that the GUI player
	// is player 0. This is mapped because the GUI depends on it.
	PlayerDataList mappedList;

	PlayerDataList::const_iterator i = m_playerDataList.begin();
	PlayerDataList::const_iterator end = m_playerDataList.end();
	int numPlayers = GetStartData().numberOfPlayers;

	while (i != end)
	{
		boost::shared_ptr<PlayerData> tmpData(new PlayerData(*(*i)));
		int numberDiff = tmpData->GetNumber() - guiPlayerNum;
		if (numberDiff >= 0)
			tmpData->SetNumber(numberDiff);
		else
			tmpData->SetNumber(numPlayers + numberDiff);
		mappedList.push_back(tmpData);
		++i;
	}

	// Sort the list by player number.
	mappedList.sort(*boost::lambda::_1 < *boost::lambda::_2);

	m_playerDataList = mappedList;
}

const PlayerDataList &
ClientThread::GetPlayerDataList() const
{
	return m_playerDataList;
}

boost::shared_ptr<PlayerData>
ClientThread::GetPlayerDataByUniqueId(unsigned id)
{
	boost::shared_ptr<PlayerData> tmpPlayer;

	PlayerDataList::const_iterator i = m_playerDataList.begin();
	PlayerDataList::const_iterator end = m_playerDataList.end();

	while (i != end)
	{
		if ((*i)->GetUniqueId() == id)
		{
			tmpPlayer = *i;
			break;
		}
		++i;
	}
	return tmpPlayer;
}

boost::shared_ptr<PlayerData>
ClientThread::GetPlayerDataByName(const std::string &name)
{
	boost::shared_ptr<PlayerData> tmpPlayer;

	if (!name.empty())
	{
		PlayerDataList::const_iterator i = m_playerDataList.begin();
		PlayerDataList::const_iterator end = m_playerDataList.end();

		while (i != end)
		{
			if ((*i)->GetName() == name)
			{
				tmpPlayer = *i;
				break;
			}
			++i;
		}
	}
	return tmpPlayer;
}

void
ClientThread::RemoveDisconnectedPlayers()
{
	// This should only be called between hands.
	if (m_game.get())
	{
		PlayerListIterator it;
		for (it = m_game->getSeatsList()->begin(); it != m_game->getSeatsList()->end(); it++)
		{
			boost::shared_ptr<PlayerInterface> tmpPlayer = *it;
			if (tmpPlayer->getMyActiveStatus())
			{
				// If a player is not in the player data list, it was disconnected.
				if (!GetPlayerDataByUniqueId(tmpPlayer->getMyUniqueID()).get())
				{
					tmpPlayer->setMyCash(0);
					tmpPlayer->setMyActiveStatus(false);
				}
			}
		}
	}
}

void
ClientThread::AddServerInfo(unsigned serverId, const ServerInfo &info)
{
	{
		boost::mutex::scoped_lock lock(m_serverInfoMapMutex);
		m_serverInfoMap.insert(ServerInfoMap::value_type(serverId, info));
	}
	GetCallback().SignalNetClientServerListAdd(serverId);
}

void
ClientThread::ClearServerInfoMap()
{
	{
		boost::mutex::scoped_lock lock(m_serverInfoMapMutex);
		m_serverInfoMap.clear();
	}
	GetCallback().SignalNetClientServerListClear();
}

bool
ClientThread::GetSelectedServer(unsigned &serverId) const
{
	bool retVal = false;
	boost::mutex::scoped_lock lock(m_selectServerMutex);
	if (m_isServerSelected)
	{
		retVal = true;
		serverId = m_selectedServerId;
	}
	return retVal;
}

void
ClientThread::UseServer(unsigned serverId)
{
	ClientContext &context = GetContext();
	ServerInfo useInfo(GetServerInfo(serverId));

	if (context.GetAddrFamily() == AF_INET6)
		context.SetServerAddr(useInfo.ipv6addr);
	else
		context.SetServerAddr(useInfo.ipv4addr);

	context.SetServerPort((unsigned)useInfo.port);
	context.SetAvatarServerAddr(useInfo.avatarServerAddr);
}

unsigned
ClientThread::GetGameIdByName(const std::string &name) const
{
	// Find the game.
	boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
	GameInfoMap::const_iterator i = m_gameInfoMap.begin();
	GameInfoMap::const_iterator end = m_gameInfoMap.end();
	while (i != end)
	{
		if (i->second.name == name)
			break;
		++i;
	}

	if (i == end)
		throw ClientException(__FILE__, __LINE__, ERR_NET_UNKNOWN_GAME, 0);
	return i->first;
}

void
ClientThread::AddGameInfo(unsigned gameId, const GameInfo &info)
{
	{
		boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
		m_gameInfoMap.insert(GameInfoMap::value_type(gameId, info));
	}
	GetCallback().SignalNetClientGameListNew(gameId);
}

void
ClientThread::UpdateGameInfoMode(unsigned gameId, GameMode mode)
{
	bool found = false;
	{
		boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
		GameInfoMap::iterator pos = m_gameInfoMap.find(gameId);
		if (pos != m_gameInfoMap.end())
		{
			found = true;
			(*pos).second.mode = mode;
		}
	}
	if (found)
		GetCallback().SignalNetClientGameListUpdateMode(gameId, mode);
}

void
ClientThread::UpdateGameInfoAdmin(unsigned gameId, unsigned adminPlayerId)
{
	bool found = false;
	{
		boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
		GameInfoMap::iterator pos = m_gameInfoMap.find(gameId);
		if (pos != m_gameInfoMap.end())
		{
			found = true;
			(*pos).second.adminPlayerId = adminPlayerId;
		}
	}
	if (found)
		GetCallback().SignalNetClientGameListUpdateAdmin(gameId, adminPlayerId);
}

void
ClientThread::RemoveGameInfo(unsigned gameId)
{
	bool found = false;
	{
		boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
		GameInfoMap::iterator pos = m_gameInfoMap.find(gameId);
		if (pos != m_gameInfoMap.end())
		{
			found = true;
			m_gameInfoMap.erase(pos);
		}
	}
	if (found)
		GetCallback().SignalNetClientGameListRemove(gameId);
}

void
ClientThread::ModifyGameInfoAddPlayer(unsigned gameId, unsigned playerId)
{
	bool playerAdded = false;
	{
		boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
		GameInfoMap::iterator pos = m_gameInfoMap.find(gameId);
		if (pos != m_gameInfoMap.end())
		{
			pos->second.players.push_back(playerId);
			playerAdded = true;
		}
	}
	if (playerAdded)
		GetCallback().SignalNetClientGameListPlayerJoined(gameId, playerId);
}

void
ClientThread::ModifyGameInfoRemovePlayer(unsigned gameId, unsigned playerId)
{
	bool playerRemoved = false;
	{
		boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
		GameInfoMap::iterator pos = m_gameInfoMap.find(gameId);
		if (pos != m_gameInfoMap.end())
		{
			pos->second.players.remove(playerId);
			playerRemoved = true;
		}
	}
	if (playerRemoved)
		GetCallback().SignalNetClientGameListPlayerLeft(gameId, playerId);
}

void
ClientThread::ClearGameInfoMap()
{
	boost::mutex::scoped_lock lock(m_gameInfoMapMutex);
	m_gameInfoMap.clear();
}

void
ClientThread::StartPetition(unsigned petitionId, unsigned proposingPlayerId, unsigned kickPlayerId, int timeoutSec, int numVotesToKick)
{
	{
		boost::mutex::scoped_lock lock(m_curPetitionIdMutex);
		m_curPetitionId = petitionId;
	}
	GetGui().startVoteOnKick(kickPlayerId, proposingPlayerId, timeoutSec, numVotesToKick);
	if (GetGuiPlayerId() != kickPlayerId
		&& GetGuiPlayerId() != proposingPlayerId)
	{
		GetGui().changeVoteOnKickButtonsState(true);
	}
}

void
ClientThread::UpdatePetition(unsigned petitionId, int /*numVotesAgainstKicking*/, int numVotesInFavourOfKicking, int numVotesToKick)
{
	bool isCurPetition;
	{
		boost::mutex::scoped_lock lock(m_curPetitionIdMutex);
		isCurPetition = m_curPetitionId == petitionId;
	}
	if (isCurPetition)
	{
		GetGui().refreshVotesMonitor(numVotesInFavourOfKicking, numVotesToKick);
	}
}

void
ClientThread::EndPetition(unsigned petitionId)
{
	bool isCurPetition;
	{
		boost::mutex::scoped_lock lock(m_curPetitionIdMutex);
		isCurPetition = m_curPetitionId == petitionId;
	}
	if (isCurPetition)
		GetGui().endVoteOnKick();
}

void
ClientThread::UpdateStatData(const ServerStats &stats)
{
	boost::mutex::scoped_lock lock(m_curStatsMutex);
	if (stats.numberOfPlayersOnServer)
		m_curStats.numberOfPlayersOnServer = stats.numberOfPlayersOnServer;

	if (stats.totalPlayersEverLoggedIn)
		m_curStats.totalPlayersEverLoggedIn = stats.totalPlayersEverLoggedIn;

	if (stats.totalGamesEverCreated)
		m_curStats.totalGamesEverCreated = stats.totalGamesEverCreated;

	GetCallback().SignalNetClientStatsUpdate(m_curStats);
}

ServerStats
ClientThread::GetStatData() const
{
	boost::mutex::scoped_lock lock(m_curStatsMutex);
	return m_curStats;
}

bool
ClientThread::IsSessionEstablished() const
{
	return m_sessionEstablished;
}

void
ClientThread::SetSessionEstablished(bool flag)
{
	m_sessionEstablished = flag;
}

bool
ClientThread::IsSynchronized() const
{
	return m_playerInfoRequestList.empty();
}

