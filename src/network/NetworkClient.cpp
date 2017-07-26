#include <network/NetworkClient.hpp>

NetworkClient::NetworkClient(std::string address, unsigned short port, std::string username, bool &quit)
		: network(address, port), isRunning(quit) {
	state = Handshaking;

	PacketHandshake handshake;
	handshake.protocolVersion = 335;
	handshake.serverAddress = "127.0.0.1";
	handshake.serverPort = 25565;
	handshake.nextState = 2;
	network.SendPacket(handshake);
	state = Login;

	PacketLoginStart loginStart;
	loginStart.Username = "HelloOne";
	network.SendPacket(loginStart);

	auto response = std::static_pointer_cast<PacketLoginSuccess>(network.ReceivePacket(Login));
	if (response->Username != username) {
		throw std::logic_error("Received username is not sended username");
	}

	state = Play;

	isActive = true;
	std::thread thread(&NetworkClient::NetworkLoop, this);
	std::swap(networkThread, thread);
}

NetworkClient::~NetworkClient() {
	isActive = false;
	networkThread.join();
}

std::shared_ptr<Packet> NetworkClient::ReceivePacket() {
	if (toReceive.empty())
		return std::shared_ptr<Packet>(nullptr);
	toReceiveMutex.lock();
	auto ret = toReceive.front();
	toReceive.pop();
	toReceiveMutex.unlock();
	return ret;
}

void NetworkClient::SendPacket(std::shared_ptr<Packet> packet) {
	toSendMutex.lock();
	toSend.push(packet);
	toSendMutex.unlock();
}

void NetworkClient::NetworkLoop() {
	auto timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();
	el::Helpers::setThreadName("Network");
	LOG(INFO) << "Network thread is started";
	try {
		while (isActive) {
			toSendMutex.lock();
			while (!toSend.empty()) {
				if (toSend.front()!=nullptr)
					network.SendPacket(*toSend.front());
				toSend.pop();
			}
			toSendMutex.unlock();
			auto packet = network.ReceivePacket(state);
			if (packet.get() != nullptr) {
				if (packet->GetPacketId() != PacketNamePlayCB::KeepAliveCB) {
					toReceiveMutex.lock();
					toReceive.push(packet);
					toReceiveMutex.unlock();
				} else {
					timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();
					auto packetKeepAlive = std::static_pointer_cast<PacketKeepAliveCB>(packet);
					auto packetKeepAliveSB = std::make_shared<PacketKeepAliveSB>(packetKeepAlive->KeepAliveId);
					network.SendPacket(*packetKeepAliveSB);
				}
			}
			using namespace std::chrono_literals;
			if (std::chrono::steady_clock::now() - timeOfLastKeepAlivePacket > 20s) {
				auto disconnectPacket = std::make_shared<PacketDisconnectPlay>();
				disconnectPacket->Reason = "Timeout";
				toReceiveMutex.lock();
				toReceive.push(disconnectPacket);
				toReceiveMutex.unlock();
				break;
			}
		}
	} catch (std::exception &e) {
		LOG(ERROR) << "Exception catched in NetworkLoop: " << e.what();
		isRunning = false;
	}
	LOG(INFO) << "Network thread is stopped";
}