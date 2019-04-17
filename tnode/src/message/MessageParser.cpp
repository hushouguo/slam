/*
 * \file: MessageParser.cpp
 * \brief: Created by hushouguo at 09:24:04 Apr 17 2019
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "MessageParser.h"

BEGIN_NAMESPACE_TNODE {
	MessageParser::MessageParser() {
		this->_tree.MapPath("", "./");
		this->_in = new Importer(&this->_tree, &this->_errorCollector);
	}

	MessageParser::~MessageParser() {
		for (auto& i : this->_messages) {
			delete i.second;
		}
		this->_messages.clear();
		SafeDelete(this->_in);
	}

	bool MessageParser::ParseProtoFile(const char* filename) {
		const FileDescriptor* fileDescriptor = this->_in->Import(filename);
		CHECK_RETURN(fileDescriptor, false, "import file: %s failure", filename);
		//Debug << "import file: " << filename;
		return true;
	}
	
	//
	// load `proto` file or load all `protos` in the folder
	bool MessageParser::LoadMessageDescriptor(const char* filename) {
        std::function<bool(const char*)> func = [this](const char* fullname)->bool {
            return this->ParseProtoFile(fullname);
        };  
        return traverseDirectory(filename, ".proto", std::ref(func));	
	}
	
	//
	// register msgid => name of protobuf::Message
	bool MessageParser::RegisteMessage(u32 msgid, const char* name) {
        Message* message = FindOrNull(this->_messages, msgid);
        CHECK_RETURN(message == nullptr, false, "duplicate RegisteMessage: %d, name: %s", msgid, name);
        //
        // preallocate protobuf::Message to cache
        message = this->NewMessage(name);
        if (message) {
			//Debug << "message: " << message->DebugString();
        	this->_messages.insert(std::make_pair(msgid, message));
        }
        return message != nullptr;
    }
	
	//
	// allocate new protobuf::Message
	Message* MessageParser::NewMessage(u32 msgid) {
		Message* message = FindOrNull(this->_messages, msgid);
        CHECK_RETURN(message, nullptr, "message: %d not register", msgid);
        return this->NewMessage(message->GetTypeName().c_str());
	}
	
	Message* MessageParser::NewMessage(const char* name) {
        const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(name);
        CHECK_RETURN(descriptor, nullptr, "not found descriptor for message: %s", name);
        const Message* prototype = this->_factory.GetPrototype(descriptor);
        CHECK_RETURN(prototype, nullptr, "not found prototype for message");
        return prototype->New();
	}
	
	//
	// get a protobuf::Message from cache, if not exists, call `newmsg` to allocate new message
	Message* MessageParser::GetMessage(u32 msgid) {
		Message* message = FindOrNull(this->_messages, msgid);
		if (!message) {
			message = this->NewMessage(msgid);
			if (message) {
				this->_messages.insert(std::make_pair(msgid, message));
			}
		}
		return message;
	}

	//
	// decode buffer to NEW protobuf::Message
	Message* MessageParser::DecodeToMessage(u32 msgid, const std::string& in) {
        //
        // allocate NEW protobuf::Message
		Message* message = this->NewMessage(msgid);
        assert(message->ByteSize() == 0);
        if (!message->ParseFromString(in)) {
            SafeDelete(message);
            CHECK_RETURN(false, nullptr, "DecodeToMessage failure, strlen: %ld, msgid: %d", in.length(), msgid);
        }
        return message;
	}
	
	Message* MessageParser::DecodeToMessage(u32 msgid, const void* buf, size_t bufsize) {
        //
        // allocate NEW protobuf::Message
		Message* message = this->NewMessage(msgid);
        assert(message->ByteSize() == 0);
        if (!message->ParseFromArray(buf, bufsize)) {
            SafeDelete(message);
            CHECK_RETURN(false, nullptr, "DecodeToMessage failure, bufsize: %ld, msgid: %d", bufsize, msgid);
        }
        return message;
    }
    
	const Descriptor* MessageParser::FindMessageDescriptor(Message* message) {
		return this->_in->pool()->FindMessageTypeByName(message->GetTypeName());
	}

	MessageFactory* MessageParser::GetMessageFactory() {
		return &this->_factory;
	}

	bool MessageParser::MergeMessage(Message* dest, const Message* src) {
		//TODO:
		return false;
	}
}

