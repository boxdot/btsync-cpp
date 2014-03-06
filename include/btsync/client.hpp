#include <string>
#include "json/json.h"

namespace Btsync {

	//
	// Custom Exceptions
	//

	class CUrlException : public std::runtime_error {
	public:
		explicit CUrlException(const std::string& what) : runtime_error(what) {}
	};

	class JsonException : public std::runtime_error {
	public:
		explicit JsonException(const std::string& what) : runtime_error(what) {}
	};

	class BtsyncException : public std::runtime_error {
	public:
		explicit BtsyncException(const std::string& what) : runtime_error(what) {}
	};

	//
	// Secret type
	// ENCRYPTION_READONLY is not used, since the functionality is not implemented
	//

	enum SecretType { READ_WRITE, READ_ONLY, ENCRYPTION_READONLY };

	//
	// BTSync class definition
	//

	class Client {

		const std::string username;
		const std::string password;

	public:
		Client(const std::string& u, const std::string& p);
		~Client();

		Json::Value get_folders();
	 	Json::Value get_folders(const std::string& secret);
	 	
	 	void add_folder(const std::string& dir);
		void add_folder(const std::string& dir, const std::string& secret);
		void remove_folder(const std::string& secret);

		Json::Value get_files(const std::string& secret, const std::string& path);
		Json::Value get_files(const std::string& secret);
	 	Json::Value set_file_preferences(const std::string& secret, 
	 		const std::string& path, const bool download);
		Json::Value get_folder_peers(const std::string& secret);
		
		std::pair<std::string, std::string> get_secrets();
		// returns a secret pair ('secret', read only secret for 'secret')
		std::pair<std::string, std::string> get_secrets(const std::string& secret);
		
		// api function: not implemented
		// bool get_secrets(const std::string& secret, const std::string& type);

		// non-api function: returns a pair (secret, type of secret)
	 	std::pair<std::string, SecretType> get_folder_secret(const std::string& dir);
	 	// non-api function: returns a pair (secret, secret_ro)
	 	std::pair<std::string, std::string> get_folder_secrets(const std::string& dir);

		Json::Value get_folder_preferences(const std::string& secret);
		Json::Value set_folder_preferences(const std::string& secret, const Json::Value& params);

		std::vector<std::string> get_folder_hosts(const std::string& secret);
		std::vector<std::string> set_folder_hosts(const std::string& secret, std::vector<std::string>& hosts);

		Json::Value get_preferences();
		Json::Value set_preferences(const Json::Value& params);
		std::string get_os_name();
		std::string get_version();
		std::pair<int, int> get_speed();
		void shutdown();

	private:
		Json::Reader _reader;
		std::string req_base;

		std::string _request(const std::string& url);
		Json::Value _parseJSON(const std::string& str);

	private:
		void* curl;
	};

}
