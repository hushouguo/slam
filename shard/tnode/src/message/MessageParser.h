/*
 * \file: MessageParser.h
 * \brief: Created by hushouguo at 09:23:36 Apr 17 2019
 */
 
#ifndef __MESSAGEPARSER_H__
#define __MESSAGEPARSER_H__

using namespace google::protobuf;
using namespace google::protobuf::compiler;

BEGIN_NAMESPACE_TNODE {
	class MessageParser {
		public:
			MessageParser();
			~MessageParser();

		public:
			//
			// load `proto` file or load all `protos` in the folder
			bool LoadMessageDescriptor(const char* filename);/* filename also can be directory */
			//
			// register msgid => name of protobuf::Message
			bool RegisteMessage(u32 msgid, const char* name);
			//
			// allocate NEW protobuf::Message, not push it into cache
			Message* NewMessage(u32 msgid);
			Message* NewMessage(const char* name);
			//
			// get a protobuf::Message from cache, if not exists, call `newmsg` to allocate new message
			Message* GetMessage(u32 msgid);

		public:
			//
			// decode buffer to NEW protobuf::Message
			Message* DecodeToNewMessage(u32 msgid, const std::string& in);
			Message* DecodeToNewMessage(u32 msgid, const void* buf, size_t bufsize);

			//
			// decode buffer to protobuf::Message
			Message* DecodeToMessage(u32 msgid, const std::string& in);
			Message* DecodeToMessage(u32 msgid, const void* buf, size_t bufsize);
			
			//
			// get descriptor of message
			const Descriptor* FindMessageDescriptor(Message* message);

			//
			// get Message Factory
			MessageFactory* GetMessageFactory();

			//
			// merge two protobuf::Messages
			bool MergeMessage(Message* dest, const Message* src);
			
		private:
			DiskSourceTree _tree;
			Importer* _in = nullptr;
			DynamicMessageFactory _factory;
			std::unordered_map<u32, Message*> _messages;
			class ImporterErrorCollector : public MultiFileErrorCollector {
				public:
					// implements ErrorCollector ---------------------------------------
					void AddError(const std::string& filename, int line, int column,	const std::string& message) override {
						Error("file: %s:%d:%d, error: %s", filename.c_str(), line, column, message.c_str());
					}
					void AddWarning(const std::string& filename, int line, int column, const std::string& message) override {
						Error("file: %s:%d:%d, error: %s", filename.c_str(), line, column, message.c_str());
					}
			};
			ImporterErrorCollector _errorCollector;
			bool ParseProtoFile(const char* filename);
	};
}

#endif
