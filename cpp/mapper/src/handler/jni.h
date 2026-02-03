/* 
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/handler/jni.h
		* Purpose: JNI initialization and helpers. Connects to JVM, provides FindClass/GetMethodID etc.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <jdk/include/jni.h>
#include <jdk/include/jvmti.h>

#include "console.h"

class jni_handler {
private:
	static inline JavaVM* s_jvm = nullptr;
	static inline JNIEnv* s_env = nullptr;
	static inline jvmtiEnv* s_jvmti = nullptr;
	static inline bool s_initialized = false;
public:
	static bool init()
	{
		if (s_initialized)
			return true;

		jsize count = 0;
		if (JNI_GetCreatedJavaVMs(&s_jvm, 1, &count) != JNI_OK || count == 0)
		{
			LOG(("ERROR"), ("Failed to get Java VM"));
			return false;
		}

		jint res = s_jvm->GetEnv((void**)&s_env, JNI_VERSION_1_8);
		if (res == JNI_EDETACHED)
		{
			res = s_jvm->AttachCurrentThreadAsDaemon((void**)&s_env, nullptr);
		}

		if (res != JNI_OK || s_env == nullptr)
		{
			LOG(("ERROR"), ("Failed to get JNI environment"));
			return false;
		}

		res = s_jvm->GetEnv((void**)&s_jvmti, JVMTI_VERSION_1_2);
		if (res != JNI_OK)
		{
			LOG(("WARNING"), ("Failed to get JVMTI environment (some features may not work)"));
		}

		s_initialized = true;

		LOG(("INFO"), ("JNI initialized successfully"));

		return true;
	}

	static void cleanup()
	{
		if (!s_initialized || !s_jvm)
			return;

		if (s_env)
		{
			s_jvm->DetachCurrentThread();
			s_env = nullptr;
		}

		s_jvm = nullptr;
		s_initialized = false;

		LOG(("INFO"), ("JNI cleaned up"));
	}

	static JNIEnv* get_env()
	{
		if (!s_initialized || !s_jvm)
			return nullptr;

		if (s_env)
			return s_env;

		jint res = s_jvm->GetEnv((void**)&s_env, JNI_VERSION_1_8);
		if (res == JNI_EDETACHED)
		{
			res = s_jvm->AttachCurrentThreadAsDaemon((void**)&s_env, nullptr);
		}

		return (res == JNI_OK) ? s_env : nullptr;
	}

	static JavaVM* get_jvm()
	{
		return s_jvm;
	}

	static jclass find_class(const char* class_name)
	{
		JNIEnv* env = get_env();
		if (!env)
			return nullptr;

		return env->FindClass(class_name);
	}

	static jclass find_class(const std::string& class_name)
	{
		return find_class(class_name.c_str());
	}

	static jmethodID get_method_id(jclass clazz, const char* method_name, const char* signature)
	{
		JNIEnv* env = get_env();
		if (!env || !clazz)
			return nullptr;

		return env->GetMethodID(clazz, method_name, signature);
	}

	static jmethodID get_method_id(jclass clazz, const std::string& method_name, const std::string& signature)
	{
		return get_method_id(clazz, method_name.c_str(), signature.c_str());
	}

	static jmethodID get_static_method_id(jclass clazz, const char* method_name, const char* signature)
	{
		JNIEnv* env = get_env();
		if (!env || !clazz)
			return nullptr;

		return env->GetStaticMethodID(clazz, method_name, signature);
	}

	static jmethodID get_static_method_id(jclass clazz, const std::string& method_name, const std::string& signature)
	{
		return get_static_method_id(clazz, method_name.c_str(), signature.c_str());
	}

	static jfieldID get_field_id(jclass clazz, const char* field_name, const char* signature)
	{
		JNIEnv* env = get_env();
		if (!env || !clazz)
			return nullptr;

		return env->GetFieldID(clazz, field_name, signature);
	}

	static jfieldID get_field_id(jclass clazz, const std::string& field_name, const std::string& signature)
	{
		return get_field_id(clazz, field_name.c_str(), signature.c_str());
	}

	static jfieldID get_static_field_id(jclass clazz, const char* field_name, const char* signature)
	{
		JNIEnv* env = get_env();
		if (!env || !clazz)
			return nullptr;

		return env->GetStaticFieldID(clazz, field_name, signature);
	}

	static jfieldID get_static_field_id(jclass clazz, const std::string& field_name, const std::string& signature)
	{
		return get_static_field_id(clazz, field_name.c_str(), signature.c_str());
	}

	static bool is_initialized()
	{
		return s_initialized;
	}

	static jvmtiEnv* get_jvmti()
	{
		return s_jvmti;
	}

	static jclass find_class_by_signature(const std::string& class_name)
	{
		if (!s_jvmti || !s_env)
			return nullptr;

		jclass* loaded_classes = nullptr;
		jint loaded_classes_count = 0;

		if (s_jvmti->GetLoadedClasses(&loaded_classes_count, &loaded_classes) != JVMTI_ERROR_NONE)
		{
			return nullptr;
		}

		jclass found_class = nullptr;
		std::string target_name = class_name;
		std::replace(target_name.begin(), target_name.end(), '.', '/');

		for (jint i = 0; i < loaded_classes_count; ++i)
		{
			char* signature_buffer = nullptr;
			if (s_jvmti->GetClassSignature(loaded_classes[i], &signature_buffer, nullptr) == JVMTI_ERROR_NONE)
			{
				std::string signature = signature_buffer;
				s_jvmti->Deallocate((unsigned char*)signature_buffer);

				if (signature.length() > 2 && signature[0] == 'L' && signature.back() == ';')
				{
					signature = signature.substr(1, signature.length() - 2);
				}

				if (signature == target_name)
				{
					found_class = (jclass)s_env->NewLocalRef(loaded_classes[i]);
					break;
				}
			}
			s_env->DeleteLocalRef(loaded_classes[i]);
		}

		if (loaded_classes)
		{
			s_jvmti->Deallocate((unsigned char*)loaded_classes);
		}

		return found_class;
	}
};