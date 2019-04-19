#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include "utils/pattern.hpp"

bool readFile(const std::filesystem::path& path, std::string& content);

template <typename R>
class Res: public std::enable_shared_from_this<R> {
public:
    using ptr = std::shared_ptr<R>;
	virtual ~Res(){};

protected:
    Res() = default;
};

template <typename K, typename R>
class ResContainer: public NoCopy {
public:
	using resptr = typename R::ptr;
    using conmap = std::map<K, resptr>;

    inline void add(const K& key, const resptr& res) {
        _con[key] = res;
    }

    inline void del(const K& key) {
        _con.erase(key);
    }

    inline resptr get(const K& key) {
        auto it = _con.find(key);
		if (it == _con.end())
			return nullptr;
		else
			return it->second;
    }

	inline conmap container() {
		return _con;
	}

private:
	conmap _con;
};

template <typename K, typename R>
using ResMgr = Singleton<ResContainer<K, R>>;
