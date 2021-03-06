/* 
 * File:   KeyValuePairs.h
 * Author: daniel
 *
 * Created on November 26, 2015, 4:05 PM
 */

#ifndef KEYVALUEPAIRS_H
#define	KEYVALUEPAIRS_H

#include <vector>
#include "Utility.h"

namespace ohmcomm
{
    template<typename KeyType>
    struct KeyValuePair
    {
        typedef KeyType type;
        KeyType key;
        std::string value;

        KeyValuePair()
        {
        }

        KeyValuePair(KeyType key, std::string value) : key(key), value(Utility::trim(value))
        {
        }

        int getInteger() const
        {
            return atoi(value.c_str());
        }

        const std::vector<std::string> getValues() const
        {
            std::vector<std::string> values = Utility::splitString(value, ',');
            for(std::string& val : values)
            {
                val = Utility::trim(val);
            }
            return values;
        }

        inline void fromString(const std::string& input, const char delimiter)
        {
            std::string::size_type delimPos = input.find(delimiter);
            key = toKey(input.substr(0, delimPos));
            value = Utility::trim(input.substr(delimPos + 1));
        }
    protected:

        template<typename T = type, typename = typename std::enable_if<std::is_same<T, char>::value>::type>
        inline char toKey(const std::string&& key) const
        {
            return key[0];
        }

        template<typename T = type, typename = typename std::enable_if<std::is_same<T, std::string>::value>::type>
        inline std::string toKey(const std::string&& key) const
        {
            return Utility::trim(key);
        }
    };

    template<typename KeyValueType>
    struct KeyValuePairs
    {
        std::vector<KeyValueType> fields;

        KeyValuePairs() : fields(0)
        {
        }

        template<typename keyType = typename KeyValueType::type>
        std::string& operator[](const keyType& fieldKey)
        {
            for (KeyValueType& pair : fields) {
                if (isSame(pair.key, fieldKey)) {
                    return pair.value;
                }
            }
            //if key is not found, insert new
            fields.push_back(KeyValueType(fieldKey, ""));
            return (*this)[fieldKey];
        }

        template<typename keyType = typename KeyValueType::type>
        const std::string operator[](const keyType& fieldKey) const
        {
            for (const KeyValueType& pair : fields) {
                if (isSame(pair.key, fieldKey)) {
                    return pair.value;
                }
            }
            //if key is not found, return empty string
            return "";
        }

        template<typename keyType = typename KeyValueType::type>
        const std::vector<std::string> getFieldValues(const keyType& fieldKey) const
        {
            std::vector<std::string> results;
            for (const KeyValueType& pair : fields) {
                if (isSame(pair.key, fieldKey)) {
                    const std::vector<std::string> tmp = pair.getValues();
                    for (const std::string& value : tmp) {
                        results.push_back(value);
                    }
                }
            }
            return results;
        }

        template<typename keyType = typename KeyValueType::type>
        bool hasKey(const keyType& fieldKey) const
        {
            for (const KeyValueType& pair : fields) {
                if (isSame(pair.key, fieldKey)) {
                    return true;
                }
            }
            return false;
        }

    private:
        //TODO compact form of header-fields

        inline bool isSame(const std::string& s1, const std::string& s2) const
        {
            //SIP-keys are case-insensitive
            return Utility::equalsIgnoreCase(s1, s2);
        }

        inline bool isSame(const char& c1, const char& c2) const
        {
            //SDP-keys are chars
            return c1 == c2;
        }
    };
}
#endif	/* KEYVALUEPAIRS_H */

