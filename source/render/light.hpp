#pragma once

#include <any>
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"

class LightProto;
class LightInst: public ResInst<LightProto, LightInst> {
public:
	static ptr create(const proto_ptr& proto, const Config::node& conf);

	inline void setPos(const glm::vec3& pos) { _pos = pos; }
	inline const glm::vec3& getPos() const { return _pos; }
	inline void setDir(const glm::vec3& dir) { _dir = dir; }
	inline const glm::vec3& getDir() const { return _dir; }

private:
	glm::vec3 _pos{0.0f}, _dir{0.0f};
};

class LightProto: public ResProto<LightProto, LightInst> {
public:
	enum lighttype {
		LIGHT_DIR = 1,
		LIGHT_POINT,
		LIGHT_SPOT,
	};
	static ptr create(const std::string& name);

	inline const lighttype getType() const { return _type; }

public:
	Attributes attrs;

private:
	inline static Config _confs;
	lighttype _type{LIGHT_DIR};
};

using LightProtoMgr = ResMgr<LightProto>;
