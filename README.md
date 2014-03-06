A simple C++ wrapper class for BitTorrent Sync API. Check http://www.bittorrent.com/intl/de/sync/developers for more information.

Usage:

	#include <iostream>	
	#include "btsync/client.hpp"

	int main(int argc, char** argv) {

		Btsync::Client c("api_user", "password");
		Json::Value res = c.get_folders();
		std::cout << res << endl;
		return 0;
	}

# Build

The module depends on [json-cpp](https://github.com/mrtazz/json-cpp) and libcurl.

	$ git submodule update --init
	$ make

# Testing

The only test depends on boost and [Catch](https://github.com/philsquared/Catch). Build with

	$ make tests

and run the executable file in `bin`. Note that BitTorrent Sync must run in the api mode and you probably need to adapt the credentials in `Makefile`.