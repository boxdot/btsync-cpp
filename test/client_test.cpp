#define CATCH_CONFIG_MAIN

#include <unistd.h>
#include <boost/filesystem.hpp>
#include "catch.hpp"
#include "client.hpp"

#ifndef BTSYNC_LOGIN
#define BTSYNC_LOGIN "api"
#endif

#ifndef BTSYNC_PASSWORD
#define BTSYNC_PASSWORD "secret"
#endif

using namespace std;

TEST_CASE("Adding and removing folders", "[client]") {

	Btsync::Client c(BTSYNC_LOGIN, BTSYNC_PASSWORD);
	Json::Value res;

	SECTION("adding folder") {

		boost::filesystem::path dir("test_dir");
		boost::filesystem::create_directory(dir);
		dir = boost::filesystem::canonical(dir);

		CHECK_THROWS( c.add_folder("nonexistent_folder") );
		REQUIRE_NOTHROW( c.add_folder(dir.string()) );	
		CHECK_THROWS( c.add_folder(dir.string()) );
		
		CHECK_THROWS( c.get_folder_secrets("nonexistent_folder") );

		string secret, secret2 = "dummy";
		CHECK_NOTHROW( c.get_secrets() );
		CHECK_NOTHROW( secret = c.get_folder_secret(dir.string()).first );
		CHECK_NOTHROW( secret2 = c.get_folder_secrets(dir.string()).first );
		REQUIRE( secret == secret2 );
		CHECK_NOTHROW( c.get_secrets(secret) );

		INFO("Getting files");

		boost::filesystem::create_directory(dir / "subfolder");

		REQUIRE_NOTHROW( res = c.get_files(secret) );
		int countdown = 30;
		while ( !res.size() || countdown-- > 0 ) {
			res = c.get_files(secret);
			sleep(1);
		}

		REQUIRE( res.size() > 0 );
		
		CHECK_NOTHROW( res = c.get_files(secret, "/") );
		CHECK( res.isArray() );
		CHECK( res.size() == 1 );
		CHECK_NOTHROW( res = c.get_files(secret, "subfolder") );
		CHECK( res.size() == 0 );

		INFO("Getting/Setting folder preferences")

		CHECK_NOTHROW( res = c.get_folder_preferences(secret) );
		CHECK( res.isObject() );

		Json::Value prefs(Json::objectValue);
		prefs["use_hosts"] = "1";

		CHECK_NOTHROW( res = c.set_folder_preferences(secret, prefs) );
		CHECK( res["use_hosts"].asInt() == 1 );

		INFO("Getting folder hosts")

		vector<string> hosts;

		CHECK_NOTHROW( hosts = c.get_folder_hosts(secret) );

		static const string a[] = {"127.0.0.2:9999"};
		vector<string> new_hosts(a, a+1);
		CHECK_NOTHROW( hosts = c.set_folder_hosts(secret, new_hosts) );
		CHECK( hosts == new_hosts );

		INFO("Getting folder peers")

		CHECK_NOTHROW( c.get_folder_peers(secret) );

		CHECK_THROWS(c.remove_folder("wrong_secret"));
		CHECK_NOTHROW(c.remove_folder(secret));

		boost::filesystem::remove_all(dir);
	}
}

TEST_CASE("Getting folders") {

	Btsync::Client c(BTSYNC_LOGIN, BTSYNC_PASSWORD);
	Json::Value res;
	
	CHECK_NOTHROW( c.get_folders() );
	CHECK( res.isArray() );
}

TEST_CASE("Getting info", "[client]") {

	Btsync::Client c(BTSYNC_LOGIN, BTSYNC_PASSWORD);
	Json::Value res;

	CHECK_NOTHROW( c.get_os_name() );
	CHECK_NOTHROW( c.get_version() );
	CHECK_NOTHROW( c.get_speed() );
}

TEST_CASE("Preferences", "[client]") {

	Btsync::Client c(BTSYNC_LOGIN, BTSYNC_PASSWORD);
	Json::Value res;

	CHECK_NOTHROW( res = c.get_preferences() );
	string old_name = res["device_name"].asString();

	Json::Value prefs(Json::objectValue);
	prefs["device_name"] = "test_device";

	CHECK_NOTHROW( res = c.set_preferences(prefs) );
	REQUIRE( res["device_name"].asString() == "test_device" );
	CHECK_NOTHROW( res = c.set_preferences(prefs) );
} 
