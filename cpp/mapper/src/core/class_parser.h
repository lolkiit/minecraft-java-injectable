/*
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/core/class_parser.h
		* Purpose: Java class file parser. Reads constant pool, class name, super class, interfaces.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

#pragma pack(push, 1)

namespace nexus
{
    class class_parser {
    public:
        class_parser(const uint8_t* data, size_t size) : m_data(data), m_size(size) {}

        std::string get_class_name()
        {
            if (!is_valid_class_file())
                return "";

            const uint8_t* ptr = m_data;
            ptr += 8;

            uint16_t cp_count = read_u2(ptr);
            ptr += 2;

            ptr = skip_constant_pool(ptr, cp_count);
            ptr += 2;

            uint16_t this_class = read_u2(ptr);
            
            return get_class_name_from_index(this_class);
        }

        std::string get_super_name()
        {
            if (!is_valid_class_file())
                return "";

            const uint8_t* ptr = m_data;
            ptr += 8;

            uint16_t cp_count = read_u2(ptr);
            ptr += 2;

            ptr = skip_constant_pool(ptr, cp_count);

            ptr += 2;
            ptr += 2;

            uint16_t super_class = read_u2(ptr);
            if (super_class == 0)
                return "";

            return get_class_name_from_index(super_class);
        }

        std::vector<std::string> get_interfaces()
        {
            std::vector<std::string> interfaces;

            if (!is_valid_class_file())
                return interfaces;

            const uint8_t* ptr = m_data;
            ptr += 8;

            uint16_t cp_count = read_u2(ptr);
            ptr += 2;

            ptr = skip_constant_pool(ptr, cp_count);

            ptr += 2;
            ptr += 2;
            ptr += 2;

            uint16_t interfaces_count = read_u2(ptr);
            ptr += 2;

            for (uint16_t i = 0; i < interfaces_count; i++)
            {
                uint16_t interface_index = read_u2(ptr);
                ptr += 2;
                interfaces.push_back(get_class_name_from_index(interface_index));
            }

            return interfaces;
        }

    private:
        const uint8_t* m_data;
        size_t m_size;

        bool is_valid_class_file() const
        {
            return m_size >= 10 && read_u4_at(0) == 0xCAFEBABE;
        }

        uint16_t read_u2(const uint8_t* ptr) const
        {
            return ((uint16_t)ptr[0] << 8) | (uint16_t)ptr[1];
        }

        uint32_t read_u4_at(size_t offset) const
        {
            return ((uint32_t)m_data[offset] << 24) | ((uint32_t)m_data[offset + 1] << 16) | ((uint32_t)m_data[offset + 2] << 8) | (uint32_t)m_data[offset + 3];
        }

        const uint8_t* skip_constant_pool(const uint8_t* ptr, uint16_t count) const
        {
            for (uint16_t i = 1; i < count; i++)
            {
                uint8_t tag = *ptr++;

                switch (tag)
                {
                    case 1: // CONSTANT_Utf8
                    {
                        uint16_t length = read_u2(ptr);
                        ptr += 2 + length;
                        break;
                    }
                    case 3: // CONSTANT_Integer
                    case 4: // CONSTANT_Float
                        ptr += 4;
                        break;
                    case 5: // CONSTANT_Long
                    case 6: // CONSTANT_Double
                        ptr += 8;
                        i++; // Long and Double take two entries
                        break;
                    case 7:  // CONSTANT_Class
                    case 8:  // CONSTANT_String
                    case 16: // CONSTANT_MethodType
                    case 19: // CONSTANT_Module
                    case 20: // CONSTANT_Package
                        ptr += 2;
                        break;
                    case 9:  // CONSTANT_Fieldref
                    case 10: // CONSTANT_Methodref
                    case 11: // CONSTANT_InterfaceMethodref
                    case 12: // CONSTANT_NameAndType
                    case 17: // CONSTANT_Dynamic
                    case 18: // CONSTANT_InvokeDynamic
                        ptr += 4;
                        break;
                    case 15: // CONSTANT_MethodHandle
                        ptr += 3;
                        break;
                    default:
                        return ptr; // Unknown tag
                }
            }
            return ptr;
        }

        std::string get_class_name_from_index(uint16_t index) const
        {
            if (index == 0)
                return "";

            const uint8_t* ptr = m_data + 10;

            const uint8_t* entry_ptr = get_cp_entry(index);
            if (!entry_ptr)
                return "";

            if (*entry_ptr != 7)
                return "";

            uint16_t name_index = read_u2(entry_ptr + 1);
            return get_utf8_from_index(name_index);
        }

        std::string get_utf8_from_index(uint16_t index) const
        {
            if (index == 0)
                return "";

            const uint8_t* entry_ptr = get_cp_entry(index);
            if (!entry_ptr)
                return "";

            if (*entry_ptr != 1)
                return "";

            uint16_t length = read_u2(entry_ptr + 1);

            std::string result((const char*)(entry_ptr + 3), length);
            
            std::replace(result.begin(), result.end(), '/', '.');

            return result;
        }

        const uint8_t* get_cp_entry(uint16_t index) const
        {
            if (index == 0 || m_size < 10)
                return nullptr;

            const uint8_t* ptr = m_data + 8;
            uint16_t cp_count = read_u2(ptr);
            ptr += 2;

            if (index >= cp_count)
                return nullptr;

            for (uint16_t i = 1; i < index; i++)
            {
                uint8_t tag = *ptr++;

                switch (tag)
                {
                    case 1: // CONSTANT_Utf8
                    {
                        uint16_t length = read_u2(ptr);
                        ptr += 2 + length;
                        break;
                    }
                    case 3: // CONSTANT_Integer
                    case 4: // CONSTANT_Float
                        ptr += 4;
                        break;
                    case 5: // CONSTANT_Long
                    case 6: // CONSTANT_Double
                        ptr += 8;
                        i++; // Long and Double take two entries
                        break;
                    case 7:  // CONSTANT_Class
                    case 8:  // CONSTANT_String
                    case 16: // CONSTANT_MethodType
                    case 19: // CONSTANT_Module
                    case 20: // CONSTANT_Package
                        ptr += 2;
                        break;
                    case 9:  // CONSTANT_Fieldref
                    case 10: // CONSTANT_Methodref
                    case 11: // CONSTANT_InterfaceMethodref
                    case 12: // CONSTANT_NameAndType
                    case 17: // CONSTANT_Dynamic
                    case 18: // CONSTANT_InvokeDynamic
                        ptr += 4;
                        break;
                    case 15: // CONSTANT_MethodHandle
                        ptr += 3;
                        break;
                    default:
                        return nullptr; // Unknown tag
                }
            }

            return ptr;
        }
    };
}

#pragma pack(pop)
