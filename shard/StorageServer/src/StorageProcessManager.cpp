/*
 * \file: StorageProcessManager.cpp
 * \brief: Created by hushouguo at 10:21:48 May 06 2019
 */

BEGIN_NAMESPACE_SLAM {
	bool StorageProcessManager::init(int number) {
		this->_size = number;
		if (this->_size <= 0) {
			this->_size = std::thread::hardware_concurrency();
		}
		assert(this->_size > 0);
		for (int n = 0; n < this->_size; ++n) {
			StorageProcess* process = new StorageProcess(n);
			CHECK_RETURN(process->init(), false, "process init failure");
			this->add(process);
		}
		Trace << "spawn StorageProcess: " << this->_size;
		return true;
	}

	void StorageProcessManager::stop() {
		struct StorageProcessCallback : public Callback<StorageProcess> {
			bool invoke(StorageProcess* entry) {
				SafeDelete(entry);				
				return true;
			}
		}eee;
		this->traverse(eee);
		this->clear();
	}

	void StorageProcessManager::serialize(Easynet* easynet, SOCKET socket, StorageSerializeRequest* request) {
		int id = request->entity().id() % this->_size;
		if (request->entity().id() == 0) {
			id = randomBetween(0, this->_size);
		}
		StorageProcess* process = this->find(id);
		assert(process);
		sThreadPool.add([process](Easynet* easynet, SOCKET socket, StorageSerializeRequest* request) {
				process->serialize(easynet, socket, request);
				}, easynet, socket, request);	
	}
	
	void StorageProcessManager::unserialize(Easynet* easynet, SOCKET socket, StorageUnserializeRequest* request) {
		int id = request->entityid() % this->_size;
		StorageProcess* process = this->find(id);
		assert(process);
		sThreadPool.add([process](Easynet* easynet, SOCKET socket, StorageUnserializeRequest* request) {
				process->unserialize(easynet, socket, request);
				}, easynet, socket, request);
	}

	INITIALIZE_INSTANCE(StorageProcessManager);
}

