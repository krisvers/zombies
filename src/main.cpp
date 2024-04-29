#include <kgfx/kgfx.h>

extern "C" {
#include <kwnd/kwnd.h>
}

#include <linmath.h>
#define KOML_IMPLEMENTATION
#include <koml.h>

#include <iostream>
#include <vector>
#include <tuple>
#include <array>
#include <filesystem>
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using KGFXdestroyfunc = void(*)(KGFXcontext context, void*);

struct DestructionPair {
	KGFXdestroyfunc func;
	void* object;
};

struct Vertex {
	float position[3];
	float normal[3];
	float color[3];
	float texcoord[2];
};

struct MatrixUniform {
	mat4x4 model;
	mat4x4 view;
	mat4x4 projection;
};

struct Camera {
	vec3 position;
	vec3 rotation;
	float fov;
	float aspectRatio;
	float nearPlane;
	float farPlane;
};

struct GameObject {
	usize id;
	vec3 position;
	vec3 rotation;
	vec3 scale;

	GameObject() : position{ 0, 0, 0 }, rotation{ 0, 0, 0 }, scale{ 1, 1, 1 } { }

	virtual void update() = 0;
};

struct Renderable {
	usize gameObjectID;
	vec3 position = { 0, 0, 0 };
	vec3 rotation = { 0, 0, 0 };
	vec3 scale = { 1, 1, 1 };

	KGFXbuffer vertexBuffer = nullptr;
	u32 vertexCount = 0;
	KGFXbuffer indexBuffer = nullptr;
	u32 indexCount = 0;
	KGFXpipeline pipeline = nullptr;

	void calculateMVP(mat4x4 model) {
		mat4x4_identity(model);
		mat4x4_translate(model, position[0], position[1], position[2]);
		mat4x4_rotate_X(model, model, rotation[0]);
		mat4x4_rotate_Y(model, model, rotation[1]);
		mat4x4_rotate_Z(model, model, rotation[2]);
		mat4x4_scale_aniso(model, model, scale[0], scale[1], scale[2]);
	}

	void addSetupCommandsToList(KGFXcontext context, KGFXcommandlist commandList) {
		if (vertexBuffer == nullptr || pipeline == nullptr) {
			return;
		}

		kgfxCommandBindPipeline(context, commandList, pipeline);
		kgfxCommandBindVertexBuffer(context, commandList, vertexBuffer, 0);
		if (indexBuffer != nullptr) {
			kgfxCommandBindIndexBuffer(context, commandList, indexBuffer, 0);
		}
	}

	void addDrawCommandsToList(KGFXcontext context, KGFXcommandlist commandList) {
		if (vertexBuffer == nullptr || pipeline == nullptr) {
			return;
		}

		if (indexBuffer != nullptr) {
			kgfxCommandDrawIndexed(context, commandList, indexCount, 1, 0, 0, 0);
			return;
		}

		kgfxCommandDraw(context, commandList, vertexCount, 1, 0, 0);
	}
};

struct GameObjectRenderable : public GameObject {
	Renderable& renderable;

	GameObjectRenderable(Renderable& r) : renderable(r), GameObject() { }

	void update() override {
		updateRenderable();
	}

	void updateRenderable() {
		renderable.position[0] = position[0];
		renderable.position[1] = position[1];
		renderable.position[2] = position[2];

		renderable.rotation[0] = rotation[0];
		renderable.rotation[1] = rotation[1];
		renderable.rotation[2] = rotation[2];

		renderable.scale[0] = scale[0];
		renderable.scale[1] = scale[1];
		renderable.scale[2] = scale[2];
	}
};

struct ContextConfig {
	koml_table_t koml;
	koml_table_t shaderKOML;
	std::string assetDirectory = ".";
	std::string shaderDirectory = ".";
};

struct ShaderDescriptionAttributeConfig {
	char* semanticName;
	int semanticIndex;
	char* type;
	int location;
};

struct ShaderDescriptionBindingConfig {
	char* inputRate;
	std::vector<ShaderDescriptionAttributeConfig> attributes;
	char* bindpoint;
	int binding;
};

struct ShaderDescriptionDescriptorSetConfig {
	char* bindpoint;
	int binding;
	char* usage;
	int size;
};

struct ShaderDescriptionConfig {
	char* cullMode;
	char* frontFace;
	char* fillMode;
	char* topology;
	std::vector<ShaderDescriptionBindingConfig> bindings;
	std::vector<ShaderDescriptionDescriptorSetConfig> descriptorSets;
};

struct ShaderConfig {
	char* name;
	char* vertexPath;
	char* vertexEntry;
	char* vertexMedium;
	char* fragmentPath;
	char* fragmentEntry;
	char* fragmentMedium;

	ShaderDescriptionConfig description;
};

using ShaderDescriptionAttributeConfigParsed = KGFXpipelineattribute;

struct ShaderDescriptionBindingConfigParsed {
	KGFXinputrate inputRate;
	std::vector<ShaderDescriptionAttributeConfigParsed> attributes;
	KGFXbindpoint bindpoint;
	u32 binding;
};

struct ShaderDescriptionConfigParsed {
	KGFXcullmode cullMode;
	KGFXfrontface frontFace;
	KGFXfillmode fillMode;
	KGFXtopology topology;
	std::vector<ShaderDescriptionBindingConfigParsed> bindings;
	std::vector<KGFXdescriptorsetdesc> descriptorSets;
};

struct ShaderConfigParsed {
	std::string name;
	std::string vertexPath;
	std::string vertexEntry;
	KGFXshadermedium vertexMedium;
	std::string fragmentPath;
	std::string fragmentEntry;
	KGFXshadermedium fragmentMedium;

	ShaderDescriptionConfigParsed description;
};

struct Shader {
	KGFXpipeline pipeline = nullptr;
};

struct ShaderStorage {
	std::unordered_map<std::string, Shader> shaders;
};

struct Context {
	KGFXcontext context = nullptr;
	KGFXcommandlist commandList = nullptr;
	KGFXpipeline defaultPipeline = nullptr;
	ContextConfig config;
	ShaderStorage shaderStorage;

	Camera camera = {
		.position = { 0, 0, 0 },
		.rotation = { 0, 0, 0 },
		.fov = 80.0f,
		.aspectRatio = 800.0f / 600.0f,
		.nearPlane = 0.1f,
		.farPlane = 100.0f,
	};

	std::vector<DestructionPair> destructionPairs;
	std::vector<GameObject*> gameObjects;
	std::vector<Renderable*> renderables;

	static inline std::array<bool, 256> keys;
	static inline std::array<bool, 256> prevKeys;
	static inline std::array<bool, 5> mouseButtons;
	static inline std::array<bool, 5> prevMouseButtons;

	static inline float mouseX = 0;
	static inline float mouseY = 0;
	static inline float mouseZ = 0;
	static inline float prevMouseX = 0;
	static inline float prevMouseY = 0;
	static inline float prevMouseZ = 0;
	static inline float deltaMouseX = 0;
	static inline float deltaMouseY = 0;
	static inline float deltaMouseZ = 0;

	static inline float deltaTime = 0;
	static inline float lastTime;

	bool inited = false;
	bool deinited = false;

	bool isWindowed = false;
	u32 windowWidth = 0;
	u32 windowHeight = 0;
	kwnd_window_t* window = nullptr;

	int init(kwnd_window_t* w) {
		if (inited) {
			return 0;
		}

		inited = true;
		deinited = false;

		if (loadConfig(config.koml, "config.koml") != 0) {
			std::cerr << "Failed to load config.koml" << std::endl;
			return 1;
		}

		koml_table_print(&config.koml);

		koml_symbol_t* assetDirectory = koml_table_symbol(&config.koml, "config:assetDirectory");
		if (assetDirectory != nullptr) {
			if (assetDirectory->type == KOML_TYPE_STRING) {
				config.assetDirectory = std::string(assetDirectory->data.string, assetDirectory->stride);
			} else {
				std::cerr << "assetDirectory in config.koml is not a string" << std::endl;
			}
		}

		koml_symbol_t* shaderDirectory = koml_table_symbol(&config.koml, "config:shaderDirectory");
		if (shaderDirectory != nullptr) {
			if (shaderDirectory->type == KOML_TYPE_STRING) {
				config.shaderDirectory = std::string(shaderDirectory->data.string, shaderDirectory->stride);
			} else {
				std::cerr << "shaderDirectory in config.koml is not a string" << std::endl;
			}
		}

		if (loadConfig(config.shaderKOML, config.shaderDirectory + "/shaders.koml") != 0) {
			std::cerr << "Failed to load " + config.shaderDirectory + "/shaders.koml" << std::endl;
			return 1;
		}

		koml_table_print(&config.shaderKOML);

		if (kgfxCreateContext(KGFX_ANY_VERSION, { w->platform.hwnd }, &context) != KGFX_SUCCESS) {
			std::cerr << "Failed to create KGFX context" << std::endl;
			return 1;
		}

		commandList = kgfxCreateCommandList(context);
		if (commandList == nullptr) {
			std::cerr << "Failed to create command list" << std::endl;
			return 1;
		}

		addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyCommandList), commandList);

		if (!loadShaders()) {
			std::cerr << "Failed to load shaders" << std::endl;
			return 1;
		}

		window = w;
		return 0;
	}

	void addForDestruction(KGFXdestroyfunc func, void* object) {
		destructionPairs.push_back({ func, object });
	}

	void addGameObject(GameObject* object) {
		gameObjects.push_back(object);
		object->id = gameObjects.size();
	}

	void addGameObject(GameObject& object) {
		gameObjects.push_back(&object);
		object.id = gameObjects.size();
	}

	void addRenderable(Renderable* renderable) {
		renderables.push_back(renderable);
	}
	
	void addRenderable(Renderable& renderable) {
		renderables.push_back(&renderable);
	}

	void addGameObjectRenderable(GameObjectRenderable* object) {
		addGameObject(object);
		addRenderable(object->renderable);
		object->id = gameObjects.size();
		object->renderable.gameObjectID = object->id;
	}

	void addGameObjectRenderable(GameObjectRenderable& object) {
		addGameObject(object);
		addRenderable(object.renderable);
		object.id = gameObjects.size();
		object.renderable.gameObjectID = object.id;
	}

	void update() {
		prevKeys = keys;
		prevMouseButtons = mouseButtons;

		deltaMouseX = mouseX - prevMouseX;
		deltaMouseY = mouseY - prevMouseY;
		deltaMouseZ = mouseZ - prevMouseZ;

		prevMouseX = mouseX;
		prevMouseY = mouseY;
		prevMouseZ = mouseZ;

		deltaTime = static_cast<float>(0.016f);
		lastTime = static_cast<float>(1);

		for (auto& object : gameObjects) {
			object->update();
		}
	}

	bool reloadShaders() {
		std::vector<std::vector<DestructionPair>::iterator> iteratorsToErase;
		for (auto& pair : shaderStorage.shaders) {
			for (u32 i = 0; i < destructionPairs.size(); ++i) {
				if (destructionPairs[i].object == pair.second.pipeline) {
					destructionPairs[i].func(context, destructionPairs[i].object);
					iteratorsToErase.push_back(destructionPairs.begin() + i);
				}
			}
		}

		for (auto& iter : iteratorsToErase) {
			destructionPairs.erase(iter);
		}

		shaderStorage.shaders.clear();

		if (!loadShaders()) {
			std::cerr << "Failed to reload shaders" << std::endl;
			return false;
		}

		return true;
	}

	void deinit() {
		if (deinited || !inited) {
			return;
		}

		deinited = true;
		inited = false;

		if (config.koml.symbols != nullptr) {
			koml_table_destroy(&config.koml);
		}

		if (config.shaderKOML.symbols != nullptr) {
			koml_table_destroy(&config.shaderKOML);
		}

		for (auto& pair : destructionPairs) {
			pair.func(context, pair.object);
		}
		destructionPairs.clear();

		if (context != nullptr) {
			kgfxDestroyContext(context);
		}
	}

	std::string komlTypeToString(koml_type_enum type) {
		switch (type) {
			case KOML_TYPE_UNKNOWN:
				return "unknown";
			case KOML_TYPE_INT:
				return "int";
			case KOML_TYPE_FLOAT:
				return "float";
			case KOML_TYPE_STRING:
				return "string";
			case KOML_TYPE_BOOLEAN:
				return "boolean";
			case KOML_TYPE_ARRAY:
				return "array";
		}
	}

	bool loadKOMLSymbol(koml_table_t& table, std::string const& label, koml_type_enum expectedType, koml_symbol_t*& symbol) {
		koml_symbol_t* sym = koml_table_symbol(&table, label.c_str());
		if (sym == nullptr) {
			std::cerr << "Failed to find " << label << std::endl;
			return false;
		}

		if (sym->type != expectedType) {
			std::cerr << label << ":inputRate is not a " << komlTypeToString(expectedType) << std::endl;
			return false;
		}

		symbol = sym;
		return true;
	}

	bool loadShaderAttributeConfig(koml_table_t& table, std::string const& label, ShaderDescriptionAttributeConfig& attr) {
		koml_symbol_t* semanticName;
		if (!loadKOMLSymbol(table, label + ":semanticName", KOML_TYPE_STRING, semanticName)) {
			return false;
		}

		attr.semanticName = semanticName->data.string;

		koml_symbol_t* semanticIndex;
		if (!loadKOMLSymbol(table, label + ":semanticIndex", KOML_TYPE_INT, semanticIndex)) {
			return false;
		}

		attr.semanticIndex = semanticIndex->data.i32;

		koml_symbol_t* type;
		if (!loadKOMLSymbol(table, label + ":type", KOML_TYPE_STRING, type)) {
			return false;
		}

		attr.type = type->data.string;

		koml_symbol_t* location;
		if (!loadKOMLSymbol(table, label + ":location", KOML_TYPE_INT, location)) {
			return false;
		}

		attr.location = location->data.i32;
		return true;
	}

	bool loadShaderBindingConfig(koml_table_t& table, std::string const& label, ShaderDescriptionBindingConfig& cfg) {
		koml_symbol_t* inputRate;
		if (!loadKOMLSymbol(table, label + ":inputRate", KOML_TYPE_STRING, inputRate)) {
			return false;
		}

		cfg.inputRate = inputRate->data.string;

		koml_symbol_t* attributes;
		if (!loadKOMLSymbol(table, label + ":attributes", KOML_TYPE_ARRAY, attributes)) {
			return false;
		}

		if (attributes->data.array.type != KOML_TYPE_STRING) {
			std::cerr << label << ":attributes is not an array of strings" << std::endl;
			return false;
		}

		cfg.attributes.resize(attributes->data.array.length);
		for (u32 i = 0; i < attributes->data.array.length; ++i) {
			if (!loadShaderAttributeConfig(table, std::string(attributes->data.array.elements.string[i]), cfg.attributes[i])) {
				return false;
			}
		}

		koml_symbol_t* bindpoint;
		if (!loadKOMLSymbol(table, label + ":bindpoint", KOML_TYPE_STRING, bindpoint)) {
			return false;
		}

		cfg.bindpoint = bindpoint->data.string;

		koml_symbol_t* binding;
		if (!loadKOMLSymbol(table, label + ":binding", KOML_TYPE_INT, binding)) {
			return false;
		}

		cfg.binding = binding->data.i32;
		return true;
	}

	bool loadShaderDescriptionDescriptorSetConfig(koml_table_t& table, std::string const& label, ShaderDescriptionDescriptorSetConfig& desc) {
		koml_symbol_t* bindpoint;
		if (!loadKOMLSymbol(table, label + ":bindpoint", KOML_TYPE_STRING, bindpoint)) {
			return false;
		}

		desc.bindpoint = bindpoint->data.string;

		koml_symbol_t* binding;
		if (!loadKOMLSymbol(table, label + ":binding", KOML_TYPE_INT, binding)) {
			return false;
		}

		desc.binding = binding->data.i32;

		koml_symbol_t* usage;
		if (!loadKOMLSymbol(table, label + ":usage", KOML_TYPE_STRING, usage)) {
			return false;
		}

		desc.usage = usage->data.string;

		koml_symbol_t* size;
		if (!loadKOMLSymbol(table, label + ":size", KOML_TYPE_INT, size)) {
			return false;
		}

		desc.size = size->data.i32;
		return true;
	}

	bool loadShaderDescriptionConfig(koml_table_t& table, std::string const& label, ShaderDescriptionConfig& desc) {
		koml_symbol_t* cullMode;
		if (!loadKOMLSymbol(table, label + ":cullMode", KOML_TYPE_STRING, cullMode)) {
			return false;
		}

		desc.cullMode = cullMode->data.string;

		koml_symbol_t* frontFace;
		if (!loadKOMLSymbol(table, label + ":frontFace", KOML_TYPE_STRING, frontFace)) {
			return false;
		}

		desc.frontFace = frontFace->data.string;

		koml_symbol_t* fillMode;
		if (!loadKOMLSymbol(table, label + ":fillMode", KOML_TYPE_STRING, fillMode)) {
			return false;
		}

		desc.fillMode = fillMode->data.string;

		koml_symbol_t* topology;
		if (!loadKOMLSymbol(table, label + ":topology", KOML_TYPE_STRING, topology)) {
			return false;
		}

		desc.topology = topology->data.string;

		koml_symbol_t* bindings;
		if (!loadKOMLSymbol(table, label + ":bindings", KOML_TYPE_ARRAY, bindings)) {
			return false;
		}
		
		if (bindings->data.array.type != KOML_TYPE_STRING) {
			std::cerr << label << ":bindings is not an array of strings" << std::endl;
			return false;
		}

		desc.bindings.resize(bindings->data.array.length);
		for (u32 i = 0; i < bindings->data.array.length; ++i) {
			if (!loadShaderBindingConfig(table, std::string(bindings->data.array.elements.string[i]), desc.bindings[i])) {
				return false;
			}
		}

		koml_symbol_t* descriptorSets;
		if (!loadKOMLSymbol(table, label + ":descriptorSets", KOML_TYPE_ARRAY, descriptorSets)) {
			return false;
		}

		if (descriptorSets->data.array.type != KOML_TYPE_STRING) {
			std::cerr << label << ":descriptorSets is not an array of strings" << std::endl;
			return false;
		}

		desc.descriptorSets.resize(descriptorSets->data.array.length);
		for (u32 i = 0; i < descriptorSets->data.array.length; ++i) {
			if (!loadShaderDescriptionDescriptorSetConfig(table, std::string(descriptorSets->data.array.elements.string[i]), desc.descriptorSets[i])) {
				return false;
			}
		}

		return true;
	}

	bool loadShaderConfig(koml_table_t& table, std::string const& label, ShaderConfig& cfg) {
		koml_symbol_t* name;
		if (!loadKOMLSymbol(table, label + ":name", KOML_TYPE_STRING, name)) {
			return false;
		}

		cfg.name = name->data.string;

		koml_symbol_t* vertexPath;
		if (!loadKOMLSymbol(table, label + ":vertexPath", KOML_TYPE_STRING, vertexPath)) {
			return false;
		}

		cfg.vertexPath = vertexPath->data.string;

		koml_symbol_t* vertexEntry = koml_table_symbol(&table, (label + ":vertexEntry").c_str());
		if (vertexEntry == nullptr) {
			cfg.vertexEntry = const_cast<char*>("main");
		} else {
			if (vertexEntry->type != KOML_TYPE_STRING) {
				std::cerr << label << ":vertexEntry is not a string" << std::endl;
				return false;
			}

			cfg.vertexEntry = vertexEntry->data.string;
		}

		koml_symbol_t* vertexMedium;
		if (!loadKOMLSymbol(table, label + ":vertexMedium", KOML_TYPE_STRING, vertexMedium)) {
			return false;
		}

		cfg.vertexMedium = vertexMedium->data.string;

		koml_symbol_t* fragmentPath;
		if (!loadKOMLSymbol(table, label + ":fragmentPath", KOML_TYPE_STRING, fragmentPath)) {
			return false;
		}

		cfg.fragmentPath = fragmentPath->data.string;

		koml_symbol_t* fragmentEntry = koml_table_symbol(&table, (label + ":fragmentEntry").c_str());
		if (fragmentEntry == nullptr) {
			cfg.fragmentEntry = const_cast<char*>("main");
		} else {
			if (fragmentEntry->type != KOML_TYPE_STRING) {
				std::cerr << label << ":fragmentEntry is not a string" << std::endl;
				return false;
			}

			cfg.fragmentEntry = fragmentEntry->data.string;
		}

		koml_symbol_t* fragmentMedium;
		if (!loadKOMLSymbol(table, label + ":fragmentMedium", KOML_TYPE_STRING, fragmentMedium)) {
			return false;
		}

		cfg.fragmentMedium = fragmentMedium->data.string;

		koml_symbol_t* description;
		if (!loadKOMLSymbol(table, label + ":description", KOML_TYPE_STRING, description)) {
			return false;
		}

		if (!loadShaderDescriptionConfig(table, std::string(description->data.string), cfg.description)) {
			return false;
		}

		return true;
	}

	bool parseShaderConfig(ShaderConfig& cfg, ShaderConfigParsed& parsed) {
		ShaderConfigParsed p = {};
		p.name = cfg.name;
		p.vertexPath = cfg.vertexPath;
		p.vertexEntry = cfg.vertexEntry;
		if (strcmp(cfg.vertexMedium, "glsl") == 0) {
			p.vertexMedium = KGFX_MEDIUM_GLSL;
		} else if (strcmp(cfg.vertexMedium, "hlsl") == 0) {
			p.vertexMedium = KGFX_MEDIUM_HLSL;
		} else if (strcmp(cfg.vertexMedium, "spirv") == 0) {
			p.vertexMedium = KGFX_MEDIUM_SPIRV;
		} else if (strcmp(cfg.vertexMedium, "msl") == 0) {
			p.vertexMedium = KGFX_MEDIUM_MSL;
		} else {
			std::cerr << "Failed to parse " << cfg.vertexMedium << " as a shader medium" << std::endl;
			return false;
		}

		p.fragmentPath = cfg.fragmentPath;
		p.fragmentEntry = cfg.fragmentEntry;
		if (strcmp(cfg.fragmentMedium, "glsl") == 0) {
			p.fragmentMedium = KGFX_MEDIUM_GLSL;
		} else if (strcmp(cfg.fragmentMedium, "hlsl") == 0) {
			p.fragmentMedium = KGFX_MEDIUM_HLSL;
		} else if (strcmp(cfg.fragmentMedium, "spirv") == 0) {
			p.fragmentMedium = KGFX_MEDIUM_SPIRV;
		} else if (strcmp(cfg.fragmentMedium, "msl") == 0) {
			p.fragmentMedium = KGFX_MEDIUM_MSL;
		} else {
			std::cerr << "Failed to parse " << cfg.fragmentMedium << " as a shader medium" << std::endl;
			return false;
		}

		ShaderDescriptionConfigParsed desc = {};
		if (strcmp(cfg.description.cullMode, "none") == 0) {
			desc.cullMode = KGFX_CULLMODE_NONE;
		} else if (strcmp(cfg.description.cullMode, "front") == 0) {
			desc.cullMode = KGFX_CULLMODE_FRONT;
		} else if (strcmp(cfg.description.cullMode, "back") == 0) {
			desc.cullMode = KGFX_CULLMODE_BACK;
		} else {
			std::cerr << "Failed to parse " << cfg.description.cullMode << " as a cull mode" << std::endl;
			return false;
		}

		if (strcmp(cfg.description.frontFace, "ccw") == 0) {
			desc.frontFace = KGFX_FRONTFACE_CCW;
		} else if (strcmp(cfg.description.frontFace, "cw") == 0) {
			desc.frontFace = KGFX_FRONTFACE_CW;
		} else {
			std::cerr << "Failed to parse " << cfg.description.frontFace << " as a front face" << std::endl;
			return false;
		}

		if (strcmp(cfg.description.fillMode, "solid") == 0) {
			desc.fillMode = KGFX_FILLMODE_SOLID;
		} else if (strcmp(cfg.description.fillMode, "lines") == 0) {
			desc.fillMode = KGFX_FILLMODE_LINES;
		} else {
			std::cerr << "Failed to parse " << cfg.description.fillMode << " as a fill mode" << std::endl;
			return false;
		}

		if (strcmp(cfg.description.topology, "points") == 0) {
			desc.topology = KGFX_TOPOLOGY_POINTS;
		} else if (strcmp(cfg.description.topology, "lines") == 0) {
			desc.topology = KGFX_TOPOLOGY_LINES;
		} else if (strcmp(cfg.description.topology, "triangles") == 0) {
			desc.topology = KGFX_TOPOLOGY_TRIANGLES;
		} else {
			std::cerr << "Failed to parse " << cfg.description.topology << " as a topology" << std::endl;
			return false;
		}

		desc.bindings.resize(cfg.description.bindings.size());
		for (u32 i = 0; i < desc.bindings.size(); ++i) {
			ShaderDescriptionBindingConfigParsed& binding = desc.bindings[i];
			ShaderDescriptionBindingConfig& bindingCfg = cfg.description.bindings[i];

			if (strcmp(bindingCfg.inputRate, "vertex") == 0) {
				binding.inputRate = KGFX_VERTEX_INPUT_RATE_VERTEX;
			} else if (strcmp(bindingCfg.inputRate, "instance") == 0) {
				binding.inputRate = KGFX_VERTEX_INPUT_RATE_INSTANCE;
			} else {
				std::cerr << "Failed to parse " << bindingCfg.inputRate << " as an input rate" << std::endl;
				return false;
			}

			binding.attributes.resize(bindingCfg.attributes.size());
			for (u32 j = 0; j < binding.attributes.size(); ++j) {
				ShaderDescriptionAttributeConfigParsed& attr = binding.attributes[j];
				ShaderDescriptionAttributeConfig& attrCfg = bindingCfg.attributes[j];

				attr.semanticName = attrCfg.semanticName;
				attr.semanticIndex = attrCfg.semanticIndex;

				if (strcmp(attrCfg.type, "float") == 0) {
					attr.type = KGFX_DATATYPE_FLOAT;
				} else if (strcmp(attrCfg.type, "float2") == 0) {
					attr.type = KGFX_DATATYPE_FLOAT2;
				} else if (strcmp(attrCfg.type, "float3") == 0) {
					attr.type = KGFX_DATATYPE_FLOAT3;
				} else if (strcmp(attrCfg.type, "float4") == 0) {
					attr.type = KGFX_DATATYPE_FLOAT4;
				} else if (strcmp(attrCfg.type, "int") == 0) {
					attr.type = KGFX_DATATYPE_INT;
				} else if (strcmp(attrCfg.type, "int2") == 0) {
					attr.type = KGFX_DATATYPE_INT2;
				} else if (strcmp(attrCfg.type, "int3") == 0) {
					attr.type = KGFX_DATATYPE_INT3;
				} else if (strcmp(attrCfg.type, "int4") == 0) {
					attr.type = KGFX_DATATYPE_INT4;
				} else if (strcmp(attrCfg.type, "uint") == 0) {
					attr.type = KGFX_DATATYPE_UINT;
				} else if (strcmp(attrCfg.type, "uint2") == 0) {
					attr.type = KGFX_DATATYPE_UINT2;
				} else if (strcmp(attrCfg.type, "uint3") == 0) {
					attr.type = KGFX_DATATYPE_UINT3;
				} else if (strcmp(attrCfg.type, "uint4") == 0) {
					attr.type = KGFX_DATATYPE_UINT4;
				} else if (strcmp(attrCfg.type, "mat2") == 0) {
					attr.type = KGFX_DATATYPE_MAT2;
				} else if (strcmp(attrCfg.type, "mat3") == 0) {
					attr.type = KGFX_DATATYPE_MAT3;
				} else if (strcmp(attrCfg.type, "mat4") == 0) {
					attr.type = KGFX_DATATYPE_MAT4;
				} else if (strcmp(attrCfg.type, "texture") == 0) {
					attr.type = KGFX_DATATYPE_TEXTURE_SAMPLER;
				} else {
					std::cerr << "Failed to parse " << attrCfg.type << " as a type" << std::endl;
					return false;
				}

				attr.location = attrCfg.location;
			}

			if (strcmp(bindingCfg.bindpoint, "vertex") == 0) {
				binding.bindpoint = KGFX_BINDPOINT_VERTEX;
			} else if (strcmp(bindingCfg.bindpoint, "fragment") == 0) {
				binding.bindpoint = KGFX_BINDPOINT_FRAGMENT;
			} else if (strcmp(bindingCfg.bindpoint, "geometry") == 0) {
				binding.bindpoint = KGFX_BINDPOINT_GEOMETRY;
			} else if (strcmp(bindingCfg.bindpoint, "compute") == 0) {
				binding.bindpoint = KGFX_BINDPOINT_COMPUTE;
			} else {
				std::cerr << "Failed to parse " << bindingCfg.bindpoint << " as a bindpoint" << std::endl;
				return false;
			}

			binding.binding = bindingCfg.binding;
		}

		desc.descriptorSets.resize(cfg.description.descriptorSets.size());
		for (u32 i = 0; i < desc.descriptorSets.size(); ++i) {
			KGFXdescriptorsetdesc& descSet = desc.descriptorSets[i];
			ShaderDescriptionDescriptorSetConfig& descCfg = cfg.description.descriptorSets[i];

			if (strcmp(descCfg.bindpoint, "vertex") == 0) {
				descSet.bindpoint = KGFX_BINDPOINT_VERTEX;
			} else if (strcmp(descCfg.bindpoint, "fragment") == 0) {
				descSet.bindpoint = KGFX_BINDPOINT_FRAGMENT;
			} else if (strcmp(descCfg.bindpoint, "geometry") == 0) {
				descSet.bindpoint = KGFX_BINDPOINT_GEOMETRY;
			} else if (strcmp(descCfg.bindpoint, "compute") == 0) {
				descSet.bindpoint = KGFX_BINDPOINT_COMPUTE;
			} else {
				std::cerr << "Failed to parse " << descCfg.bindpoint << " as a bindpoint" << std::endl;
				return false;
			}

			descSet.binding = descCfg.binding;

			if (strcmp(descCfg.usage, "uniform") == 0) {
				descSet.usage = KGFX_DESCRIPTOR_USAGE_UNIFORM_BUFFER;
			} else if (strcmp(descCfg.usage, "texture") == 0) {
				descSet.usage = KGFX_DESCRIPTOR_USAGE_TEXTURE;
			} else {
				std::cerr << "Failed to parse " << descCfg.usage << " as a descriptor set usage" << std::endl;
				return false;
			}

			descSet.size = descCfg.size;
		}

		p.description = desc;
		parsed = p;
		return true;
	}

	bool loadShader(koml_table_t& table, std::string const& label, Shader& shader) {
		ShaderConfig cfg;
		if (!loadShaderConfig(table, label, cfg)) {
			return false;
		}

		ShaderConfigParsed parsed;
		if (!parseShaderConfig(cfg, parsed)) {
			return false;
		}

		KGFXshaderdesc shaderDesc = {};
		shaderDesc.entryName = parsed.vertexEntry.c_str();
		{
			FILE* fp = fopen((config.shaderDirectory + "/" + parsed.vertexPath).c_str(), "rb");
			if (fp == nullptr) {
				std::cerr << "Failed to open " << config.shaderDirectory + "/" + parsed.vertexPath << std::endl;
				return false;
			}

			fseek(fp, 0, SEEK_END);
			shaderDesc.size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			shaderDesc.pData = new u8[shaderDesc.size];
			if (fread(const_cast<void*>(shaderDesc.pData), 1, shaderDesc.size, fp) != shaderDesc.size) {
				std::cerr << "Failed to read " << config.shaderDirectory + "/" + parsed.vertexPath << std::endl;
				delete[] shaderDesc.pData;
				return false;
			}

			fclose(fp);
		}
		shaderDesc.type = KGFX_SHADERTYPE_VERTEX;
		shaderDesc.medium = parsed.vertexMedium;

		KGFXshader vertexShader = kgfxCreateShader(context, shaderDesc);
		if (vertexShader == nullptr) {
			std::cerr << "Failed to create vertex shader" << std::endl;
			delete[] shaderDesc.pData;
			return false;
		}

		shaderDesc.entryName = parsed.fragmentEntry.c_str();
		if (parsed.fragmentPath != parsed.vertexPath) {
			delete[] shaderDesc.pData;

			FILE* fp = fopen((config.shaderDirectory + "/" + parsed.fragmentPath).c_str(), "rb");
			if (fp == nullptr) {
				std::cerr << "Failed to open " << config.shaderDirectory + "/" + parsed.fragmentPath << std::endl;
				return false;
			}

			fseek(fp, 0, SEEK_END);
			shaderDesc.size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			shaderDesc.pData = new u8[shaderDesc.size];
			if (fread(const_cast<void*>(shaderDesc.pData), 1, shaderDesc.size, fp) != shaderDesc.size) {
				std::cerr << "Failed to read " << config.shaderDirectory + "/" + parsed.fragmentPath << std::endl;
				return false;
			}

			fclose(fp);
		}
		shaderDesc.type = KGFX_SHADERTYPE_FRAGMENT;
		shaderDesc.medium = parsed.fragmentMedium;

		KGFXshader fragmentShader = kgfxCreateShader(context, shaderDesc);
		delete[] shaderDesc.pData;
		if (fragmentShader == nullptr) {
			std::cerr << "Failed to create fragment shader" << std::endl;
			return false;
		}

		KGFXshader shaders[2] = { vertexShader, fragmentShader };
		
		KGFXpipelinedesc pipelineDesc = {};
		pipelineDesc.cullMode = parsed.description.cullMode;
		pipelineDesc.frontFace = parsed.description.frontFace;
		pipelineDesc.fillMode = parsed.description.fillMode;
		pipelineDesc.topology = parsed.description.topology;
		pipelineDesc.shaderCount = 2;
		pipelineDesc.pShaders = shaders;
		pipelineDesc.framebuffer = nullptr;

		std::vector<KGFXpipelinebinding> bindings(parsed.description.bindings.size());
		for (u32 i = 0; i < bindings.size(); ++i) {
			ShaderDescriptionBindingConfigParsed& binding = parsed.description.bindings[i];
			KGFXpipelinebinding& b = bindings[i];
			b.inputRate = binding.inputRate;
			b.attributeCount = binding.attributes.size();
			b.pAttributes = binding.attributes.data();
			b.bindpoint = binding.bindpoint;
			b.binding = binding.binding;
		}

		pipelineDesc.layout.pDescriptorSets = parsed.description.descriptorSets.data();
		pipelineDesc.layout.descriptorSetCount = parsed.description.descriptorSets.size();
		pipelineDesc.layout.pBindings = bindings.data();
		pipelineDesc.layout.bindingCount = bindings.size();

		KGFXpipeline pipeline = kgfxCreatePipeline(context, pipelineDesc);
		if (pipeline == nullptr) {
			std::cerr << "Failed to create pipeline" << std::endl;
			return false;
		}

		addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyPipeline), pipeline);
		kgfxDestroyShader(context, vertexShader);
		kgfxDestroyShader(context, fragmentShader);

		shader.pipeline = pipeline;
		return true;
	}

	bool loadShaders() {
		koml_table_t& table = config.shaderKOML;
		Shader shader;
		if (!loadShader(table, "shaders.default", shader)) {
			std::cerr << "Failed to load default shader" << std::endl;
			return false;
		}

		shaderStorage.shaders["default"] = shader;
		defaultPipeline = shader.pipeline;
		return true;
	}

	Context() = default;
	Context(kwnd_window_t* w) {
		if (init(w) != 0) {
			deinit();
		}
	}

	~Context() {
		deinit();
	}

	static int loadConfig(koml_table_t& table, std::string const& path) {
		FILE* fp = fopen(path.c_str(), "rb");
		if (fp == nullptr) {
			return 1;
		}

		fseek(fp, 0, SEEK_END);
		usize size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char* buffer = new char[size + 1];
		if (fread(buffer, 1, size, fp) != size) {
			std::cerr << "Failed to read " << path << std::endl;
			return 1;
		}

		buffer[size] = '\0';
		fclose(fp);

		if (koml_table_load(&table, buffer, size) != 0) {
			std::cerr << "Failed to load " << path << std::endl;
			return 1;
		}

		delete[] buffer;
		return 0;
	}

	static bool isKey(int key) {
		if (key < 0 || key >= keys.size()) {
			return false;
		}

		return keys[key];
	}

	static bool isKeyDown(int key) {
		if (key < 0 || key >= keys.size()) {
			return false;
		}

		return keys[key] && !prevKeys[key];
	}

	static bool isKeyUp(int key) {
		if (key < 0 || key >= keys.size()) {
			return false;
		}

		return !keys[key] && prevKeys[key];
	}

	static bool isMouseButton(int button) {
		if (button < 0 || button > mouseButtons.size()) {
			return false;
		}

		return mouseButtons[button];
	}

	static bool isMouseButtonDown(int button) {
		if (button < 0 || button > mouseButtons.size()) {
			return false;
		}

		return mouseButtons[button] && !prevMouseButtons[button];
	}

	static bool isMouseButtonUp(int button) {
		if (button < 0 || button > mouseButtons.size()) {
			return false;
		}

		return !mouseButtons[button] && prevMouseButtons[button];
	}

	static double getMouseX() {
		return mouseX;
	}

	static double getMouseY() {
		return mouseY;
	}

	static double getMouseZ() {
		return mouseZ;
	}

	static double getMouseDX() {
		return deltaMouseX;
	}

	static double getMouseDY() {
		return deltaMouseY;
	}

	static double getMouseDZ() {
		return deltaMouseZ;
	}
};

struct Player : public GameObject {
	float speed = 1.0f;
	float mouseSensitivityX = 1.0f;
	float mouseSensitivityY = 1.0f;
	float health = 100.0f;
	Camera& camera;

	Player(Camera& c) : camera(c) {}

	void update() {
		vec3 movement = { 0, 0, 0 };
		vec3 forward;
		vec3 right;
		vec3 up;

		forward[0] = -sinf(camera.rotation[1]);
		forward[1] = -cosf(camera.rotation[0]);
		forward[2] = -cosf(camera.rotation[1]);

		up[0] = 0;
		up[1] = 1;
		up[2] = 0;

		vec3_mul_cross(right, forward, up);

		if (Context::isKey(KWND_KEYCODE_W)) {
			movement[0] -= forward[0] * speed;
			movement[2] -= forward[2] * speed;
		}

		if (Context::isKey(KWND_KEYCODE_S)) {
			movement[0] += forward[0] * speed;
			movement[2] += forward[2] * speed;
		}

		if (Context::isKey(KWND_KEYCODE_A)) {
			movement[0] -= right[0] * speed;
			movement[2] -= right[2] * speed;
		}

		if (Context::isKey(KWND_KEYCODE_D)) {
			movement[0] += right[0] * speed;
			movement[2] += right[2] * speed;
		}

		if (Context::isKey(KWND_KEYCODE_SPACE)) {
			movement[1] += speed;
		}

		if (Context::isKey(KWND_KEYCODE_SHIFT)) {
			movement[1] -= speed;
		}

		position[0] += movement[0] * Context::deltaTime;
		position[1] += movement[1] * Context::deltaTime;
		position[2] += movement[2] * Context::deltaTime;

		if (Context::isKey(KWND_KEYCODE_LEFT)) {
			rotation[1] -= 1.0f * Context::deltaTime;
		}

		if (Context::isKey(KWND_KEYCODE_RIGHT)) {
			rotation[1] += 1.0f * Context::deltaTime;
		}

		if (Context::isKey(KWND_KEYCODE_UP)) {
			rotation[0] += 1.0f * Context::deltaTime;
		}

		if (Context::isKey(KWND_KEYCODE_DOWN)) {
			rotation[0] -= 1.0f * Context::deltaTime;
		}

		double dx = Context::getMouseDX();
		double dy = Context::getMouseDY();

		rotation[1] += dx * mouseSensitivityX / 1000;
		rotation[0] -= dy * mouseSensitivityY / 1000;

		camera.position[0] = position[0];
		camera.position[1] = position[1];
		camera.position[2] = position[2];

		camera.rotation[0] = rotation[0];
		camera.rotation[1] = rotation[1];
		camera.rotation[2] = rotation[2];
	}
};

int main() {
	std::filesystem::path originalPath = std::filesystem::current_path();

	std::filesystem::current_path(originalPath);
	if (!std::filesystem::is_regular_file("config.koml")) {
		std::cerr << "Failed to find config.koml" << std::endl << "Current path is " << std::filesystem::current_path() << std::endl;
		return -1;
	}

	kwnd_window_t window;
	if (kwnd_window_create(&window, "test", 800, 600) != KWND_SUCCESS) {
		std::cerr << "Failed to create window" << std::endl;
		return -1;
	}

	Context context;
	context.windowWidth = window.width;
	context.windowHeight = window.height;
	context.isWindowed = false;
	context.camera.aspectRatio = static_cast<float>(window.width) / static_cast<float>(window.height);
	if (context.init(&window) != 0) {
		kwnd_window_destroy(&window);
		std::cerr << "Failed to initialize context" << std::endl;
		return 0;
	}

	Vertex vertices[3] = {
		{ { 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
	};

	KGFXbufferdesc bufferDesc = {};
	bufferDesc.location = KGFX_BUFFER_LOCATION_GPU;
	bufferDesc.usage = KGFX_BUFFER_USAGE_VERTEX_BUFFER;
	bufferDesc.size = sizeof(vertices);
	bufferDesc.pData = vertices;

	KGFXbuffer vertexBuffer = kgfxCreateBuffer(context.context, bufferDesc);
	if (vertexBuffer == nullptr) {
		std::cerr << "Failed to create vertex buffer" << std::endl;
		kwnd_window_destroy(&window);
		return 0;
	}

	context.addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyBuffer), vertexBuffer);

	bufferDesc.location = KGFX_BUFFER_LOCATION_CPU;
	bufferDesc.usage = KGFX_BUFFER_USAGE_UNIFORM_BUFFER;
	bufferDesc.size = sizeof(mat4x4) * 3;
	bufferDesc.pData = nullptr;

	KGFXbuffer uniformBuffer = kgfxCreateBuffer(context.context, bufferDesc);
	if (uniformBuffer == nullptr) {
		std::cerr << "Failed to create uniform buffer" << std::endl;
		kwnd_window_destroy(&window);
		return 0;
	}

	context.addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyBuffer), uniformBuffer);

	MatrixUniform* matrixMapped = static_cast<MatrixUniform*>(kgfxBufferMap(context.context, uniformBuffer));
	if (matrixMapped == nullptr) {
		std::cerr << "Failed to map uniform buffer" << std::endl;
		kwnd_window_destroy(&window);
		return 0;
	}

	Renderable renderable;
	renderable.vertexBuffer = vertexBuffer;
	renderable.vertexCount = 3;
	renderable.indexBuffer = nullptr;
	renderable.indexCount = 0;
	renderable.pipeline = context.defaultPipeline;

	GameObjectRenderable triangle = GameObjectRenderable(renderable);
	triangle.position[0] = 0;
	triangle.position[1] = 0;
	triangle.position[2] = 5;
	context.addGameObjectRenderable(triangle);

	Player player = Player(context.camera);
	context.addGameObject(player);

	kwnd_show_window(&window);

	while (!window.closed) {
		context.update();
		kwnd_update_window(&window);

		Context::prevKeys = Context::keys;
		Context::prevMouseButtons = Context::mouseButtons;

		kwnd_event_t event;
		while (kwnd_poll_event(&window, &event) == KWND_SUCCESS) {
			if (event.type == KWND_EVENT_CLOSE) {
				window.closed = true;
				break;
			} else if (event.type == KWND_EVENT_KEY) {
				Context::keys[event.data.key.keycode] = event.data.key.pressed;
			} else if (event.type == KWND_EVENT_MOUSE_MOVE) {
				Context::deltaMouseX = event.data.mouse_pos.x - Context::mouseX;
				Context::deltaMouseY = event.data.mouse_pos.y - Context::mouseY;
				Context::mouseX = event.data.mouse_pos.x;
				Context::mouseY = event.data.mouse_pos.y;
			} else if (event.type == KWND_EVENT_SCROLL) {
				Context::deltaMouseZ = event.data.scroll.vertical - Context::mouseZ;
				Context::mouseZ = event.data.scroll.vertical;
			} else if (event.type == KWND_EVENT_MOUSE_BUTTON) {
				Context::mouseButtons[event.data.mouse_button.button] = event.data.mouse_button.pressed;
			}
		}

		if (Context::isKey(KWND_KEYCODE_LEFT_CONTROL) && Context::isKeyDown(KWND_KEYCODE_Q)) {
			break;
		}

		if (Context::isKeyDown(KWND_KEYCODE_F11)) {
			context.isWindowed = !context.isWindowed;

			if (context.isWindowed) {
				kwnd_disable_window_decoration(&window);
				kwnd_resize_window(&window, 1920, 1080);
				kwnd_position_window(&window, 0, 0);
			} else {
				kwnd_enable_window_decoration(&window);
				kwnd_resize_window(&window, 800, 600);
				kwnd_position_window(&window, 0, 0);
			}
		}

		if (Context::isKey(KWND_KEYCODE_LEFT_CONTROL) && Context::isKeyDown(KWND_KEYCODE_R)) {
			if (!context.reloadShaders()) {
				break;
			}
			triangle.renderable.pipeline = context.defaultPipeline;
		}

		mat4x4_identity(matrixMapped->model);
		mat4x4_identity(matrixMapped->view);
		mat4x4_identity(matrixMapped->projection);

		mat4x4_rotate_X(matrixMapped->view, matrixMapped->view, -context.camera.rotation[0]);
		mat4x4_rotate_Y(matrixMapped->view, matrixMapped->view, -context.camera.rotation[1]);
		mat4x4_rotate_Z(matrixMapped->view, matrixMapped->view, -context.camera.rotation[2]);
		mat4x4_translate_in_place(matrixMapped->view, context.camera.position[0], -context.camera.position[1], context.camera.position[2]);

		mat4x4_ortho(matrixMapped->projection, -context.camera.aspectRatio, context.camera.aspectRatio, -1, 1, -100, 100);
		//mat4x4_perspective(matrixMapped->projection, context.camera.fov * (M_PI / 180.0f), context.camera.aspectRatio, context.camera.nearPlane, context.camera.farPlane);
		//matrixMapped->projection[1][1] *= -1;

		kgfxCommandReset(context.context, context.commandList);

		for (Renderable* r : context.renderables) {
			r->calculateMVP(matrixMapped->model);
			r->addSetupCommandsToList(context.context, context.commandList);
			kgfxCommandBindDescriptorSetBuffer(context.context, context.commandList, uniformBuffer, 0);
			r->addDrawCommandsToList(context.context, context.commandList);
		}

		kgfxCommandListSubmit(context.context, context.commandList);
		kgfxPresent(context.context);
	}
	
	kwnd_window_destroy(&window);
	return 0;
}
