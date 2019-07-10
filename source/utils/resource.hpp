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

	inline virtual ~Res() {
		_id = 0;
	}

	inline void setID(const idt id) { _id = id; }
	inline const idt getID() const { return _id; }
	inline void setName(const nat& name) { _name = name; }
	inline const nat& getName() const { return _name; }

protected:
	inline Res() : _id(++_total){};
	inline Res(const std::string& name) : Res(), _name(name){};

public:
	inline static ptr empty{};
protected:
	inline static idt _total{0};
	idt _id;
	nat _name;
};

template <typename P, typename I>
class ResInst : public Res<I> {
public:
	using proto_ptr = typename Res<P>::ptr;
	using inst_ptr = typename Res<I>::ptr;

	template <typename ...ARGS>
	inline static inst_ptr create(const proto_ptr& proto, ARGS... args) {
		inst_ptr inst = I::create(proto, args...);
		if (!inst)
			return empty;

		inst->_proto = proto;
		return inst;
	}

	inline proto_ptr& prototype() { return _proto; }

protected:
	proto_ptr _proto;
};

template <typename P, typename I>
class ResProto : public Res<P> {
public:
	using insmap = std::map<typename I::idt, typename I::ptr>;

	template <typename ...ARGS>
	inline typename I::ptr instance(ARGS... args) {
		I::ptr inst = ResInst<P, I>::create(shared_from_this(), args...);
		if (!inst)
			return I::empty;
		_insts[inst->getID()] = inst;
		return inst;
	}

	inline insmap& container() { return _insts; }

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
			return R::empty;

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
			return R::empty;
		else
			return it->second;
	}
	inline const resptr& get(const resna& key) {
		const resid id = index(key);
		if (id)
			return get(id);
		else
			return R::empty;
	}

	template <typename... ARGS>
	inline const resptr& req(const resna& key, ARGS... args) {
		const resid id = index(key);
		if (id)
			return get(id);
		else
			return create(key, args...);
	}

	inline resmap& container() { return _rmap; }

private:
	keymap _kmap;
	resmap _rmap;
};

template <typename R>
using ResMgr = Singleton<ResContainer<R>>;

using strcube = std::array<std::string, 6>;
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

template <>
struct convert<strcube> {
	static Node encode(const strcube& rhs) {
		Node node;
		for (const auto& it: rhs)
			node.push_back(it);
		return node;
	}

	static bool decode(const Node& node, strcube& rhs) {
		if (!node.IsSequence() || node.size() != 6) {
			return false;
		}

		for (int i = 0; i < 6; ++i)
			rhs[i] = node[i].as<std::string>();
		return true;
	}
};
} // namespace YAML

class Config {
public:
	using node = YAML::Node;
	static const node visit(const node& doc, const std::string& path);
	static std::any guess(const node& doc);
	inline static bool valid(const node& doc) { return doc.IsDefined() && !doc.IsNull(); }

	bool load(const std::filesystem::path& path);
	const node& root() const {
		return _doc;
	}
	inline const node operator[](const std::string& path) const {
		return visit(_doc, path);
	}

public:
	inline static node empty{};
private:
	node _doc;
};
