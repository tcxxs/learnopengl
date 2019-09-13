#include "render/process.hpp"
#include "render/vertex.hpp"

Process::ptr Process::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "procs.yml";
		if (!_confs.load(path)) {
			ERR("process config error");
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		ERR("process config not find, %s", name.c_str());
		return {};
	}

	if (_vbos.empty()) {
		_initVBO();
	}

	Process::ptr proc = std::make_shared<Process>();
	proc->setName(name);
	const std::string& shape = conf["shape"].as<std::string>();
	const auto& find = _vbos.find(shape);
	if (find == _vbos.end()) {
		ERR("proc shape %s not found", shape.c_str());
		return {};
	}
	proc->_vbo = find->second;

	proc->_material = MaterialMgr::inst().req(conf["material"].as<std::string>());
	if (!proc->_material)
		return {};
	if (!proc->_initVAO())
		return {};

	return proc;
}

void Process::_initVBO() {
	GLuint vbo{0};
	constexpr int screen_size = 6 * (3 + 2 + 3 + 3);
	float screen_verts[screen_size] = {
	    // pos3, uv2, normal3, tangent3
	    -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0f};
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, screen_size * sizeof(float), screen_verts, GL_DYNAMIC_STORAGE_BIT);
	_vbos["screen"] = vbo;

	constexpr int cube_size = 6 * 6 * (3 + 2 + 3 + 3);
	float cube_verts[cube_size] = {
	    // pos3, uv2, normal3, tangent3
	    -0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
	    -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,

	    -0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, 0.5, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, 0.5, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
	    -0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,

	    -0.5, 0.5, 0.5, 1.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, 0.5, -0.5, 1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, -0.5, 0.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, -0.5, 0.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, 0.5, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, 0.5, 0.5, 1.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,

	    0.5, 0.5, 0.5, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, -0.5, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, -0.5, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, 0.5, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,

	    -0.5, -0.5, -0.5, 0.0, 1.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, -0.5, 1.0, 1.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, 0.5, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, -0.5, 0.5, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, 0.5, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, -0.5, -0.5, 0.0, 1.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,

	    -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, -0.5, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
	    0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
	    -0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, cube_size * sizeof(float), cube_verts, GL_DYNAMIC_STORAGE_BIT);
	_vbos["cube"] = vbo;
}

Process::~Process() {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	//if (_vbo) {
	//	glDeleteBuffers(1, &_vbo);
	//	_vbo = 0;
	//}
}

bool Process::_initVAO() {
	glCreateVertexArrays(1, &_vao);
	if (!_material->getShader()->bindVertex(VERTEX_BASE, _vao, _vbo))
		return false;

	return true;
}

int Process::draw(CommandQueue& cmds) {
	auto& cmd = cmds.emplace_back();
	cmd.vao = _vao;
	cmd.verts = 6;
	cmd.material = _material;
	return 1;
}
