#include "render/post.hpp"
#include "render/vertex.hpp"

Post::ptr Post::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "posts.yml";
		if (!_confs.load(path)) {
			ERR("posteffects config error");
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		ERR("posts config not find, %s", name.c_str());
		return {};
	}

	if (!_vbo) {
		if (!_initVBO())
			return {};
	}

	Post::ptr post = std::make_shared<Post>();
	post->setName(name);
	post->_material = MaterialMgr::inst().req(conf["material"].as<std::string>());
	if (!post->_material)
		return {};
	if (!post->_initVAO())
		return {};

	return post;
}

bool Post::_initVBO() {
	constexpr int size = 6 * (3 + 2 + 3 + 3);
	float verts[size] = {
		// pos3, uv2, normal3, tangent3
	    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, size * sizeof(float), verts, GL_DYNAMIC_STORAGE_BIT);

	return true;
}

Post::~Post() {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	//if (_vbo) {
	//	glDeleteBuffers(1, &_vbo);
	//	_vbo = 0;
	//}
}

bool Post::_initVAO() {
	glCreateVertexArrays(1, &_vao);
	if (!_material->getShader()->bindVertex(VERTEX_BASE, _vao, _vbo))
		return false;

	return true;
}

int Post::draw(CommandQueue& cmds) {
	auto& cmd = cmds.emplace_back();
	cmd.vao = _vao;
	cmd.verts = 6;
	cmd.material = _material;
	return 1;
}
