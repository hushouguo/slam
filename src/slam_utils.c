/*
 * \file: slam_utils.c
 * \brief: Created by hushouguo at 16:51:19 Jul 25 2019
 */

#include "slam.h"

#define CONVERT_CST_TIME    1
#define SYS_ERRNO	        128

	static char** __errlist = nullptr;

	__attribute__((constructor)) static void __strerror_init() {
		if (!__errlist) {
			__errlist = (char **) slam_malloc(SYS_ERRNO * sizeof(char*));
		}
		int err;
		for (err = 0; err < SYS_ERRNO; ++err) {
			__errlist[err] = strdup(strerror(err));
		}
	}

	__attribute__((destructor)) static void __strerror_destroy() {
		if (__errlist) {
		    int err;
			for (err = 0; err < SYS_ERRNO; ++err) {
				slam_free(__errlist[err]);
			}
			slam_free(__errlist);
		}
	}

//
// like ::strerror
const char* errstring(int err) {
    return err >= 0 && err < SYS_ERRNO ? __errlist[err] : strerror(err);
}

uint32_t slam_cpus() {
	long n = sysconf(_SC_NPROCESSORS_CONF);
	return n > 0 ? n : 1;
}

/* check that a floating point number is integer */
bool slam_is_integer(double value) {
	return value == (int64_t) value;
}

//
// get current time seconds 
uint64_t slam_seconds() {       
    return time(nullptr); // cost of executing 1 million times is: 4 ms
}


//   
// get current time milliseconds    
uint64_t slam_milliseconds() {
    // cost of executing 1 million times is:
    //      c++ 11 waste: 38 ms
    //      gettimeofday waste: 19 ms
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// cost of executing 1 million times is:
//		c++ 11 waste: 1721 ms
//		gettimeofday waste: 138 ms
//
// current timestamp
// if time_format is nullptr, default value is "%y/%02m/%02d %02H:%02M:%02S", like: 18/06/29 15:04:18
const char* slam_timestamp(char* buf, size_t bufsize, uint64_t seconds, const char* time_format) {
	struct timeval tv = { tv_sec: (time_t) seconds, tv_usec: 0 };
	if (tv.tv_sec == 0) {
		gettimeofday(&tv, nullptr);
	}

	if (!time_format) {
		time_format = "%y/%02m/%02d %02H:%02M:%02S"; // 18/06/29 15:04:18
	}

#if CONVERT_CST_TIME
	// utc -> cst
	tv.tv_sec += 8 * 3600;
#endif

	struct tm result;
	gmtime_r(&tv.tv_sec, &result);

	strftime(buf, bufsize, time_format, &result);

	return (const char *) buf;
}


//
// Sat, 11 Mar 2017 21:49:51 GMT
const char* slam_timestamp_gmt(char* buf, size_t bufsize, uint64_t seconds) {
	struct timeval tv = { tv_sec: (time_t) seconds, tv_usec: 0 };
	if (tv.tv_sec == 0) {
		gettimeofday(&tv, nullptr);
	}

	// Sat, 11 Mar 2017 21:49:51 GMT
	const char* time_format = "%a, %d %b %Y %H:%M:%S GMT";

#if CONVERT_CST_TIME
	// utc -> cst
	tv.tv_sec += 8 * 3600;
#endif

	struct tm result;
	gmtime_r(&tv.tv_sec, &result);

	strftime(buf, bufsize, time_format, &result);

	return (const char *) buf;
}


//
// hash string
uint32_t slam_hash_string(const char* s) {
	uint32_t h = 0, g;
	const char* end = s + strlen(s);
	while (s < end) {
		h = (h << 4) + *s++;
		if ((g = (h & 0xF0000000))) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	return h;
}


//
// test for the file is a directory
bool slam_is_dir(const char* file) {
    struct stat buf;
    if (stat(file, &buf) != 0) { return false; }
    return S_ISDIR(buf.st_mode);
}

//
// existDir: 
//     test for the existence of the file
// accessableDir, readableDir, writableDir:
//     test whether the file exists and grants read, write, and execute permissions, respectively.
bool slam_file_exist(const char* file) {
    return access(file, F_OK) == 0;
}
bool slam_file_readable(const char* file) {
    return access(file, R_OK) == 0;
}
bool slam_file_writable(const char* file) {
    return access(file, W_OK) == 0;
}
bool slam_file_accessable(const char* file) {
    return access(file, X_OK) == 0;
}

//
// get existence file size
uint64_t slam_file_size(const char* filename) {
    struct stat buf;
    if (stat(filename, &buf) != 0) { return 0; }
    return buf.st_size;
}

void slam_set_process_title(int argc, char* argv[], const char* title) {
	//if (__argv.empty()) {
	//	for (int i = 0; i < argc; ++i) {
	//		__argv.push_back(argv[i]);
	//	}
	//}
	int i;
	for (i = 1; i < argc; ++i) {
		memset(argv[i], 0, strlen(argv[i]));
	}
	strcat(argv[0], title);
}

#if 0
//
// get the execution of the program, like: foo
const char* slam_program_name() {
#if defined(__APPLE__) || defined(__FreeBSD__)
	return getprogname();	// need libbsd
#elif defined(_GNU_SOURCE)
	extern char* program_invocation_name;			// like: ./bin/routine
	extern char* program_invocation_short_name;	    // like: routine
	return program_invocation_short_name;
#else
	extern char *__progname;						// routine:  defined by the libc
	return __progname;
#endif
}

//
// get the complete execution of the program, like: ./bin/foo
const char* getProgramFullName() {
#if defined(_GNU_SOURCE)
		//extern char* program_invocation_name; 		// like: ./bin/routine
		//extern char* program_invocation_short_name;	// like: routine
		return ::program_invocation_name;
#else
		return ::getenv("_");
#endif
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

	//
	// get extension of filename
	const char* getFilenameExtension(const char* filename) {
		static char __filename_buffer[PATH_MAX];
		strncpy(__filename_buffer, filename, sizeof(__filename_buffer));
		char* p = strchr(__filename_buffer, '.');
		if (p) {
			*p++ = '\0';
		}
		return p;
	}

	//
	// get prefix of filename
	const char* getFilenamePrefix(const char* filename) {
		static char __filename_buffer[PATH_MAX];
		strncpy(__filename_buffer, filename, sizeof(__filename_buffer));
		char* p = strchr(__filename_buffer, '.');
		if (p != nullptr) {
			*p++ = '\0';
		}
		return __filename_buffer;
	}

	const char* absoluteDirectory(const char* fullname) {
		static char __dir_buffer[PATH_MAX];		
		char* realdir = realpath(getDirectoryName(fullname), nullptr);
		snprintf(__dir_buffer, sizeof(__dir_buffer), "%s/%s", realdir, getFilename(fullname));
		SafeFree(realdir);
		return __dir_buffer;
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
            CHECK_RETURN(rc == 0 || errno == EEXIST, false, "mkdir:%s error:%d,%s", dir.c_str(), errno, strerror(errno));
        }
        if (head == std::string::npos) {
            break;
        }
        i = head + 1;
    }
    return true;
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




void DumpLibraryVersion() {

    //
    // Config information
    //
    if (sConfig.confile.empty()) {
        Alarm << "specify config file: Unspecified";
    }
    else {
        Trace << "specify config file: " << sConfig.confile;
    }
    sConfig.dump();

    //Trace << std::setw(24) << std::setfill(' ') << i.first << ": " << i.second;
    
    //
    // output 3rd libraries
    //
    Trace("all 3rd libraries:");
    
#ifdef TC_VERSION_STRING		
            Trace("    tcmalloc: %d.%d%s", TC_VERSION_MAJOR, TC_VERSION_MINOR, TC_VERSION_PATCH);
#else
            Trace("    not link tcmalloc");
#endif
    
#ifdef LIBEVENT_VERSION
            Trace("    libevent: %s", LIBEVENT_VERSION);
#endif
    
#ifdef ZMQ_VERSION_MAJOR
            Trace("    libzmq: %d.%d.%d", ZMQ_VERSION_MAJOR, ZMQ_VERSION_MINOR, ZMQ_VERSION_PATCH);
#endif
    
#ifdef LUAJIT_VERSION
            Trace("    luaJIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);
#endif
    
#ifdef GOOGLE_PROTOBUF_VERSION
            Trace("    protobuf: %d, library: %d", GOOGLE_PROTOBUF_VERSION, GOOGLE_PROTOBUF_MIN_LIBRARY_VERSION);
#endif
    
            Trace("    rapidxml: 1.13");
    
#ifdef MYSQL_SERVER_VERSION		
            Trace("    mysql: %s", MYSQL_SERVER_VERSION);
#endif
    
            Trace("    gcc version: %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
        }


	//
	// check that a string is utf8 encoding
	bool isUTF8String(const std::string& string) {
		int c,i,ix,n,j;
		for (i = 0, ix = string.length(); i < ix; i++) {
			c = (unsigned char) string[i];
			//if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
			if (0x00 <= c && c <= 0x7f) { n=0; }	// 0bbbbbbb
			else if ((c & 0xE0) == 0xC0) { n=1; }	// 110bbbbb
			else if ( c==0xed && i<(ix-1) && ((unsigned char)string[i+1] & 0xa0)==0xa0) { return false; }	//U+d800 to U+dfff
			else if ((c & 0xF0) == 0xE0) { n=2; }	// 1110bbbb
			else if ((c & 0xF8) == 0xF0) { n=3; }	// 11110bbb
			//else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
			//else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
			else { return false; }
			for (j=0; j<n && i<ix; j++) 
			{ // n bytes matching 10bbbbbb follow ?
				if ((++i == ix) || (( (unsigned char)string[i] & 0xC0) != 0x80))
				{
					return false;
				}
			}
		}
		return true;
	}


	//
	// allocate new buffer and copy buffer to new buffer, like: strdup
	void* memdup(void* buffer, size_t size) {
		void* newbuffer = ::malloc(size);
		memcpy(newbuffer, buffer, size);
		return newbuffer;
	}

	static std::vector<std::string> __argv;


	//
	// setup/reset process title
	void setProcesstitle(int argc, char* argv[], const char* title) {
		if (__argv.empty()) {
			for (int i = 0; i < argc; ++i) {
				__argv.push_back(argv[i]);
			}
		}
		for (int i = 1; i < argc; ++i) {
			memset(argv[i], 0, strlen(argv[i]));
		}
		strcat(argv[0], title);
	}

	void resetProcesstitle(int argc, char* argv[]) {
		for (size_t i = 0; i < __argv.size(); ++i) {
			strcpy(argv[i], __argv[i].c_str());
		}
	}

	bool VerifyLimits() {
		//
		// limit
		//
		size_t stack_size = sConfig.get("limit.stack_size", 0u);
		if (stack_size > 0) {
			setStackSizeLimit(stack_size);
		}
	
		size_t max_files = sConfig.get("limit.max_files", 0u);
		if (max_files > 0) {
			setOpenFilesLimit(max_files);
		}
	
		Trace("stack size: %u (limit.stack_size), max files: %u (limit.max_files)", getStackSizeLimit(), getOpenFilesLimit());
	
		//
		// verify lua version
		CHECK_RETURN(sizeof(lua_Integer) == 8, false, "require right version for lua");
		CHECK_RETURN(sizeof(lua_Number) == 8, false, "require right version for lua");
	
		return true;
	}

#endif

//
// limits: stack_size, max_files


// limits, linux default stack size: 8M (soft), 4G (hard)
bool slam_set_limit_stack_size(uint32_t value) {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_STACK, &limit);
	CHECK_RETURN(rc == 0, false, "getrlimit error:%d,%s", errno, errstring(errno));
	limit.rlim_cur = value;
	rc = setrlimit(RLIMIT_STACK, &limit);
	CHECK_RETURN(rc == 0, false, "setrlimit error:%d,%s", errno, errstring(errno));
	return true;
}

uint32_t slam_get_limit_stack_size() {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_STACK, &limit);
	CHECK_RETURN(rc == 0, 0, "getrlimit error:%d,%s", errno, errstring(errno));
	return limit.rlim_cur;
}

// limits, linux default open files: 1024 (soft), 4096 (hard)
bool slam_set_limit_open_files(uint32_t value) {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_NOFILE, &limit);
	CHECK_RETURN(rc == 0, false, "getrlimit error:%d,%s", errno, errstring(errno));
	limit.rlim_cur = value;
	rc = setrlimit(RLIMIT_NOFILE, &limit);
	CHECK_RETURN(rc == 0, false, "setrlimit error:%d,%s", errno, errstring(errno));
	return true;
}

uint32_t slam_get_limit_open_files() {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_NOFILE, &limit);
	CHECK_RETURN(rc == 0, 0, "getrlimit error:%d,%s", errno, errstring(errno));
	return limit.rlim_cur;
}

void slam_mallinfo() {
#ifdef TC_VERSION_STRING
    struct mallinfo info = tc_mallinfo();
#else
    struct mallinfo info = mallinfo();
#endif
	Trace("dump memory allocate info");
    Trace("    arena:    %d", info.arena);
    Trace("    ordblks:  %d", info.ordblks);
    Trace("    smblks:   %d", info.smblks);
    Trace("    hblks:    %d", info.hblks);
    Trace("    hblkhd:   %d", info.hblkhd);
    Trace("    usmblks:  %d", info.usmblks);
    Trace("    fsmblks:  %d", info.fsmblks);
    Trace("    uordblks: %d", info.uordblks);
    Trace("    fordblks: %d", info.fordblks);
    Trace("    keepcost: %d", info.keepcost);
}

