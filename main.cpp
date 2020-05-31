#include "core/Timer.h"
#include "core/common.h"

class Shader {
public:
	static filesystem::path shadersDir;
	GLuint programId = 0;
	GLuint vertShaderId = 0;
	GLuint fragShaderId = 0;

	~Shader() {
		glDeleteShader(vertShaderId);
		glDeleteShader(fragShaderId);
		glDetachShader(programId, vertShaderId);
		glDetachShader(programId, fragShaderId);
		glDeleteProgram(programId);
	}

	void build(string vert_file, string frag_file) {
		vert_file = (shadersDir / vert_file).string();
		frag_file = (shadersDir / frag_file).string();
		build_shader_program(vert_file, frag_file);
	}

	bool compile_shader(const string filename, GLuint type, GLuint& shaderId) {
		shaderId = glCreateShader(type);

		ifstream reader;
		size_t code_size;
		string code;
		reader.open(filename.c_str(), ios::in);
		if (!reader.is_open()) {
			return false;
		}

		reader.seekg(0, ios::end);
		code_size = reader.tellg();
		code.resize(code_size);
		reader.seekg(0);
		reader.read(&code[0], code_size);

		reader.close();

		const char* code_chars = code.c_str();
		glShaderSource(shaderId, 1, &code_chars, NULL);
		glCompileShader(shaderId);

		GLint compile_status;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compile_status);
		if (compile_status == GL_FALSE) {
			fprintf(stderr, "Failed to compile a shader!\n");
			GLint log_length;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &log_length);
			if (log_length > 0) {
				GLsizei length;
				string err_msg;
				err_msg.resize(log_length);
				glGetShaderInfoLog(shaderId, log_length, &length, &err_msg[0]);

				fprintf(stderr, "[ ERROR ] %s\n", err_msg.c_str());
				fprintf(stderr, "%s\n", code.c_str());
			}
			exit(1);
		}
		return compile_status;
	}

	bool build_shader_program(const string& vert_shader_file, const string& frag_shader_file) {
		compile_shader(vert_shader_file, GL_VERTEX_SHADER, vertShaderId);
		bool is_succeeded = compile_shader(frag_shader_file, GL_FRAGMENT_SHADER, fragShaderId);
		if (!is_succeeded) {
			return false;
		}

		programId = glCreateProgram();
		glAttachShader(programId, vertShaderId);
		glAttachShader(programId, fragShaderId);
		glLinkProgram(programId);

		GLint link_state;
		glGetProgramiv(programId, GL_LINK_STATUS, &link_state);
		if (link_state == GL_FALSE) {
			fprintf(stderr, "Failed to link shaders!\n");

			GLint log_length;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &log_length);
			if (log_length > 0) {
				GLsizei length;
				std::string errMsg;
				errMsg.resize(log_length);
				glGetProgramInfoLog(programId, log_length, &length, &errMsg[0]);

				fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
			}
			exit(1);
		}
		glUseProgram(0);
		return link_state;
	}

	void set_uniform_value(float x, float y, const char* val_name) {
		float val[2] = { x,y };
		GLuint loc_id = glGetUniformLocation(programId, val_name);
		glUniform2fv(loc_id, 1, val);
	}

	void set_uniform_value(float val, const char* val_name) {
		GLuint loc_id = glGetUniformLocation(programId, val_name);
		glUniform1f(loc_id, val);
	}

	void set_uniform_value(int val, const char* val_name) {
		GLuint loc_id = glGetUniformLocation(programId, val_name);
		glUniform1i(loc_id, val);
	}

	void bind() {
		glUseProgram(programId);
	}

	void release() {
		glUseProgram(0);
	}
};
filesystem::path Shader::shadersDir;

class Window {
public:
	GLFWwindow* window = nullptr;
	int width;
	int height;
	float mouse[2] = { 0,0 };
	const string win_name;

	GLuint vao_id = 0;

	Window(const int width, const int height, const char* window_name)
		: width(width), height(height), win_name(window_name) {
		initialize();
	}

	virtual ~Window() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	explicit operator bool() {
		int renderBufferWidth, renderBufferHeight;
		glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
		glViewport(0, 0, renderBufferWidth, renderBufferHeight);
		// ウィンドウを閉じる必要がなければ true を返す
		return !glfwWindowShouldClose(window);
	}

	void initialize() {
		// GLFW を初期化する
		if (glfwInit() == GL_FALSE) {
			std::cerr << "Can't initialize GLFW" << std::endl;
			exit(1);
		}

#ifdef _WIN32
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#elif __APPLE__
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, win_name.c_str(), nullptr, nullptr);
		if (!window) {
			std::cerr << "Can't create GLFW window." << std::endl;
			abort();
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);
		glfwSetWindowPos(window, 100, 100);

		// OpenGL 3.x/4.xの関数をロードする (glfwMakeContextCurrentの後でないといけない)
		const int version = gladLoadGL(glfwGetProcAddress);
		if (version == 0) {
			fprintf(stderr, "Failed to load OpenGL 3.x/4.x libraries!\n");
			exit(1);
		}

		// バージョンを出力する
		printf("Vendor: %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("OpenGL: %s\n", glGetString(GL_VERSION));
		printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		glfwSetWindowUserPointer(window, this);

		//resizeの設定
		auto resize_callback = [](GLFWwindow* w, int width, int height) {
			reinterpret_cast<Window*>(glfwGetWindowUserPointer(w))->resize(width, height);
		};
		glfwSetWindowSizeCallback(window, resize_callback);

		auto mouseMoveEvent_callback = [](GLFWwindow* w, double xpos, double ypos) {
			reinterpret_cast<Window*>(glfwGetWindowUserPointer(w))->mouseMoveEvent(xpos, ypos);
		};
		glfwSetCursorPosCallback(window, mouseMoveEvent_callback);

		glGenVertexArrays(1, &vao_id);
		glBindVertexArray(vao_id);

		// OpenGLの初期設定
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	void flush() {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	void render() {
		//while (*this) {
			Shader shader;
			shader.build("render.vert", "render.frag");
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader.bind();
			shader.set_uniform_value(float(width), float(height), "resolution");
			shader.set_uniform_value(mouse[0], mouse[1], "mouse");
			glBindVertexArray(vao_id);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			shader.release();
			flush();
		//}
		while(true){}
	}

	void resize(int _width, int _height) {
		width = _width;
		height = _height;
		glfwSetWindowSize(window, _width, _height);
		int renderBufferWidth, renderBufferHeight;
		glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
		glViewport(0, 0, renderBufferWidth, renderBufferHeight);
	}

	void mouseMoveEvent(double xpos, double ypos) {
		mouse[0] = xpos;
		mouse[1] = ypos;
	}
};


int main(int argc, char** argv) {
	Shader::shadersDir = "shaders";
	Window window(640, 480, "interactive GLSL");
	window.render();
}

