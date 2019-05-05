/*
 * \file: ClientTask.h
 * \brief: Created by hushouguo at 13:43:04 Apr 30 2019
 */
 
#ifndef __CLIENTTASK_H__
#define __CLIENTTASK_H__

BEGIN_NAMESPACE_SLAM {
	class ClientTask : public Entry<SOCKET> {
		public:
			ClientTask(Easynet* easynet, SOCKET socket);
			const char* getClassName() override { return "ClientTask"; }

		public:
			inline u64 entityid() { return this->_entityid; }
			inline void set_entityid(u64 value) { this->_entityid = value; }
			inline Easynet* easynet() { return this->_easynet; }
		
		public:	
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			u64 _entityid = 0;
			Easynet* _easynet = nullptr;
	};
}

#endif
