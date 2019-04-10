/*
 * \file: test_easynet.cpp
 * \brief: Created by hushouguo at 11:15:22 Apr 10 2019
 */

#include <random>

#include "Easylog.h"
#include "Easynet.h"

//using namespace net;

#pragma pack(push, 1)
	struct NetMessage {
		uint32_t len;
		uint16_t msgid;
		char payload[0];
	};
#pragma pack(pop)	

	thread_local int _randomSeed = std::time(nullptr) + ::getpid();
	thread_local std::default_random_engine _randomEngine(_randomSeed);

	//
	// random between int, long, long long, float or double, [min, max]
	int randomBetween(int min, int max) {
		//std::default_random_engine randomEngine(_randomSeed);
		std::uniform_int_distribution<int> dist(min, max);//[min, max]
		return dist(_randomEngine);
	}

	static char _alphabet[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
	};

	//
	// random a string
	void randomString(std::string& result, size_t len, bool has_digit, bool has_lowercase, bool has_uppercase) {
		while (len > 0 && (has_digit || has_lowercase || has_uppercase)) {
			int i = randomBetween(0, sizeof(_alphabet) - 1);
			assert(i >= 0 && i < (int)sizeof(_alphabet));
			char c = _alphabet[i];
			if ((has_digit && std::isdigit(c)) 
					|| (has_lowercase && std::islower(c))
					|| (has_uppercase && std::isupper(c))) {
				--len;
				result.push_back(c);
			}
		}
	}

void test_easynet() {	
	net::Easynet* easynet = net::Easynet::createInstance([](const void* buffer, size_t len) -> int {
			NetMessage* netmsg = (NetMessage*) buffer;
			return len < sizeof(NetMessage) || len < netmsg->len ? 0 : netmsg->len;
			});

	SOCKET ss = easynet->createServer("127.0.0.1", 12306);
	assert(ss != -1);

	SOCKET cs = easynet->createClient("127.0.0.1", 12306);
	assert(cs != -1);

	std::thread* ts = new std::thread([](net::Easynet* easynet, SOCKET s) {
			while (true) {
				const net::SocketMessage* msg = easynet->getMessage();
				if (msg) {
					assert(msg->payload_len >= sizeof(NetMessage));
					NetMessage* netmsg = (NetMessage*) msg->payload;
					Debug << "Server recv netmsg: len: " << netmsg->len << ", msgid: " << netmsg->msgid;
					Debug << "    content: " << netmsg->payload;
					net::releaseSocketMessage(msg);
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
			}, easynet, ss);

	std::thread* tc = new std::thread([](net::Easynet* easynet, SOCKET s) {
			uint32_t total = 0;
			while (true) {
				const net::SocketMessage* msg = easynet->getMessage();
				if (msg) {
					assert(msg->payload_len >= sizeof(NetMessage));
					NetMessage* netmsg = (NetMessage*) msg->payload;
					Debug << "Client recv netmsg: len: " << netmsg->len << ", msgid: " << netmsg->msgid;
					Debug << "    content: " << netmsg->payload;
					net::releaseSocketMessage(msg);
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					++total;
					if (total >= 1000) {
						total = 0;
						std::string content;
						size_t len = randomBetween(10, 100);
						randomString(content, len, true, true, true);
						net::SocketMessage* newmsg = net::allocateSocketMessage(content.length());
						newmsg->fd = s;
						newmsg->payload_len = content.length();
						memcpy(newmsg->payload, content.data(), content.length());
						easynet->sendMessage(newmsg);
					}
				}
			}
			}, easynet, cs);

	tc->join();
	ts->join();
	easynet->stop();
	delete tc;
	delete ts;
	delete easynet;
	Debug << "easynet test OK";
}

int main() {
	logger::Easylog::syslog()->set_autosplit_day(true);
	logger::Easylog::syslog()->set_destination(".logs");
	logger::Easylog::syslog()->set_tofile(logger::GLOBAL, "tests.easynet");

	test_easynet();
	logger::Easylog::syslog()->stop();
	return 0;
}

