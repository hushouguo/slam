/*
 * \file: base64.h
 * \brief: Created by hushouguo at Fri 21 Jul 2017 06:14:20 AM CST
 */
 
#ifndef __BASE64_H__
#define __BASE64_H__

BEGIN_NAMESPACE_TNODE {
	void base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len, std::string& ret_string);
	void base64_decode(std::string const& encoded_string, std::string& ret_string);
}

#endif
