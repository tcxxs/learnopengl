#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <any>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "yaml-cpp/yaml.h"
#include "utils/pattern.hpp"

bool readFile(const std::filesystem::path& path, std::string& content);

template <typename R>
class Res : public std::enable_shared_from_this<R> {
public:
	using ptr = std::shared_ptr<R>;
	using idt = unsigned long long;
	using nat = std::string;

	virtual ~Res() = default;

	inline void setID(const idt id) { _id = id; }
	inline const idt getID() const { return _id; }
	inline void setName(const nat& name) { _name = name; }
	inline const nat& getName() const { return _name; }

protected:
	inline Res() : _id(++_total){};
	inline Res(const std::string& name) : Res(), _name(name){};

protected:
	inline static idt _total{0};
	idt _id;
	nat _name;
};

template <typename P, typename I>
class ResInst : public Res<I> {
public:
	using proto_ptr = std::shared_ptr<P>;
	using inst_ptr = typename Res<I>::ptr;

	template <typename ...ARGS>
	inline static inst_ptr create(const proto_ptr& proto, ARGS... args) {
		inst_ptr inst = I::create(proto, args...);
		inst->_proto = proto;
		return inst;
	}

protected:
	proto_ptr _proto;
};

template <typename P, typename I>
class ResProto : public Res<P> {
public:
	using insmap = std::map<typename I::idt, typename I::ptr>;

	template <typename ...ARGS>
	inline typename I::ptr instance(ARGS... args) {
		I::ptr inst = I::create(shared_from_this(), args...);
		_insts[inst->getID()] = inst;
		return inst;
	}

protected:
	insmap _insts;
};

template <typename R>
class ResContainer : public NoCopy {
public:
	using resptr = typename R::ptr;
	using resid = typename R::idt;
	using resna = typename R::nat;
	using keymap = std::map<resna, resid>;
	using resmap = std::map<resid, resptr>;

	inline const resid index(const resna& key) {
		const auto& it = _kmap.find(key);
		if (it == _kmap.end())
			return 0;
		else
			return it->second;
	}

	template <typename... ARGS>
	inline const resptr& create(ARGS... args) {
		resptr r = R::create(args...);
		if (!r)
			return _empty;

		return add(r);
	}

	inline const resptr& add(const resptr& res) {
		const resid id = res->getID();
		const resna na = res->getName();
		if (!na.empty())
			_kmap[na] = id;
		auto ret = _rmap.insert(resmap::value_type(id, res));
		return ret.first->second;
	}

	inline void del(const resid id) {
		_rmap.erase(key);
	}
	inline void del(const resna& key) {
		const resid id = index(key);
		if (id)
			del(it->second);
	}

	inline const resptr& get(const resid id) {
		auto it = _rmap.find(id);
		if (it == _rmap.end())
			return _empty;
		else
			return it->second;
	}
	inline const resptr& get(const resna& key) {
		const resid id = index(key);
		if (id)
			return get(id);
		else
			return _empty;
	}

	template <typename... ARGS>
	inline const resptr& req(const resna& key, ARGS... args) {
		const resid id = index(key);
		if (id)
			return get(id);
		else
			return create(key, args...);
	}

	inline resmap container() {
		return _rmap;
	}

private:
	inline static const resptr _empty{};
	keymap _kmap;
	resmap _rmap;
};

template <typename R>
using ResMgr = Singleton<ResContainer<R>>;

namespace YAML {
template <>
struct convert<glm::vec3> {
	static Node encode(const glm::vec3& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		return node;
	}

	static bool decode(const Node& node, glm::vec3& rhs) {
		if (!node.IsSequence() || node.size() != 3) {
			return false;
		}

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		return true;
	}
};
} // namespace YAML

class Config {
public:
	using node = YAML::Node;
	static const node visit(const node& doc, const std::string& path);
	static std::any guess(const node& doc);

	bool load(const std::filesystem::path& path);
	const node& root() const {
		return _doc;
	}
	inline const node operator[](const std::string& path) const {
		return visit(_doc, path);
	}

private:
	inline static node _empty{};
	node _doc;
};
