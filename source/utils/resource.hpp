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
	virtual ~Res() = default;

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

    inline const resptr& get(const K& key) {
        auto it = _con.find(key);
		if (it == _con.end())
			return _empty;
		else
			return it->second;
    }

    inline const resptr& operator[] (const K& key) {
        auto it = _con.find(key);
        if (it == _con.end()) {
            resptr r = R::create(key);
            if (!r)
                return _empty;

			auto rit = _con.insert(conmap::value_type(key, r));
            return rit.first->second;
        }
        else
            return it->second;
    }

	inline conmap container() {
		return _con;
	}

private:
	inline static const resptr _empty{};
	conmap _con;
};

template <typename K, typename R>
using ResMgr = Singleton<ResContainer<K, R>>;
