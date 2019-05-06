/*
 * \file: CentralTask.h
 * \brief: Created by hushouguo at 13:43:04 Apr 30 2019
 */
 
#ifndef __CENTRALTASK_H__
#define __CENTRALTASK_H__

BEGIN_NAMESPACE_SLAM {
	class CentralTask : public Entry<SOCKET> {
		public:
			CentralTask(Easynet* easynet, SOCKET socket);
			const char* getClassName() override { return "CentralTask"; }

		public:
			inline Easynet* easynet() { return this->_easynet; }
		
		public:	
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
	};
}

#endif
