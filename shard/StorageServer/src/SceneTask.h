/*
 * \file: SceneTask.h
 * \brief: Created by hushouguo at 11:18:18 May 06 2019
 */
 
#ifndef __SCENETASK_H__
#define __SCENETASK_H__

BEGIN_NAMESPACE_SLAM {
	class SceneTask : public Entry<SOCKET> {
		public:
			SceneTask(Easynet* easynet, SOCKET socket);
			const char* getClassName() override { return "SceneTask"; }

		public:
			inline Easynet* easynet() { return this->_easynet; }
		
		public:	
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
	};
}

#endif
