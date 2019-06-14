#include "render/post.hpp"

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

	Post::ptr post = std::shared_ptr<Post>(new Post());
	post->setName(name);
	post->_shader = ShaderMgr::inst().req(conf["shader"].as<std::string>());
	if (!post->_shader)
		return {};
	for (auto& it: conf["in"]) {
		post->_ins.push_back(it.as<std::string>());
	}

	if (!post->_initGL())
		return {};

	return post;
}

Post::~Post() {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	if (_vbo) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
}

bool Post::_initGL() {
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), _vertexs, GL_STATIC_DRAW);

	glEnableVertexAttribArray(POS_LOC);
	glVertexAttribPointer(POS_LOC, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(UV_LOC);
	glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (oglError())
		return false;

	return true;
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
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
