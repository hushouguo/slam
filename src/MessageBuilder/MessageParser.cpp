/*
 * \file: MessageParser.cpp
 * \brief: Created by hushouguo at 16:50:42 Jul 30 2019
 */

#include "protocol.h"

//
// test for the file is a directory
bool isDir(const char* file) {
	struct stat buf;
	if (stat(file, &buf) != 0) { return false; }
	return S_ISDIR(buf.st_mode);
}

//
// iterate specifying folder
bool traverseDirectory(const char* folder, const char* filter_suffix, std::function<bool(const char*)>& callback) {
	if (!isDir(folder)) {
		return callback(folder);
	}

	DIR* dir = opendir(folder);

	struct dirent* ent;
	while ((ent = readdir(dir)) != nullptr) {
		if (ent->d_name[0] == '.') { continue; } //filter hide file

		if (filter_suffix != nullptr) {
			char* suffix = strrchr(ent->d_name, '.');//filter not .proto suffix file 
			if (!suffix || strcasecmp(suffix, filter_suffix) != 0) {
				continue; 
			}
		}

		char fullname[PATH_MAX];
		snprintf(fullname, sizeof(fullname), "%s/%s", folder, ent->d_name);
		if (ent->d_type & DT_DIR) {
			return traverseDirectory(fullname, filter_suffix, callback);
		}
		else {
			if (!callback(fullname)) { return false; }
		}
	}

	return true;
}

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
bool MessageParser::RegisteMessage(msgid_t msgid, const char* name) {
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
Message* MessageParser::NewMessage(msgid_t msgid) {
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
Message* MessageParser::GetMessage(msgid_t msgid) {
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
Message* MessageParser::DecodeToNewMessage(msgid_t msgid, const std::string& in) {
	//
	// allocate NEW protobuf::Message
	Message* message = this->NewMessage(msgid);
	assert(message->ByteSize() == 0);
	if (!message->ParseFromString(in)) {
		SafeDelete(message);
		CHECK_RETURN(false, nullptr, "DecodeToNewMessage failure, strlen: %ld, msgid: %d", in.length(), msgid);
	}
	return message;
}

Message* MessageParser::DecodeToNewMessage(msgid_t msgid, const void* buf, size_t bufsize) {
	//
	// allocate NEW protobuf::Message
	Message* message = this->NewMessage(msgid);
	assert(message->ByteSize() == 0);
	if (!message->ParseFromArray(buf, bufsize)) {
		SafeDelete(message);
		CHECK_RETURN(false, nullptr, "DecodeToNewMessage failure, bufsize: %ld, msgid: %d", bufsize, msgid);
	}
	return message;
}

//
// decode buffer to protobuf::Message
Message* MessageParser::DecodeToMessage(msgid_t msgid, const std::string& in) {
	Message* message = this->GetMessage(msgid);
	message->Clear();
	if (!message->ParseFromString(in)) {
		SafeDelete(message);
		CHECK_RETURN(false, nullptr, "DecodeToNewMessage failure, strlen: %ld, msgid: %d", in.length(), msgid);
	}
	return message;
}

Message* MessageParser::DecodeToMessage(msgid_t msgid, const void* buf, size_t bufsize) {
	Message* message = this->GetMessage(msgid);
	message->Clear();
	if (!message->ParseFromArray(buf, bufsize)) {
		SafeDelete(message);
		CHECK_RETURN(false, nullptr, "DecodeToNewMessage failure, bufsize: %ld, msgid: %d", bufsize, msgid);
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
	// Merge the fields from the given message into this message.  Singular
	// fields will be overwritten, if specified in from, except for embedded
	// messages which will be merged.  Repeated fields will be concatenated.
	// The given message must be of the same type as this message (i.e. the
	// exact same class).
	//
	// NOTE: `dest` and `src` must be of the same type as this message!!
	try {
		dest->MergeFrom(*src);
	}
	catch (std::exception& e) {
		CHECK_RETURN(false, false, "MergeMessage exception:%s", e.what());
	}
	return true;
}

