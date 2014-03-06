#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/curlbuild.h>
#include <string>
#include <sstream>
#include <iostream>

#include "json/json.h"
#include "client.hpp"

using namespace std;
using namespace Btsync;

#define CHECK_BTSYNC_ERROR(res) \
if (res.isObject() && \
	((!res["error"].isNull() && res["error"] != 0) || \
	(!res["result"].isNull() && res["result"] != 0))) {\
	throw BtsyncException(res["message"].asString());\
}

Client::Client(const string& u, const string& p) : 
	req_base("http://localhost:8888/api?method="),
 	username(u), password(p)
{
	curl = curl_easy_init();
}

Client::~Client() {
	curl_easy_cleanup(curl);
}

Json::Value Client::get_folders() {
	return _parseJSON(_request("http://localhost:8888/api?method=get_folders"));
}

Json::Value Client::get_folders(const string& secret) {
	return _parseJSON(_request("http://localhost:8888/api?method=get_folders&secret=" + secret));
}

void Client::add_folder(const string& dir) {
	string req = req_base + "add_folder" + "&dir=" + dir;
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
}

void Client::add_folder(const string& dir, const string& secret) {
	string req = req_base + "add_folder" + "&dir=" + dir + "&secret=" + secret;
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
} 

void Client::remove_folder(const string& secret) {
	string req = req_base + "remove_folder" + "&secret=" + secret;
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
}

Json::Value Client::get_files(const string& secret) {
	string req = req_base + "get_files" + "&secret=" + secret;
	Json::Value files = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(files);
	return files;
}

Json::Value Client::get_files(const string& secret, const string& path) {
	string req = req_base + "get_files" + "&path=" + path + "&secret=" + secret;
	Json::Value files = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(files);
	return files;
}

Json::Value Client::get_folder_peers(const string& secret) {
	string req = req_base + "get_folder_peers" + "&secret=" + secret;
	Json::Value peers = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(peers);
	return peers;
}

Json::Value Client::set_file_preferences(const string& secret, 
	const string& path, const bool download) {

	string req = req_base + "set_file_prefs" 
		+ "&secret=" + secret 
		+ "&path=" + path
		+ "download" + (download ? "1" : "0");
	Json::Value file_prefs = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(file_prefs);
	return file_prefs;
}

pair<string, string> Client::get_secrets() {
	string req = req_base + "get_secrets";
	Json::Value res = this->_parseJSON(this->_request(req));
	CHECK_BTSYNC_ERROR(res);
	return make_pair(res["read_write"].asString(), res["read_only"].asString());
}

pair<string, string> Client::get_secrets(const string& secret) {
	string req = req_base + "get_secrets" + "&secret=" + secret;
	Json::Value res = this->_parseJSON(this->_request(req));
	CHECK_BTSYNC_ERROR(res);
	return make_pair(secret, res["read_only"].asString());
}

pair<string, SecretType> Client::get_folder_secret(const string& dir) {

	Json::Value folders = get_folders();
	for (int i = 0; i < folders.size(); ++i)
	{
		if (folders[i]["dir"] == dir) {
			SecretType t = folders[i]["type"] == "read_write" ? READ_WRITE : READ_ONLY;
			return make_pair(folders[i]["secret"].asString(), t);
		}
	}

	throw BtsyncException("Error: No such directory.");
}

pair<string, string> Client::get_folder_secrets(const string& dir) {

	pair<string, SecretType> s_type = get_folder_secret(dir);
	
	if (s_type.second == READ_WRITE)
		return get_secrets(s_type.first);

	return pair<string, string>(s_type.first, "");
}

Json::Value Client::get_folder_preferences(const string& secret) {
	string req = req_base + "get_folder_prefs" + "&secret=" + secret;
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return res;
}

// Json::Value Client::set_folder_preferences(const string& secret, const string& params) {
// 	return set_folder_preferences(secret, Json::Value(params));
// }

Json::Value Client::set_folder_preferences(const string& secret, const Json::Value& params) {
	
	string req = req_base + "set_folder_prefs" + "&secret=" + secret;
	for (Json::ValueIterator it = params.begin(); it != params.end(); ++it)
		req = req + "&" + it.key().asString() + "=" + (*it).asString();

	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return res;
}

vector<string> Client::get_folder_hosts(const string& secret) {
	string req = req_base + "get_folder_hosts" + "&secret=" + secret;
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);

	res = res["hosts"];
	// dirty
	vector<string> retval;
	for (int i = 0; i < res.size(); ++i)
		retval.push_back(res[i].asString());
	return retval;
}

vector<string> Client::set_folder_hosts(const string& secret, vector<string>& hosts) {
	string req = req_base + "set_folder_hosts" + "&secret=" + secret + "&hosts=";
	// better use copy
	for (vector<string>::iterator it = hosts.begin(); it != hosts.end(); ++it) {
		req += *it;
		if (it != hosts.end() - 1)
			req += ",";
	}

	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);

	res = res["hosts"];
	// dirty
	vector<string> retval;
	for (int i = 0; i < res.size(); ++i)
		retval.push_back(res[i].asString());
	return retval;
}

Json::Value Client::get_preferences() {
	string req = req_base + "get_prefs";
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return res;
}

Json::Value Client::set_preferences(const Json::Value& params) {
	string req = req_base + "set_prefs";

	for (Json::ValueIterator it = params.begin(); it != params.end(); it++) {
		req = req + "&" + it.key().asString() + "=" + (*it).asString();
	}

	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return res;
}

string Client::get_os_name() {
	string req = req_base + "get_os";
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return res["os_name"].asString();
}

string Client::get_version() {
	string req = req_base + "get_version";
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return res["version"].asString();
}

pair<int, int> Client::get_speed() {
	string req = req_base + "get_speed";
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
	return make_pair(res["download"].asInt(), res["upload"].asInt());
}

void Client::shutdown() {
	string req = req_base + "shutdown";
	Json::Value res = _parseJSON(_request(req));
	CHECK_BTSYNC_ERROR(res);
}

//
// Utilities (depend on curl and jsoncpp)
//

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
	string data((const char*) ptr, (size_t) size * nmemb);
	*((stringstream*) stream) << data << endl;
	return size * nmemb;
}

string Client::_request(const string& url) {
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERPWD, (username + ":" + password).c_str());
	/* example.com is redirected, so we tell libcurl to follow redirection */
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
	stringstream out;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
	/* Perform the request, res will get the return code */
	CURLcode res = curl_easy_perform(curl);
	/* Check for errors */
	if (res != CURLE_OK) {
		throw CUrlException(string("Error: ") + curl_easy_strerror(res));
	}
	return out.str();	
}

Json::Value Client::_parseJSON(const string& str) {
	Json::Value result;
	bool ok = _reader.parse(str, result);
	if (!ok)
		throw JsonException("Failed to parse configuration\n");
	return result;
}
