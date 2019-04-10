/*
 * \file: test_easynet.cpp
 * \brief: Created by hushouguo at 11:15:22 Apr 10 2019
 */

#include <random>

#include "Easylog.h"
#include "Easynet.h"

//using namespace net;

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
			return len;
			//NetMessage* netmsg = (NetMessage*) buffer;
			//return len < sizeof(NetMessage) || len < netmsg->len ? 0 : netmsg->len;
			});

	SOCKET ss = easynet->createServer("127.0.0.1", 12306);
	assert(ss != -1);

	SOCKET cs = easynet->createClient("127.0.0.1", 12306);
	assert(cs != -1);

	Debug.cout("socketServer: %d, socketClient: %d", ss, cs);

	// server thread
	std::thread* ts = new std::thread([](net::Easynet* easynet, SOCKET s) {
			uint32_t total = 0;
			while (true) {
				SOCKET fd = -1;
				const void* msg = easynet->getMessage(&fd);
				if (msg) {
					size_t len = 0;
					const void* payload = easynet->getMessageContent(msg, &len);
					//Debug.cout("Server recv: %s(%ld) from (%d)", (const char*) payload, len, fd);
					fprintf(stderr, "Server recv: %s(%ld) from (%d)\n", (const char*) payload, len, fd);
					easynet->releaseMessage(msg);

					++total;
					if (total >= 5) {
						total = 0;
						std::string content;
						size_t len = randomBetween(1, 10);
						randomString(content, len, true, false, false);
						content += "\0"; len++;
						//Debug.cout("Server send: %s(%ld) to (%d)", content.c_str(), content.length(), fd);
						fprintf(stderr, "Server send: %s(%ld) to (%d)\n", content.c_str(), content.length(), fd);
						void* newmsg = easynet->allocateMessage(content.length());
						easynet->setMessageContent(newmsg, content.data(), content.length());
						easynet->sendMessage(fd, newmsg);
					}
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
			}, easynet, ss);

	// client thread
	std::thread* tc = new std::thread([](net::Easynet* easynet, SOCKET s) {
			uint32_t total = 0;
			while (true) {
				SOCKET fd = -1;
				const void* msg = easynet->getMessage(&fd);
				if (msg) {
					size_t len = 0;
					const void* payload = easynet->getMessageContent(msg, &len);
					//Debug.cout("Client recv: %s(%ld) from (%d)", (const char*) payload, len, fd);
					fprintf(stderr, "Client recv: %s(%ld) from (%d)\n", (const char*) payload, len, fd);
					easynet->releaseMessage(msg);
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					++total;
					if (total >= 1000) {
						total = 0;
						std::string content;
						size_t len = randomBetween(10, 20);
						randomString(content, len, false, true, true);
						content += "\0"; len++;
						//Debug.cout("Client send: %s(%ld) to (%d)", content.c_str(), content.length(), s);
						fprintf(stderr, "Client send: %s(%ld) to (%d)\n", content.c_str(), content.length(), s);
						void* newmsg = easynet->allocateMessage(content.length());
						easynet->setMessageContent(newmsg, content.data(), content.length());
						easynet->sendMessage(s, newmsg);
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

