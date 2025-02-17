/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "io/IniReader.h"

#include <algorithm>
#include <utility>

#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "util/String.h"

const IniSection * IniReader::getSection(std::string_view sectionName) const {
	
	iterator iter = sections.find(sectionName);
	
	if(iter != sections.end()) {
		return &(iter->second);
	} else {
		return nullptr;
	}
	
}

size_t IniReader::getKeyCount(std::string_view sectionName) const {
	
	const IniSection * section = getSection(sectionName);
	if(section) {
		return section->size();
	}
	
	return 0;
}

std::string_view IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                                   std::string_view defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue();
}

const std::string & IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                                      const std::string & defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue();
}

int IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                      int defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue(defaultValue);
}

float IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                        float defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue(defaultValue);
}


bool IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                       bool defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue(defaultValue);
}

const IniKey * IniReader::getKey(std::string_view sectionName, std::string_view keyName) const {
	
	// Look for a section
	const IniSection * section = getSection(sectionName);
	
	// If the section was not found, return nullptr
	if(!section) {
		return nullptr;
	}
	
	// If the section has no keys, return nullptr
	if(section->empty()) {
		return nullptr;
	}
	
	// If the key is not specified, return the first ones value( to avoid breakage with legacy assets )
	if(keyName.empty()) {
		return &*section->begin();
	}
	
	return section->getKey(keyName);
}

static constexpr const std::string_view WHITESPACE = " \t\r\n";
static constexpr const std::string_view ALPHANUM = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";

bool IniReader::read(std::istream & is, bool overrideValues) {
	
	// The current section
	IniSection * section = nullptr;
	
	bool ok = true;
	
	bool readline = true;
	
	std::string str;
	
	// While lines remain to be extracted
	for(size_t line = 1; is.good(); line++) {
		
		// Get a line to process
		if(readline) {
			str.clear();
			getline(is, str);
		} else {
			readline = true;
		}
		
		size_t start = str.find_first_not_of(WHITESPACE);
		if(start == std::string::npos) {
			// Empty line (only whitespace)
			continue;
		}
		
		if(str[start] == '#'
		   || (start + 1 < str.length() && str[start] == '/' && str[start + 1] == '/')) {
			// Whole line was commented, no need to do anything with it. Continue getting the next line
			continue;
		}
		
		// Section header
		if(str[start] == '[') {
			
			size_t end = str.find(']', start + 1);
			if(end == std::string::npos) {
				LogDebug("invalid header @ line " << line << ": " << str);
				end = str.find_first_not_of(ALPHANUM, start + 1);
				if(end == std::string::npos) {
					end = str.length();
				}
			}
			
			std::string sectionName = util::toLowercase(str.substr(start + 1, end - start - 1));
			LogDebug("found section: \"" << sectionName << "\"");
			section = &sections[std::move(sectionName)];
			
			// Ignoring rest of the line, not verifying that it's only whitespace / comment
			
			continue;
		}
		
		if(!section) {
			LogWarning << "Ignoring non-empty line " << line << " outside a section: " << str;
			ok = false;
			continue;
		}
		
		size_t nameEnd = str.find_first_not_of(ALPHANUM, start);
		if(nameEnd == std::string::npos) {
			ok = false;
			LogWarning << "Missing '=' separator @ line " << line << ": " << str;
			continue;
		} else if(nameEnd == start) {
			ok = false;
			LogWarning << "Empty key name @ line " << line << ": " << str;
			continue;
		}
		
		bool quoted = false;
		
		size_t separator = str.find_first_not_of(WHITESPACE, nameEnd);
		if(separator == std::string::npos || str[separator] != '=') {
			if(separator != std::string::npos && separator + 1 < str.length()
			   && str[separator] == '"' && str[separator + 1] == '=') {
				LogDebug("found '\"=' instead of '=\"' @ line " << line << ": " << str);
				quoted = true;
			} else {
				ok = false;
				LogWarning << "Missing '=' separator @ line " << line << ": " << str;
				continue;
			}
		}
		
		size_t valueStart = str.find_first_not_of(WHITESPACE, separator + 1);
		if(valueStart == std::string::npos) {
			// Empty value.
			if(overrideValues) {
				section->setKey(str.substr(start, nameEnd - start), std::string());
			} else {
				section->addKey(str.substr(start, nameEnd - start), std::string());
			}
			continue;
		}
		
		std::string key = str.substr(start, nameEnd - start);
		std::string value;
		
		if(quoted || str[valueStart] == '"') {
			valueStart++;
			size_t valueEnd = str.find_last_of('"');
			arx_assert(valueEnd != std::string::npos);
			
			if(valueEnd < valueStart) {
				
				// The localisation files are broken (missing ending quote)
				// But the spanish localisation files hae erroneous newlines in some values
				LogDebug("invalid quoted value @ line " << line << ": " << str);
				
				valueEnd = str.find_last_not_of(WHITESPACE) + 1;
				arx_assert(valueEnd >= valueStart);
				value = str.substr(valueStart, valueEnd - valueStart);
				
				// Add following lines until we find either a terminating quote,
				// an empty or commented line, a new section or a new key
				for(; is.good(); line++) {
					
					str.clear();
					getline(is, str);
					
					size_t newValueStart = str.find_first_not_of(WHITESPACE);
					if(newValueStart == std::string::npos) {
						// Empty line (only whitespace)
						break;
					}
					
					if(str[newValueStart] == '#'
					   || (newValueStart + 1 < str.length() && str[newValueStart] == '/' && str[newValueStart + 1] == '/')) {
						// Whole line was commented
						break;
					}
					
					if(str[newValueStart] == '[') {
						// New section
						line--, readline = false;
						break;
					}
					
					size_t newNameEnd = str.find_first_not_of(ALPHANUM, newValueStart);
					if(newNameEnd != std::string::npos && newNameEnd != newValueStart) {
						size_t newSeparator = str.find_first_not_of(WHITESPACE, newNameEnd);
						if(newSeparator != std::string::npos && str[newSeparator] == '=') {
							// New key
							line--, readline = false;
							break;
						}
					}
					
					// Replace newlines with spaces!
					value += ' ';
					
					size_t newValueEnd = str.find_last_of('"');
					if(newValueEnd != std::string::npos) {
						// End of multi-line value
						value += str.substr(newValueStart, newValueEnd - newValueStart);
						break;
					}
					
					newValueEnd = str.find_last_not_of(WHITESPACE) + 1;
					arx_assert(newValueEnd > newValueStart);
					value += str.substr(newValueStart, newValueEnd - newValueStart);
				}
				
			} else {
				value = str.substr(valueStart, valueEnd - valueStart);
			}
			
		} else {
			size_t valueEnd = str.find_last_not_of(WHITESPACE) + 1;
			arx_assert(valueEnd != std::string::npos);
			arx_assert(valueEnd >= valueStart);
			value = str.substr(valueStart, valueEnd - valueStart);
		}
		
		if(overrideValues) {
			section->setKey(std::move(key), std::move(value));
		} else {
			section->addKey(std::move(key), std::move(value));
		}
		
		// Ignoring rest of the line, not verifying that it's only whitespace / comment
		
	}
	
	return ok;
}

void IniReader::clear() {
	sections.clear();
}
