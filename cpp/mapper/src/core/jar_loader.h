/*
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/core/jar_loader.h
		* Purpose: Loads JAR from memory (miniz), defines classes via JNI, invokes entry point.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "miniz/miniz.h"

#include <jdk/include/jni.h>

#include "../handler/jni.h"
#include "../handler/console.h"

#include "class_parser.h"

namespace nexus
{
    class jar_loader {
    private:
        struct class_info {
            std::string name;
            std::string super_name;
            std::vector<std::string> interfaces;
            std::vector<uint8_t> bytecode;
            bool is_defined;

            class_info(const std::string& name, const std::string& super_name, const std::vector<std::string>& interfaces, const uint8_t* bytes, size_t size) : name(name), super_name(super_name), interfaces(interfaces), bytecode(bytes, bytes + size), is_defined(false) {}
        };

        std::vector<std::unique_ptr<class_info>> m_classes;
        JNIEnv* m_env;
    public:
        jar_loader() : m_env(nullptr) {}

        bool load_from_memory(const uint8_t* jar_bytes, size_t jar_size)
        {
            if (!jni_handler::is_initialized())
            {
                LOG(("ERROR"), ("JNI not initialized"));
                return false;
            }

            m_env = jni_handler::get_env();
            if (!m_env)
            {
                LOG(("ERROR"), ("Failed to get JNI environment"));
                return false;
            }

            LOG(("INFO"), ("Loading JAR from memory (%zu bytes)"), jar_size);

            mz_zip_archive archive = {};
            if (!mz_zip_reader_init_mem(&archive, jar_bytes, jar_size, 0))
            {
                LOG(("ERROR"), ("Failed to initialize JAR archive"));
                return false;
            }

            bool success = extract_classes(archive);
            mz_zip_reader_end(&archive);

            if (!success)
            {
                LOG(("ERROR"), ("Failed to extract classes from JAR"));
                return false;
            }

            if (!define_all_classes())
            {
                LOG(("ERROR"), ("Failed to define classes"));
                return false;
            }

            LOG(("SUCCESS"), ("Successfully loaded %zu classes"), m_classes.size());
            return true;
        }

        bool invoke_entry_point(const char* class_name, const char* method_name) 
        {
            if (!m_env) 
            {
                LOG(("ERROR"), ("JNI environment not available"));
                return false;
            }

            LOG(("INFO"), ("Invoking entry point: %s.%s"), class_name, method_name);

            std::string jni_class_name = class_name;
            std::replace(jni_class_name.begin(), jni_class_name.end(), '.', '/');

            jclass entry_class = m_env->FindClass(jni_class_name.c_str());
            if (!entry_class) 
            {
                m_env->ExceptionDescribe();
                m_env->ExceptionClear();
                LOG(("ERROR"), ("Entry class not found: %s"), class_name);
                return false;
            }

            jmethodID entry_method = m_env->GetStaticMethodID(entry_class, method_name, "()V");
            if (!entry_method) 
            {
                m_env->ExceptionDescribe();
                m_env->ExceptionClear();
                LOG(("ERROR"), ("Entry method not found: %s"), method_name);
                m_env->DeleteLocalRef(entry_class);
                return false;
            }

            LOG(("SUCCESS"), ("Found entry point, invoking..."));
            m_env->CallStaticVoidMethod(entry_class, entry_method);

            if (m_env->ExceptionCheck())
            {
                m_env->ExceptionDescribe();
                m_env->ExceptionClear();
                LOG(("ERROR"), ("Exception occurred during entry point invocation"));
                m_env->DeleteLocalRef(entry_class);
                return false;
            }

            m_env->DeleteLocalRef(entry_class);
            LOG(("SUCCESS"), ("Entry point invoked successfully"));
            return true;
        }

    private:
        bool extract_classes(mz_zip_archive& archive)
        {
            mz_uint file_count = mz_zip_reader_get_num_files(&archive);
            LOG(("INFO"), ("Found %u files in JAR"), file_count);

            for (mz_uint i = 0; i < file_count; i++) 
            {
                mz_zip_archive_file_stat file_stat;
                if (!mz_zip_reader_file_stat(&archive, i, &file_stat)) 
                {
                    continue;
                }

                std::string filename = file_stat.m_filename;
                if (filename.size() < 6 || filename.substr(filename.size() - 6) != ".class") 
                {
                    continue;
                }

                if (filename.find("module-info") != std::string::npos)
                {
                    continue;
                }

                size_t uncompressed_size = (size_t)file_stat.m_uncomp_size;
                std::vector<uint8_t> class_data(uncompressed_size);

                if (!mz_zip_reader_extract_to_mem(&archive, i, class_data.data(), uncompressed_size, 0))
                {
                    LOG(("WARNING"), ("Failed to extract: %s"), filename.c_str());
                    continue;
                }

                class_parser parser(class_data.data(), class_data.size());
                std::string class_name = parser.get_class_name();
                std::string super_name = parser.get_super_name();
                std::vector<std::string> interfaces = parser.get_interfaces();

                if (class_name.empty()) 
                {
                    LOG(("WARNING"), ("Invalid class: %s"), filename.c_str());
                    continue;
                }

                auto class_info_ptr = std::make_unique<class_info>(class_name, super_name, interfaces, class_data.data(), class_data.size());

                m_classes.push_back(std::move(class_info_ptr));
            }

            LOG(("INFO"), ("Extracted %zu classes"), m_classes.size());
            return !m_classes.empty();
        }

        bool define_all_classes()
        {
            LOG(("INFO"), ("Defining classes..."));

            for (auto& cls : m_classes)
            {
                if (!cls->is_defined)
                {
                    if (!define_class(cls.get())) 
                    {
                        LOG(("ERROR"), ("Failed to define class: %s"), cls->name.c_str());
                        return false;
                    }
                }
            }

            return true;
        }

        bool define_class(class_info* cls)
        {
            if (cls->is_defined)
            {
                return true;
            }

            if (!cls->super_name.empty())
            {
                class_info* super_class = find_class(cls->super_name);
                if (super_class && !super_class->is_defined) 
                {
                    if (!define_class(super_class))
                    {
                        return false;
                    }
                }
            }

            for (const auto& interface_name : cls->interfaces) 
            {
                class_info* interface_class = find_class(interface_name);
                if (interface_class && !interface_class->is_defined) 
                {
                    if (!define_class(interface_class))
                    {
                        return false;
                    }
                }
            }

            std::string jni_class_name = cls->name;
            std::replace(jni_class_name.begin(), jni_class_name.end(), '.', '/');

            jclass defined_class = m_env->DefineClass(
                jni_class_name.c_str(),
                nullptr,
                (const jbyte*)cls->bytecode.data(),
                (jsize)cls->bytecode.size()
            );

            if (!defined_class)
            {
                m_env->ExceptionDescribe();
                m_env->ExceptionClear();
                LOG(("ERROR"), ("Failed to define: %s"), cls->name.c_str());
                return false;
            }

            m_env->DeleteLocalRef(defined_class);
            cls->is_defined = true;

            LOG(("INFO"), ("Defined: %s"), cls->name.c_str());
            return true;
        }

        class_info* find_class(const std::string& name)
        {
            for (auto& cls : m_classes) 
            {
                if (cls->name == name) 
                {
                    return cls.get();
                }
            }
            return nullptr;
        }
    };

}
