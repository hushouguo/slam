 /*
 * \file: Easylog.cpp
 * \brief: Created by hushouguo at 17:58:34 Aug 10 2018
 */

#include "Easylog.h"

//
// layout: 
//	{process}
//	{thread}
//	{user}
//	{host}
//	{level}
//	{levelshort}
//	{file}
//	{line}
//	{function}
//	{msg}
//	{datetime:%y-%m-%d}"
//	{datetime:%y/%02m/%02d %02H:%02M:%02S}
//	{millisecond:3}
// example: 
//	easylog->set_layout(GLOBAL, "[{datetime:%y/%02m/%02d %02H:%02M:%02S}|{millisecond:3}] {msg}");
//	[18/08/11 06:24:24|206] Rank capacity: 20, package: 600000
//

//
// benchmark:
//	1 million EasylogMessage object constructor and ostringstream waste: 546 ms
//	1 million log_message but don't write to file and stdout waste: 1173 ms
//	1 million log_message and write to file stream and not flush, not stdout, waste: 1231 ms
//	1 million log_message to file and flush right now, waste: 2397 ms
//	1 million timestamp function waste: 1721 ms
//	1 million currentSecond function waste: 4 ms
//	1 million currentMillisecond function waste: 38 ms
//	1 million gettimeofday function waste: 19 ms
//	1 million gmtime to timestamp function waste: 138 ms
//

namespace logger {

#define SafeFree(P)		do { if(P) { ::free((void*)P); (P) = nullptr; } } while(0)
#define SafeDelete(P)	do { if(P) { delete (P); (P) = nullptr; } } while(0)

	const char* level_string(EasylogSeverityLevel level) {
		static const char* __level_string[] = {
			[GLOBAL]			= 	"GLOBAL",
			[LEVEL_DEBUG]		=	"DEBUG",
			[LEVEL_TRACE]		=	"TRACE",
			[LEVEL_ALARM]		=	"ALARM",
			[LEVEL_ERROR]		=	"ERROR",
			[LEVEL_PANIC]		=	"PANIC",
			[LEVEL_SYSTEM] 		=	"SYSTEM",
		};
		return __level_string[level];
	}

	const char* levelshort_string(EasylogSeverityLevel level) {
		static const char* __level_string[] = {
			[GLOBAL]			= 	"G",
			[LEVEL_DEBUG]		=	"D",	
			[LEVEL_TRACE]		=	"T",	
			[LEVEL_ALARM]		=	"ALM",
			[LEVEL_ERROR]		=	"ERR",
			[LEVEL_PANIC]		=	"PANIC",
			[LEVEL_SYSTEM] 		=	"SYS",
		};
		return __level_string[level];
	}

	// cost of executing 1 million times is:
	//		c++ 11 waste: 1721 ms
	//		gettimeofday waste: 138 ms
	//
	// get current timestamp
	//	if time_format is nullptr, default value is "%y/%02m/%02d %02H:%02M:%02S", like: 18/06/29 15:04:18
	const char* timestamp(char* buffer, size_t len, struct timeval * tv, const char* time_format) {
		struct timeval tv_now = { tv_sec: 0, tv_usec: 0 };
		if (tv == nullptr) {
			gettimeofday(&tv_now, nullptr);
			tv = &tv_now;
		}		
	
		if (time_format == nullptr) {
			time_format = "%y/%02m/%02d %02H:%02M:%02S"; // 18/06/29 15:04:18
		}

#if EASYLOG_CONVERT_CST_TIME
		// utc -> cst
		tv->tv_sec += 8 * 3600;
#endif

		struct tm result;
		gmtime_r(&tv->tv_sec, &result);

		std::strftime(buffer, len, time_format, &result);

		return (const char *) buffer;
	}

	const char* getCurrentDirectory() {
		static char __dir_buffer[PATH_MAX];
		//#if defined(_GNU_SOURCE)
#if false
		// absolute path name, like: /home/hushouguo/libtnode/tests 
		const char* s = get_current_dir_name();			
		strncpy(__dir_buffer, s, sizeof(__dir_buffer));
		SafeFree(s);
		return __dir_buffer;
#else
		// absolute path name, like: /home/hushouguo/libtnode/tests
		return ::getcwd(__dir_buffer, sizeof(__dir_buffer));
#endif
	}
	
#if EASYLOG_HAS_LOG_LAYOUT
	struct EasylogLayoutNode {
		std::string plainstring;
		int arg = -1;
		std::function<void(EasylogLayoutNode*, EasylogMessage*, std::ostream&)> dynamicstring = nullptr;
		EasylogLayoutNode(const char* s) : plainstring(s) {}
		EasylogLayoutNode(const char* s, size_t n) : plainstring(s, n) {}
		EasylogLayoutNode(std::string s) : plainstring(s) {}
		EasylogLayoutNode(EasylogLayoutNode* layoutNode) {
			this->plainstring = layoutNode->plainstring;
			this->arg = layoutNode->arg;
			this->dynamicstring = layoutNode->dynamicstring;
		}
		EasylogLayoutNode(std::function<void(EasylogLayoutNode*, EasylogMessage*, std::ostream&)> func) : dynamicstring(func) {}
	};
#endif

	struct EasylogLevelNode {
		EasylogSeverityLevel level;
		EasylogColor color;
		bool to_stdout;
		std::string filename, fullname;
		std::ofstream* fs;
		uint64_t fs_launchtime;
#if EASYLOG_HAS_LOG_LAYOUT				
		std::list<EasylogLayoutNode*> layouts_prefix;
		std::list<EasylogLayoutNode*> layouts_postfix;
#endif				
	};

	struct EasylogNode {
		EasylogLevelNode* levelNode;
		std::stringbuf buffer;
		EasylogNode* next;
		EasylogNode() : levelNode(nullptr), next(nullptr) {}
	};


	EasylogMessage::EasylogMessage(Easylog* easylog, EasylogSeverityLevel level, std::string file, int line, std::string func)
		: std::ostream(nullptr)
		, _easylog(easylog)
		, _level(level)
#if EASYLOG_HAS_LOG_LAYOUT		
		, _file(file)
		, _line(line)
		, _function(func)
#endif
		, _log(new EasylogNode())
	{
		rdbuf(&this->_log->buffer);

#if EASYLOG_HAS_LOG_LAYOUT
		const std::list<EasylogLayoutNode*>& layouts = this->_easylog->layout_prefix(this->level());
		for (auto& layoutNode : layouts) {
			if (layoutNode->dynamicstring != nullptr) {
				layoutNode->dynamicstring(layoutNode, this, *this);
			}
			else {
				*this << layoutNode->plainstring;
			}
		}
#else

#if EASYLOG_ENABLE_PREFIX_DATETIME
		struct timeval tv = { tv_sec: 0, tv_usec: 0 };
		gettimeofday(&tv, nullptr);

		char time_buffer[64];
		
#if EASYLOG_ENABLE_PREFIX_DATETIME_LONG
		timestamp(time_buffer, sizeof(time_buffer), &tv, "[%y/%02m/%02d %02H:%02M:%02S");
#else
		timestamp(time_buffer, sizeof(time_buffer), &tv, "[%02H:%02M:%02S");
			//easylog->autosplit_hour() ? "[%02M:%02S" : "[%02H:%02M:%02S");
#endif

		*this << time_buffer;

#if EASYLOG_ENABLE_PREFIX_DATETIME_MILLISECOND
		*this << "|" << std::setw(3) << std::setfill('0') << (tv.tv_usec / 1000) << "] ";
#else
		*this << "] ";
#endif

#endif

#if EASYLOG_ENABLE_PREFIX_LEVEL
		if (level != LEVEL_TRACE) {
			*this << "<" << levelshort_string(level) << "> ";
		}
#endif

#if EASYLOG_ENABLE_PREFIX_ERROR_FILE
		if (level == LEVEL_ERROR || level == LEVEL_PANIC) {
			*this << "(" << file << ":" << line << ") ";
		}
#endif

#endif
	}

	EasylogMessage::~EasylogMessage() {
#if EASYLOG_HAS_LOG_LAYOUT
		const std::list<EasylogLayoutNode*>& layouts = this->_easylog->layout_postfix(this->level());
		for (auto& layoutNode : layouts) {
			if (layoutNode->dynamicstring != nullptr) {
				layoutNode->dynamicstring(layoutNode, this, *this);
			}
			else {
				*this << layoutNode->plainstring;
			}
		}
#endif	
		this->flush();
	}

	void EasylogMessage::flush() {
		//if (this->tellp() > 0) {
		if (this->rdbuf()->in_avail()) {
			*this << "\n";
			this->_easylog->log_message(this);
		}
	}

	void EasylogMessage::cout(const char* format, ...) {
		char log[65536];
		va_list va;
		va_start(va, format);
		int len = vsnprintf(log, sizeof(log), format, va);
		va_end(va);
		if (len > 0) {
			*this << log;
		}
		else {
			fprintf(stderr, "cout error:%d,%s", errno, strerror(errno));
		}
	}
	////////////////////////////////////////////////////////////////////////////////////

	class Spinlocker {
		public:
			void lock() {
				while (this->_locker.test_and_set(std::memory_order_acquire));
			}

			bool trylock() {
				return !this->_locker.test_and_set(std::memory_order_acquire);// set OK, return false
			}

			void unlock() {
				this->_locker.clear(std::memory_order_release);
			}
		private:
			std::atomic_flag _locker = ATOMIC_FLAG_INIT;
	};

	////////////////////////////////////////////////////////////////////////////////////

	class EasylogInternal : public Easylog {
		public:
			EasylogInternal();
			~EasylogInternal();

		public:
			EasylogSeverityLevel level() override { return this->_level; }
			void set_level(EasylogSeverityLevel level) override { this->_level = level; }
			EasylogColor color(EasylogSeverityLevel level) override { return this->_levels[level].color; }
			void set_color(EasylogSeverityLevel level, EasylogColor color) override;
			bool set_destination(std::string dir) override;
			void set_tostdout(EasylogSeverityLevel level, bool enable) override;
			void set_toserver(EasylogSeverityLevel level, std::string address, int port) override;
			void set_tofile(EasylogSeverityLevel level, std::string filename) override;
			bool autosplit_day() override { return this->_autosplit_day; }
			bool autosplit_hour() override { return this->_autosplit_hour; }
			void set_autosplit_day(bool value) override { this->_autosplit_day = value; }
			void set_autosplit_hour(bool value) override { this->_autosplit_hour = value; }
#if EASYLOG_HAS_LOG_LAYOUT			
			bool set_layout(EasylogSeverityLevel level, std::string layout) override;
			const std::list<EasylogLayoutNode*>& layout_prefix(EasylogSeverityLevel level) override { return this->_levels[level].layouts_prefix; }
			const std::list<EasylogLayoutNode*>& layout_postfix(EasylogSeverityLevel level) override { return this->_levels[level].layouts_postfix; }
#endif
			const char* destination() override { return this->_dest_dir.c_str(); }
			const char* current_log_filename(EasylogSeverityLevel level) override;

			inline bool isstop() { return this->_stop; }
			void stop() override;

		public:
			void log_message(EasylogMessage* easylogMessage) override;

		private:
			bool _stop = false;
#if defined(DEBUG) || defined(_DEBUG)
			EasylogSeverityLevel _level = LEVEL_DEBUG;
#else
			EasylogSeverityLevel _level = LEVEL_TRACE;
#endif
			std::string _dest_dir = getCurrentDirectory();
			bool _autosplit_day = true, _autosplit_hour = false;
			void full_filename(const std::string& filename, std::string& fullname);

		private:

			void openfile(EasylogLevelNode* levelNode);
			void autosplit_file(EasylogLevelNode* levelNode);
			
			void send_to_stdout(EasylogLevelNode* levelNode, const std::string& s);
			void send_to_file(EasylogLevelNode* levelNode, const std::string& s);
			void send_to_network(EasylogLevelNode* levelNode, const std::string& s);

			// unordered_map MUST gcc version is above 7
			//std::unordered_map<EasylogSeverityLevel, EasylogLevelNode> _levels = {
			std::map<EasylogSeverityLevel, EasylogLevelNode> _levels = {
				{ LEVEL_DEBUG, 
{ level:LEVEL_DEBUG, color:CYAN, to_stdout:true, filename:"", fullname:"", fs:nullptr, fs_launchtime:0
#if EASYLOG_HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},

				{ LEVEL_TRACE, 
{ level:LEVEL_TRACE, color:GREY, to_stdout:true, filename:"", fullname:"", fs:nullptr, fs_launchtime:0
#if EASYLOG_HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_ALARM, 
{ level:LEVEL_ALARM, color:YELLOW, to_stdout:true, filename:"", fullname:"", fs:nullptr, fs_launchtime:0
#if EASYLOG_HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_ERROR, 
{ level:LEVEL_ERROR, color:LRED, to_stdout:true, filename:"", fullname:"", fs:nullptr, fs_launchtime:0
#if EASYLOG_HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_PANIC, 
{ level:LEVEL_PANIC, color:LMAGENTA, to_stdout:true, filename:"", fullname:"", fs:nullptr, fs_launchtime:0
#if EASYLOG_HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_SYSTEM, 
{ level:LEVEL_SYSTEM, color:LCYAN, to_stdout:true, filename:"", fullname:"", fs:nullptr, fs_launchtime:0
#if EASYLOG_HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
			};

		private:
#if EASYLOG_ENABLE_ASYNC_SEND
			Spinlocker _locker;
			//std::list<EasylogNode*> _logQueue;
			EasylogNode *_headQueue = nullptr, *_tailQueue = nullptr;
			inline void pushNode(EasylogNode* node) {
				this->_locker.lock();
				if (!this->_headQueue) {
					assert(this->_tailQueue == nullptr);
					this->_headQueue = this->_tailQueue = node;
				}
				else {
					assert(this->_tailQueue != nullptr);
					this->_tailQueue->next = node;
					this->_tailQueue = node;
				}
				this->_locker.unlock();
			}
			inline EasylogNode* popNode() {
				this->_locker.lock();
				EasylogNode* node = this->_headQueue;
				if (this->_headQueue != nullptr) {
					this->_headQueue = this->_headQueue->next;
					if (this->_headQueue == nullptr) {
						assert(this->_tailQueue == node);
						this->_tailQueue = nullptr;
					}
				}
				this->_locker.unlock();
				return node;
			}
			
			std::mutex _logMutex;
			std::condition_variable _logCondition;
			std::thread* _logthread = nullptr;
			void logProcess();
#endif			

#if EASYLOG_HAS_LOG_LAYOUT
		private:
			std::unordered_map<std::string, EasylogLayoutNode*> _initnodes = {
				{ "process", new EasylogLayoutNode(std::to_string(getpid())) },
				{ "thread", new EasylogLayoutNode(std::to_string((threadid()))) },
				{ "level", new EasylogLayoutNode([this](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << level_string(easylogMessage->level());
				}) },
				{ "levelshort", new EasylogLayoutNode([this](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << levelshort_string(easylogMessage->level());
				}) },
				{ "user", new EasylogLayoutNode(getlogin()) },
				{ "host", new EasylogLayoutNode(gethostname()) },
				{ "file", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << easylogMessage->file();
				}) },
				{ "line", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << easylogMessage->line();
				}) },
				{ "function", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << easylogMessage->function();
				}) },
				{ "msg", nullptr},
				{ "datetime", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					char time_buffer[64];
					timestamp(time_buffer, sizeof(time_buffer), sTime.secondPart(), layoutNode->plainstring.c_str());
					os << time_buffer;
				}) },
				{ "millisecond", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << std::setw(layoutNode->arg) << std::setfill('0') << sTime.millisecondPart();
				}) }
			};
#endif

	};

	void EasylogInternal::send_to_stdout(EasylogLevelNode* levelNode, const std::string& s) {
		if (levelNode->to_stdout) {
			if (levelNode->color != GREY) {
				std::cout << "\x1b[" 
						<< (levelNode->color >= LRED ? (levelNode->color - 10) : levelNode->color) 
						<< (levelNode->color >= LRED ? ";1" : "") << "m";
				std::cout.write(s.data(), s.length() - 1);
				std::cout << "\x1b[0m" << std::endl;
			}
			else {
				std::cout.write(s.data(), s.length());
			}
		}
	}
	
	void EasylogInternal::send_to_file(EasylogLevelNode* levelNode, const std::string& s) {
		if (levelNode->fs != nullptr) {
			this->autosplit_file(levelNode);
			levelNode->fs->write(s.data(), s.length());
			levelNode->fs->flush();
		}	
	}
	
	void EasylogInternal::send_to_network(EasylogLevelNode* levelNode, const std::string& s) {
	}

	void EasylogInternal::log_message(EasylogMessage* easylogMessage) {
		if (!this->isstop() && easylogMessage->level() >= this->level()) {
			EasylogLevelNode* levelNode = &this->_levels[easylogMessage->level()];
			
#if EASYLOG_ENABLE_ASYNC_SEND
			easylogMessage->log()->levelNode = levelNode;
			///this->_locker.lock();
			///this->_logQueue.push_back(easylogMessage->log());
			///this->_locker.unlock();
			this->pushNode(easylogMessage->log());
			this->_logCondition.notify_all();
#else
			const std::string& s = easylogMessage->log()->buffer.str();
			this->send_to_stdout(levelNode, s);
			this->send_to_file(levelNode, s);
			this->send_to_network(levelNode, s);
			delete easylogMessage->log();
#endif
			if (easylogMessage->level() == LEVEL_PANIC) {
				this->stop();
				::abort();
			}
		}
		else {
#if 0			
			fprintf(stderr, "isstop: %s, easylogMessage->level: %d, this->level: %d, log: %s\n",
					this->isstop() ? "true" : "false",
					easylogMessage->level(),
					this->level(),
					easylogMessage->log()->buffer.str().c_str());
#endif			
		}
	}

#if EASYLOG_ENABLE_ASYNC_SEND
	void EasylogInternal::logProcess() {
		while (true) {
			EasylogNode* logNode = nullptr;
			///this->_locker.lock();
			///if (!this->_logQueue.empty()) {
			///	logNode = this->_logQueue.front();
			///	this->_logQueue.pop_front();
			///}
			///this->_locker.unlock();
			logNode = this->popNode();
			
			if (logNode) {
				const std::string& s = logNode->buffer.str();
				this->send_to_stdout(logNode->levelNode, s);
				this->send_to_file(logNode->levelNode, s);
				this->send_to_network(logNode->levelNode, s);
				SafeDelete(logNode);
			}
			else {
				if (this->isstop()) { break; }
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
				std::unique_lock<std::mutex> locker(this->_logMutex);
				this->_logCondition.wait(locker);
			}
		}
		//fprintf(stderr, "Easylog exit, logQueue: %ld\n", this->_logQueue.size());
		fprintf(stderr, "Easylog exit, logQueue: %s\n", this->_headQueue == nullptr ? "empty" : "uncleanup");
	}
#endif

#if EASYLOG_HAS_LOG_LAYOUT
	bool EasylogInternal::set_layout(EasylogSeverityLevel level, std::string layout) {
		auto parsefunc = [this](std::list<EasylogLayoutNode*>& layouts_prefix, std::list<EasylogLayoutNode*>& layouts_postfix, const std::string& layout) -> bool {
			std::string::size_type i = 0;
			bool msgNode = false;
			while (i < layout.length()) {
				std::string::size_type head = layout.find('{', i);
				if (head == std::string::npos) {
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(std::string(layout, i));
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
					return true;
				}

				std::string::size_type tail = layout.find('}', head);
				if (tail == std::string::npos) {
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(std::string(layout, i));
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
					return true;
				}

				if (head != i) {
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(std::string(layout, i, head - i));
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
				}

				std::string token, arg;				
				std::string::size_type colon = layout.find(':', head);
				if (colon == std::string::npos || colon > tail) {
					token.assign(layout, head + 1, tail - head - 1);
					arg.clear();
				}
				else {
					token.assign(layout, head + 1, colon - head - 1);
					arg.assign(layout, colon + 1, tail - colon - 1);
				}

				if (this->_initnodes.find(token) == this->_initnodes.end()) {
					fprintf(stderr, "illegal token: %s", token.c_str());
					return false;
				}

				if (this->_initnodes[token] == nullptr) {
					CHECK_RETURN(msgNode == false, false, "there is only unique {msg}");
					msgNode = true;
				}
				else {				
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(this->_initnodes[token]);
					if (!arg.empty()) {
						layoutNode->plainstring = arg;
						if (isdigit(layoutNode->plainstring)) {
							layoutNode->arg = atoi(layoutNode->plainstring.c_str());
						}
					}
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
				}

				i = tail + 1;
			}
			
			return true;
		};

		auto clearfunc = [](std::list<EasylogLayoutNode*>& layouts) {
			for (auto& layoutNode : layouts) {
				SafeDelete(layoutNode);
			}
			layouts.clear();
		};

		bool result = false;
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				clearfunc(levelNode.layouts_prefix);
				clearfunc(levelNode.layouts_postfix);
				result = parsefunc(levelNode.layouts_prefix, levelNode.layouts_postfix, layout);
				if (!result) {
					clearfunc(levelNode.layouts_prefix);
					clearfunc(levelNode.layouts_postfix);
					break;
				}
			}
		}
		
		return result;
	}
#endif

	//
	// get current time seconds	
	uint64_t currentSecond() {		
		return std::time(nullptr); // cost of executing 1 million times is: 4 ms
	}

	const char* getDirectoryName(const char* fullname) {
		static char __dir_buffer[PATH_MAX];
		strncpy(__dir_buffer, fullname, sizeof(__dir_buffer));
		return dirname(__dir_buffer);
	}

	const char* getFilename(const char* fullname) {
		static char __filename_buffer[PATH_MAX];
		strncpy(__filename_buffer, fullname, sizeof(__filename_buffer));
		return basename(__filename_buffer);
	}

	const char* absoluteDirectory(const char* fullname) {
		static char __dir_buffer[PATH_MAX];		
		char* realdir = realpath(getDirectoryName(fullname), nullptr);
		snprintf(__dir_buffer, sizeof(__dir_buffer), "%s/%s", realdir, getFilename(fullname));
		SafeFree(realdir);
		return __dir_buffer;
	}

	//
	// test for the file is a directory
	bool isDir(const char* file) {
		struct stat buf;
		if (stat(file, &buf) != 0) { return false; }
		return S_ISDIR(buf.st_mode);
	}

	//
	// existDir: 
	//	test for the existence of the file
	// accessableDir, readableDir, writableDir:
	// 	test whether the file exists and grants read, write, and execute permissions, respectively.
	bool existDir(const char* file) {
		return access(file, F_OK) == 0;
	}
	bool readableDir(const char* file) {
		return access(file, R_OK) == 0;
	}
	bool writableDir(const char* file) {
		return access(file, W_OK) == 0;
	}
	bool accessableDir(const char* file) {
		return access(file, X_OK) == 0;
	}

	//
	// create inexistence folder
	bool createDirectory(const char* path) {
		std::string fullPath = absoluteDirectory(path);
		std::string::size_type i = 0;
		umask(0);
		while (i < fullPath.length()) {
			std::string::size_type head = fullPath.find('/', i);
			std::string dir;
			dir.assign(fullPath, 0, head == std::string::npos ? fullPath.length() : head);
			if (!dir.empty()) {
				int rc = mkdir(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
				//CHECK_RETURN(rc == 0 || errno == EEXIST, false, "mkdir:%s error:%d,%s", dir.c_str(), errno, strerror(errno));
				if (rc != 0 && errno != EEXIST) {
					fprintf(stderr, "mkdir:%s error:%d,%s", dir.c_str(), errno, strerror(errno));
					return false;
				}
			}
			if (head == std::string::npos) {
				break;
			}
			i = head + 1;
		}
		return true;
	}

	bool EasylogInternal::set_destination(std::string dir) {
		const char* realdir = absoluteDirectory(dir.c_str());
		CHECK_RETURN(realdir, false, "dir: `%s` is not valid directory", dir.c_str());
		if (!existDir(realdir) && !createDirectory(realdir)) {
			return false;
		}
		CHECK_RETURN(existDir(realdir), false, "dir: `%s` not existence", realdir);
		CHECK_RETURN(isDir(realdir), false, "`%s` not directory", realdir);
		CHECK_RETURN(accessableDir(realdir), false, "dir: `%s` not accessible", realdir);
		CHECK_RETURN(writableDir(realdir), false, "dir: `%s` not writable", realdir);
		this->_dest_dir = realdir;
		return true;
	}

	void EasylogInternal::full_filename(const std::string& filename, std::string& fullname) {
		fullname = this->_dest_dir + "/" + filename;
		if (this->_autosplit_hour) {
			char time_buffer[64];
			timestamp(time_buffer, sizeof(time_buffer), nullptr, ".%Y-%02m-%02d.%02H");
			fullname += time_buffer;
		}
		else if (this->_autosplit_day) {
			char time_buffer[64];
			timestamp(time_buffer, sizeof(time_buffer), nullptr, ".%Y-%02m-%02d");
			fullname += time_buffer;
		}
	}
	
	void EasylogInternal::openfile(EasylogLevelNode* levelNode) {
		if (levelNode->fs != nullptr) {
			levelNode->fs->close();
			SafeDelete(levelNode->fs);
		}		
		this->full_filename(levelNode->filename, levelNode->fullname);
		try {
			levelNode->fs = new std::ofstream(levelNode->fullname, std::ios::app|std::ios::out);
		} catch (std::exception& e) {
			fprintf(stderr, "ofstream exception: %s, filename: %s\n", e.what(), levelNode->fullname.c_str());
			SafeDelete(levelNode->fs);
		}
		levelNode->fs_launchtime = currentSecond();	
	}

	void EasylogInternal::autosplit_file(EasylogLevelNode* levelNode) {
		if (this->_autosplit_day || this->_autosplit_hour) {
			uint64_t nowtime = currentSecond();
			struct tm tm_nowtime, tm_launchtime;
			gmtime_r((const time_t *) &nowtime, &tm_nowtime);
			gmtime_r((const time_t *) &levelNode->fs_launchtime, &tm_launchtime);
			if ((this->_autosplit_day && tm_nowtime.tm_mday != tm_launchtime.tm_mday) 
				|| (this->_autosplit_hour && tm_nowtime.tm_hour != tm_launchtime.tm_hour)){
				this->openfile(levelNode);
			}
		}
	}

	void EasylogInternal::set_tostdout(EasylogSeverityLevel level, bool enable) {
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				levelNode.to_stdout = enable;
			}
		}
	}
	
	void EasylogInternal::set_tofile(EasylogSeverityLevel level, std::string filename) {
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				levelNode.filename = filename;
			}
		}

		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.filename.length() > 0 && levelNode.fs == nullptr) {
				this->openfile(&levelNode);
			}
		}
	}

	const char* EasylogInternal::current_log_filename(EasylogSeverityLevel level) {
		EasylogLevelNode* levelNode = &this->_levels[level];
		return levelNode->fullname.c_str();
	}

	void EasylogInternal::set_toserver(EasylogSeverityLevel level, std::string address, int port) {
		//TODO: toserver by MessageQueue
	}
			
	void EasylogInternal::set_color(EasylogSeverityLevel level, EasylogColor color) {
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				levelNode.color = color;
			}
		}
	}

	void EasylogInternal::stop() {
		this->_stop = true;
#if EASYLOG_ENABLE_ASYNC_SEND		
		this->_logCondition.notify_all();
		if (this->_logthread && this->_logthread->joinable()) {
			this->_logthread->join();
		}
		SafeDelete(this->_logthread);
#endif
	}
		
	
	EasylogInternal::EasylogInternal() {
#if EASYLOG_ENABLE_ASYNC_SEND	
		SafeDelete(this->_logthread);
		this->_logthread = new std::thread([this]() {
			this->logProcess();
		});
#endif		
	}

	Easylog::~Easylog() {}
	EasylogInternal::~EasylogInternal() {
#if EASYLOG_HAS_LOG_LAYOUT
		for (auto& i : this->_initnodes) {
			SafeDelete(i.second);
		}
		this->_initnodes.clear();
#endif		

		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.fs != nullptr) {
				levelNode.fs->close();
				SafeDelete(levelNode.fs);
			}

#if EASYLOG_HAS_LOG_LAYOUT
			for (auto& layoutNode : levelNode.layouts_prefix) {
				SafeDelete(layoutNode);
			}
			levelNode.layouts_prefix.clear();
			
			for (auto& layoutNode : levelNode.layouts_postfix) {
				SafeDelete(layoutNode);
			}
			levelNode.layouts_postfix.clear();
#endif			
		}
	}

	Easylog* EasylogCreator::create() {
		return new EasylogInternal();
	}

	Easylog* Easylog::syslog() {
		static Easylog* __syslog = EasylogCreator::create();
		return __syslog;
	}
}

