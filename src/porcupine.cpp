#include <dlfcn.h>
#include <string>
#include <napi.h>

#include "pv_porcupine.h"

typedef const char *(*porcupine_status_to_string)(pv_status_t);
typedef int32_t (*porcupine_frame_length)(void);
typedef int32_t (*porcupine_sample_rate)();
typedef const char *(*porcupine_version)();
typedef pv_status_t (*porcupine_init)(const char *, int32_t, const char *const *, const float *, pv_porcupine_t **);
typedef pv_status_t (*porcupine_process)(pv_porcupine_t *, const int16_t *, int32_t *);
typedef void (*porcupine_delete)(pv_porcupine_t *);

class PorcupineNativeDetector : public Napi::ObjectWrap<PorcupineNativeDetector> {
	public:
		static Napi::Object Init(Napi::Env env, Napi::Object exports);
		PorcupineNativeDetector(const Napi::CallbackInfo& info);
		~PorcupineNativeDetector();
		Napi::Value Process(const Napi::CallbackInfo& info);
        Napi::Value GetVersion(const Napi::CallbackInfo& info);
        Napi::Value GetFrameLength(const Napi::CallbackInfo& info);
        Napi::Value GetSampleRate(const Napi::CallbackInfo& info);

	private:
		static Napi::FunctionReference constructor;
        void Cleanup();
		void* porcupine_library;
		pv_porcupine_t *porcupine;
		porcupine_status_to_string porcupine_status_to_string;
		porcupine_frame_length porcupine_frame_length;
		porcupine_sample_rate porcupine_sample_rate;
		porcupine_version porcupine_version;
		porcupine_init porcupine_init;
		porcupine_process porcupine_process;
		porcupine_delete porcupine_delete;

		int32_t frame_length;
};


/* ============================================
 Native module implementation
============================================ */

Napi::FunctionReference PorcupineNativeDetector::constructor;

Napi::Object PorcupineNativeDetector::Init(Napi::Env env, Napi::Object exports) {

	Napi::Function func = DefineClass(env, "PorcupineNativeDetector", {
    	InstanceMethod("process", &PorcupineNativeDetector::Process),
    	InstanceMethod("getVersion", &PorcupineNativeDetector::GetVersion),
    	InstanceMethod("getFrameLength", &PorcupineNativeDetector::GetFrameLength),
    	InstanceMethod("getSampleRate", &PorcupineNativeDetector::GetSampleRate)
	});

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

	exports.Set("PorcupineNativeDetector", func);

	return exports;
}

PorcupineNativeDetector::PorcupineNativeDetector(const Napi::CallbackInfo& info) : Napi::ObjectWrap<PorcupineNativeDetector>(info) {
	Napi::Env env = info.Env();
	Napi::HandleScope scope(env);

	// if ( info.Length() != 3 ) {
	// 	// throw
	// 	Napi::TypeError::New(env, "You need to provide an options object").ThrowAsJavaScriptException();
	// 	return;
	// }

    const char *library_path = info[0].As<Napi::String>().Utf8Value().c_str();
    fprintf(stdout, "Library file: %s \n", library_path);

    const char *model_path   = info[1].As<Napi::String>().Utf8Value().c_str();
    fprintf(stdout, "Model file: %s \n", model_path);

    const char *keyword_path = info[2].As<Napi::String>().Utf8Value().c_str();
    fprintf(stdout, "Keyword file: %s \n", keyword_path);

    const float sensitivity  = info[3].As<Napi::Number>().FloatValue();
    fprintf(stdout, "Sensitivity: %f \n", sensitivity);

	void *porcupine_library = dlopen(library_path, RTLD_NOW);
	if (!porcupine_library) {
	    fprintf(stderr, "failed to open library\n");
	    exit(1);
	}

	char *error;

    *(void**) (&porcupine_status_to_string) = dlsym(porcupine_library, "pv_status_to_string");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "failed to load 'pv_status_to_string' with '%s'.\n", error);
        exit(1);
    }

	*(void**) (&porcupine_frame_length) = dlsym(porcupine_library, "pv_porcupine_frame_length");
	if ((error = dlerror()) != NULL) {
	    fprintf(stderr, "failed to load 'pv_porcupine_frame_length' with '%s'.\n", error);
	    exit(1);
	}

    *(void**) (&porcupine_sample_rate) = dlsym(porcupine_library, "pv_sample_rate");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "failed to load 'pv_sample_rate' with '%s'.\n", error);
        exit(1);
    }

	*(void**) (&porcupine_version) = dlsym(porcupine_library, "pv_porcupine_version");
	if ((error = dlerror()) != NULL) {
	    fprintf(stderr, "failed to load 'pv_porcupine_version' with '%s'.\n", error);
	    exit(1);
	}

    *(void**) (&porcupine_init) = dlsym(porcupine_library, "pv_porcupine_init");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "failed to load 'pv_porcupine_init' with '%s'.\n", error);
        exit(1);
    }

    *(void**) (&porcupine_process) = dlsym(porcupine_library, "pv_porcupine_process");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "failed to load 'pv_porcupine_process' with '%s'.\n", error);
        exit(1);
    }

    *(void**) (&porcupine_delete) = dlsym(porcupine_library, "pv_porcupine_delete");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "failed to load 'pv_porcupine_delete' with '%s'.\n", error);
        exit(1);
    }

    // pv_porcupine_t *porcupine;
    pv_status_t status = porcupine_init(model_path, 1, &keyword_path, &sensitivity, &porcupine);
    if (status != PV_STATUS_SUCCESS) {
        fprintf(stderr, "'pv_porcupine_init' failed with '%s'\n", porcupine_status_to_string(status));
        exit(1);
    }

    frame_length = porcupine_frame_length();

    fprintf(stdout, "Loaded\n");
}

PorcupineNativeDetector::~PorcupineNativeDetector() {
    Cleanup();
}

void PorcupineNativeDetector::Cleanup() {
    if ( porcupine ) porcupine_delete(porcupine);
    if ( porcupine_library ) dlclose(porcupine_library);
}

Napi::Value PorcupineNativeDetector::Process(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Buffer<int16_t> buffer = info[0].As<Napi::Buffer<int16_t> >();

    int32_t keyword_index;
    int16_t *audioFrame = buffer.Data();
    pv_status_t status = porcupine_process(porcupine, audioFrame, &keyword_index);
    if (status != PV_STATUS_SUCCESS) {
        fprintf(stderr, "'pv_porcupine_process' failed with '%s'\n", porcupine_status_to_string(status));
        exit(1);
    }

    // TODO: Use keyword_index instead of boolean
    if (keyword_index != -1) {
        fprintf(stdout, "detected keyword\n");
    }

    return Napi::Number::New(env, keyword_index);
}

Napi::Value PorcupineNativeDetector::GetFrameLength(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    const int32_t frame_length = porcupine_frame_length();
    return Napi::Number::New(env, frame_length);
}

Napi::Value PorcupineNativeDetector::GetSampleRate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    const int32_t frame_length = porcupine_sample_rate();
    return Napi::Number::New(env, frame_length);
}

Napi::Value PorcupineNativeDetector::GetVersion(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    const char *version = porcupine_version();
    return Napi::String::New(env, version);
}

/* ============================================
 Native module initialization
============================================ */

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	// Register the engine
	PorcupineNativeDetector::Init(env, exports);

	return exports;
};

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)

