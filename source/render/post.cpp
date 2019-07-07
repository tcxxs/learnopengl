#include "render/post.hpp"
#include "render/vertex.hpp"

Post::ptr Post::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "posts.yml";
		if (!_confs.load(path)) {
			std::cout << "posteffects config error";
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!conf.IsDefined())
		return {};

	if (!_vbo) {
		if (!_initVBO())
			return {};
	}

	Post::ptr post = std::shared_ptr<Post>(new Post());
	post->setName(name);
	post->_shader = ShaderMgr::inst().req(conf["shader"].as<std::string>());
	if (!post->_shader)
		return {};
	for (auto& it: conf["in"]) {
		post->_ins.push_back(it.as<std::string>());
	}

	if (!post->_initVAO())
		return {};

	return post;
}

bool Post::_initVBO() {
	int size = 6 * (3 + 2 + 3);
	float verts[6 * (3 + 2 + 3)] = {
		// pos3, uv2, normal3
	    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	    1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, size * sizeof(float), verts, GL_DYNAMIC_STORAGE_BIT);

	if (oglError())
		return false;
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

void Post::draw(const framevec& ins) {
	_shader->use();

	GLuint tex = 0;
	for (int i = 0; i < _ins.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, ins[i]->getTexture());
		_shader->setVar(_ins[i], tex);
		tex += 1;
	}

	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

bool Post::_initVAO() {
	glCreateVertexArrays(1, &_vao);
	if (!_shader->bindVertex(VERTEX_BASE, _vao, _vbo))
		return false;

	return true;
}
