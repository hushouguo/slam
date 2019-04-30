/*
 * \file: CentralClient.cpp
 * \brief: Created by hushouguo at 13:43:33 Apr 30 2019
 */

BEGIN_NAMESPACE_SLAM {
	bool CentralClient::init() {
		this->_easynet = Easynet::createInstance(
			[](const void* buffer, size_t len) -> int {
				CommonMessage* msg = (CommonMessage*) buffer;
				return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
			[this]() {
				});
	}

	void CentralClient::stop() {
		if (this->_easynet) {
			this->_easynet->stop();
		}
	}

	void CentralClient::run() {
	}	

	INITIALIZE_INSTANCE(CentralClient);
}

