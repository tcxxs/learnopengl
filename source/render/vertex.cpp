#include "vertex.hpp"

VertexProto::ptr VertexProto::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "vertexs.yml";
		if (!_confs.load(path)) {
			std::cout << "vertexs config error";
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		std::printf("vertexs config not find, %s", name.c_str());
		return {};
	}

	VertexProto::ptr proto = std::make_shared<VertexProto>();
	proto->setName(name);
	proto->_bind = _binding;
	++_binding;

	proto->_divisor = conf["divisor"].as<int>(0);
	for (const auto& it: conf["vars"]) {
		const std::string& name = it.first.as<std::string>();
		const std::string& type = it.second.as<std::string>();
		const auto& find = _typenames.find(type);
		if (find == _typenames.end()) {
			std::cout << "vertex attribute type unknow: " << name << ", " << type << std::endl;
			return {};
		}

		const attrtype& atype = _typeinfos[find->second];
		proto->_attrs.emplace(name, std::make_tuple(atype, proto->_size));
		proto->_size += std::get<3>(atype) * std::get<4>(atype) * std::get<5>(atype);
	}

	return proto;
}

VertexInst::ptr VertexInst::create(const proto_ptr& proto) {
	VertexInst::ptr vertex = std::make_shared<VertexInst>();

	return vertex;
}

bool VertexInst::setAttr(const std::string& name, GLint loc, GLint type) {
	const VertexProto::attrinfo& ainfo = _proto->findInfo(name);
	const VertexProto::attrtype& atype = std::get<0>(ainfo);
	if (!std::get<0>(atype)) {
		std::cout << "vertex attribute no define: " << name << std::endl;
		return false;
	}
	if (std::get<1>(atype) != type) {
		std::cout << "vertex attribute type error: " << name << std::endl;
		return false;
	}

	_attrs.emplace(name, loc);
	return true;
}

bool VertexInst::bindBuffer(GLuint vao, GLuint vbo) {
	GLint loc{0};
	GLuint bind = _proto->getBind();
	for (const auto& it: _attrs) {
		const VertexProto::attrinfo& ainfo = _proto->findInfo(it.first);
		const VertexProto::attrtype& atype = std::get<0>(ainfo);
		// 例如matrix，shader中实际是按行分为4个vec4
		int row = std::get<3>(atype) * std::get<5>(atype);
		for (int i = 0; i < std::get<4>(atype); ++i) {
			loc = it.second + i;
			glVertexArrayAttribFormat(vao, loc, std::get<5>(atype), std::get<2>(atype), FALSE, std::get<1>(ainfo) + i * row);
			glVertexArrayAttribBinding(vao, loc, bind);
			glEnableVertexArrayAttrib(vao, loc);
		}
	}

	glVertexArrayVertexBuffer(vao, bind, vbo, 0, _proto->getSize());
	glVertexArrayBindingDivisor(vao, bind, _proto->getDivisor());
	return true;
}
