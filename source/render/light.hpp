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
	virtual ~LightInst() = default;

	inline void setPos(const glm::vec3& pos) { _pos = pos; }
	inline const glm::vec3& getPos() const { return _pos; }
	inline void setDir(const glm::vec3& dir) { _dir = dir; }
	inline const glm::vec3& getDir() const { return _dir; }

private:
	glm::vec3 _pos{0.0f}, _dir{0.0f};
};

class LightProto: public ResProto<LightProto, LightInst> {
public:
	static ptr create(const std::string& name);
	virtual ~LightProto() = default;

public:
	Attributes attrs;
private:
	Config _conf;
};

using LightProtoMgr = ResMgr<LightProto>;
