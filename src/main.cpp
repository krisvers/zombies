#include <kgfx/kgfx.h>
#include <kgfx_gh/kgfx_gh.h>
#include <GLFW/glfw3.h>
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

struct Shader {
	KGFXpipeline pipeline = nullptr;
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

struct ShaderDescriptionConfig {
	char* cullMode;
	char* frontFace;
	char* fillMode;
	char* topology;
	std::vector<ShaderDescriptionBindingConfig> bindings;
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
};

struct ShaderConfigParsed {
	char* name;
	char* vertexPath;
	char* vertexEntry;
	KGFXshadermedium vertexMedium;
	char* fragmentPath;
	char* fragmentEntry;
	KGFXshadermedium fragmentMedium;

	ShaderDescriptionConfigParsed description;
};

struct ShaderStorage {
	std::unordered_map<std::string, Shader> shaders;
};

struct Context {
	KGFXcontext context = nullptr;
	KGFXcommandlist commandList = nullptr;
	KGFXpipeline basicPipeline = nullptr;
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

	static inline std::array<bool, GLFW_KEY_LAST - 1> keys;
	static inline std::array<bool, GLFW_KEY_LAST - 1> prevKeys;
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST - 1> mouseButtons;
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST - 1> prevMouseButtons;

	static inline double mouseX = 0;
	static inline double mouseY = 0;
	static inline double mouseZ = 0;
	static inline double prevMouseX = 0;
	static inline double prevMouseY = 0;
	static inline double prevMouseZ = 0;
	static inline double deltaMouseX = 0;
	static inline double deltaMouseY = 0;
	static inline double deltaMouseZ = 0;

	static inline float deltaTime = 0;
	static inline float lastTime;

	bool inited = false;
	bool deinited = false;

	bool isWindowed = false;
	u32 windowWidth = 0;
	u32 windowHeight = 0;

	int init(GLFWwindow* window) {
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

		if (!loadShaders()) {
			std::cerr << "Failed to load shaders" << std::endl;
			return 1;
		}

		std::cerr << "Not finished" << std::endl;
		return 1;

		if (kgfxCreateContext(KGFX_ANY_VERSION, kgfxWindowFromGLFW(window), &context) != KGFX_SUCCESS) {
			std::cerr << "Failed to create KGFX context" << std::endl;
			return 1;
		}

		commandList = kgfxCreateCommandList(context);
		if (commandList == nullptr) {
			std::cerr << "Failed to create command list" << std::endl;
			return 1;
		}

		addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyCommandList), commandList);

		glfwSetKeyCallback(window, keyCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetScrollCallback(window, scrollCallback);
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

		deltaTime = static_cast<float>(glfwGetTime() - lastTime);
		lastTime = static_cast<float>(glfwGetTime());

		for (auto& object : gameObjects) {
			object->update();
		}
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

		cfg.attributes.reserve(attributes->data.array.length);
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

		desc.bindings.reserve(bindings->data.array.length);
		for (u32 i = 0; i < bindings->data.array.length; ++i) {
			if (!loadShaderBindingConfig(table, std::string(bindings->data.array.elements.string[i]), desc.bindings[i])) {
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

	bool stringToShaderMedium(std::string& string, KGFXshadermedium& medium) {
		if (string == "spirv") {
			medium = KGFX_MEDIUM_SPIRV;
		} else if (string == "glsl") {
			medium = KGFX_MEDIUM_GLSL;
		} else if (string == "hlsl") {
			medium = KGFX_MEDIUM_HLSL;
		} else if (string == "msl") {
			medium = KGFX_MEDIUM_MSL;
		} else {
			return false;
		}

		return true;
	}

	bool parseShaderConfig(ShaderConfig& cfg, ShaderConfigParsed& parsed) {
		ShaderConfigParsed p = {};

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
		shaderDesc.entryName = cfg.vertexEntry;
		shaderDesc.type = KGFX_SHADERTYPE_VERTEX;
		
		KGFXpipelinedesc pipelineDesc = {};

		return true;
	}

	bool loadShaders() {
		koml_table_t& table = config.shaderKOML;
		Shader shader;
		if (!loadShader(table, "shaders.default", shader)) {
			std::cerr << "Failed to load default shader" << std::endl;
			return false;
		}

		return true;
	}

	Context() = default;
	Context(GLFWwindow* w) {
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

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key < 0 || key >= GLFW_KEY_LAST) {
			return;
		}

		if (action == GLFW_PRESS) {
			keys[key] = true;
		} else if (action == GLFW_RELEASE) {
			keys[key] = false;
		}
	}

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) {
			return;
		}

		if (action == GLFW_PRESS) {
			mouseButtons[button] = true;
		} else if (action == GLFW_RELEASE) {
			mouseButtons[button] = false;
		}
	}

	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
		mouseX = xpos;
		mouseY = ypos;
	}

	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		mouseZ = -yoffset;
	}

	static bool isKey(int key) {
		if (key < 0 || key >= GLFW_KEY_LAST) {
			return false;
		}

		return keys[key];
	}

	static bool isKeyDown(int key) {
		if (key < 0 || key >= GLFW_KEY_LAST) {
			return false;
		}

		return keys[key] && !prevKeys[key];
	}

	static bool isKeyUp(int key) {
		if (key < 0 || key >= GLFW_KEY_LAST) {
			return false;
		}

		return !keys[key] && prevKeys[key];
	}

	static bool isMouseButton(int button) {
		if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) {
			return false;
		}

		return mouseButtons[button];
	}

	static bool isMouseButtonDown(int button) {
		if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) {
			return false;
		}

		return mouseButtons[button] && !prevMouseButtons[button];
	}

	static bool isMouseButtonUp(int button) {
		if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) {
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

		if (Context::isKey(GLFW_KEY_W)) {
			movement[0] -= forward[0] * speed;
			movement[2] -= forward[2] * speed;
		}

		if (Context::isKey(GLFW_KEY_S)) {
			movement[0] += forward[0] * speed;
			movement[2] += forward[2] * speed;
		}

		if (Context::isKey(GLFW_KEY_A)) {
			movement[0] -= right[0] * speed;
			movement[2] -= right[2] * speed;
		}

		if (Context::isKey(GLFW_KEY_D)) {
			movement[0] += right[0] * speed;
			movement[2] += right[2] * speed;
		}

		if (Context::isKey(GLFW_KEY_SPACE)) {
			movement[1] += speed;
		}

		if (Context::isKey(GLFW_KEY_LEFT_SHIFT)) {
			movement[1] -= speed;
		}

		position[0] += movement[0] * Context::deltaTime;
		position[1] += movement[1] * Context::deltaTime;
		position[2] += movement[2] * Context::deltaTime;

		if (Context::isKey(GLFW_KEY_LEFT)) {
			rotation[1] -= 1.0f * Context::deltaTime;
		}

		if (Context::isKey(GLFW_KEY_RIGHT)) {
			rotation[1] += 1.0f * Context::deltaTime;
		}

		if (Context::isKey(GLFW_KEY_UP)) {
			rotation[0] += 1.0f * Context::deltaTime;
		}

		if (Context::isKey(GLFW_KEY_DOWN)) {
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
	std::string originalPath = std::filesystem::current_path();

	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error: " << description << std::endl;
	});

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "zombies game", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);

	std::filesystem::current_path(originalPath);
	if (!std::filesystem::is_regular_file("config.koml")) {
		std::cerr << "Failed to find config.koml" << std::endl << "Current path is " << std::filesystem::current_path() << std::endl;
		return -1;
	}

	Context context;
	context.windowWidth = mode->width;
	context.windowHeight = mode->height;
	context.isWindowed = false;
	context.camera.aspectRatio = static_cast<float>(mode->width) / static_cast<float>(mode->height);
	if (context.init(window) != 0) {
		glfwDestroyWindow(window);
		glfwTerminate();
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
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}

	context.addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyBuffer), vertexBuffer);

	bufferDesc.location = KGFX_BUFFER_LOCATION_CPU;
	bufferDesc.usage = KGFX_BUFFER_USAGE_UNIFORM_BUFFER;
	bufferDesc.size = sizeof(mat4x4);
	bufferDesc.pData = nullptr;

	KGFXbuffer uniformBuffer = kgfxCreateBuffer(context.context, bufferDesc);
	if (uniformBuffer == nullptr) {
		std::cerr << "Failed to create uniform buffer" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}

	context.addForDestruction(reinterpret_cast<KGFXdestroyfunc>(kgfxDestroyBuffer), uniformBuffer);

	MatrixUniform* matrixMapped = static_cast<MatrixUniform*>(kgfxBufferMap(context.context, uniformBuffer));
	if (matrixMapped == nullptr) {
		std::cerr << "Failed to map uniform buffer" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}

	Renderable renderable;
	renderable.vertexBuffer = vertexBuffer;
	renderable.vertexCount = 3;
	renderable.indexBuffer = nullptr;
	renderable.indexCount = 0;
	renderable.pipeline = context.basicPipeline;

	GameObjectRenderable triangle = GameObjectRenderable(renderable);
	triangle.position[0] = 0;
	triangle.position[1] = 0;
	triangle.position[2] = -5;
	context.addGameObjectRenderable(triangle);

	Player player = Player(context.camera);
	context.addGameObject(player);

	while (!glfwWindowShouldClose(window)) {
		context.update();
		glfwPollEvents();

		if (Context::isKey(GLFW_KEY_LEFT_CONTROL) && Context::isKeyDown(GLFW_KEY_Q)) {
			break;
		}

		if (Context::isKeyDown(GLFW_KEY_F11)) {
			context.isWindowed = !context.isWindowed;

			if (context.isWindowed) {
				glfwSetWindowMonitor(window, nullptr, 10, 35, context.windowWidth, context.windowHeight, 0);
				glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
			} else {
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
				glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
			}
		}

		mat4x4_identity(matrixMapped->model);
		mat4x4_identity(matrixMapped->view);
		mat4x4_identity(matrixMapped->projection);

		mat4x4_rotate_X(matrixMapped->view, matrixMapped->view, -context.camera.rotation[0]);
		mat4x4_rotate_Y(matrixMapped->view, matrixMapped->view, -context.camera.rotation[1]);
		mat4x4_rotate_Z(matrixMapped->view, matrixMapped->view, -context.camera.rotation[2]);
		mat4x4_translate_in_place(matrixMapped->view, context.camera.position[0], -context.camera.position[1], context.camera.position[2]);

		mat4x4_perspective(matrixMapped->projection, context.camera.fov, context.camera.aspectRatio, context.camera.nearPlane, context.camera.farPlane);
		matrixMapped->projection[1][1] *= -1;

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
	
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
