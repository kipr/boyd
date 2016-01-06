#include "config.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

#define SEP "/"
#define EQUAL ":"

// Config

Config::Config()
{
}

Config::Config(const std::map<std::string, std::string> &config)
	: m_config(config)
{
}

Config *Config::load(const std::string &path)
{
	std::ifstream file(path.c_str());
	if(!file.is_open()) return 0;
	std::map<std::string, std::string> ret;
	while(!file.eof()) {
		std::string first;
		std::string second;
		file >> first;
		file.ignore();
		getline(file, second);
		first = first.substr(0, first.find_first_of(EQUAL[0]));
		if(first.empty()) continue;
		ret[first] = second;
	}
	file.close();
	return new Config(ret);
}

bool Config::save(const std::string &path) const
{
	std::ofstream file(path.c_str());
	if(!file.is_open()) return false;
	std::map<std::string, std::string>::const_iterator it = m_config.begin();
	for(; it != m_config.end(); ++it) {
		file << it->first << EQUAL " " << it->second << std::endl;
	}
	file.close();
	return true;
}

void Config::beginGroup(const std::string &group)
{
	m_groups.push_back(safeKey(group));
	m_cachedGroup = "";
}

void Config::endGroup()
{
	if(m_groups.empty()) return;
	m_groups.pop_back();
	m_cachedGroup = "";
}

void Config::clearGroup()
{
	m_groups.clear();
}

void Config::clear()
{
	m_groups.clear();
	m_config.clear();
}

bool Config::containsKey(const std::string &key) const
{
	return m_config.find(group() + safeKey(key)) != m_config.end();
}

bool Config::boolValue(const std::string &key) const
{
	return intValue(key);
}

int Config::intValue(const std::string &key) const
{
	std::stringstream stream(stringValue(key));
	int value = 0;
	stream >> value;
	return value;
}

double Config::doubleValue(const std::string &key) const
{
	std::stringstream stream(stringValue(key));
	double value = 0;
	stream >> value;
	return value;
}

std::string Config::stringValue(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator cit = m_config.find(group() + safeKey(key));
	//std::cout << "Value for " << key << " = \"" << cit->second << "\"" << std::endl;
	// Invalid key
	if(cit == m_config.end()) return "";
	return cit->second;
}

void Config::setValue(const std::string &key, const bool &value)
{
	setValue(key, (int)(value ? 1 : 0));
}

void Config::setValue(const std::string &key, const int &value)
{
	std::stringstream stream;
	stream << value;
	setValue(key, stream.str());
}

void Config::setValue(const std::string &key, const double &value)
{
	std::stringstream stream;
	stream << value;
	setValue(key, stream.str());
}

void Config::setValue(const std::string &key, const char *value)
{
	setValue(key, std::string(value));
}

void Config::setValue(const std::string &key, const std::string &value)
{
	m_config[group() + safeKey(key)] = value;
}

Config Config::values() const
{
	Config ret;
	const std::string group = Config::group();
	std::map<std::string, std::string>::const_iterator it = m_config.begin();
	for(; it != m_config.end(); ++it) {
		const std::string &key = it->first;
		if(key.substr(0, group.size()).compare(group)) continue;
		ret.setValue(key.substr(group.size()), it->second);
	}
	return ret;
}

void Config::addValues(const Config &config)
{
	const std::string group = Config::group();
	Config values = config.values();
	std::map<std::string, std::string>::const_iterator it = values.m_config.begin();
	for(; it != values.m_config.end(); ++it) setValue(it->first, it->second);
}

std::string Config::safeKey(std::string key) const
{
	std::string::iterator it = key.begin();
	for(; it != key.end(); ++it) {
		if((*it) == EQUAL[0]) *it = '_';
		if((*it) == ' ') *it = '_';
		if((*it) == '\n') *it = '_';
		if((*it) == '\f') *it = '_';
	}
	return key;
}

std::string Config::group() const
{
	if(!m_cachedGroup.empty()) return m_cachedGroup;
	std::vector<std::string>::const_iterator it = m_groups.begin();
	for(; it != m_groups.end(); ++it) m_cachedGroup += (*it) + SEP;
	return m_cachedGroup;
}

// ConfigPath

const std::string ConfigPath::m_sysBaseDir = "/etc/boyd/channels/";
std::string ConfigPath::m_baseDir = ConfigPath::m_sysBaseDir;

std::string ConfigPath::extension()
{
  return "conf";
}

std::string ConfigPath::sysBaseDir()
{
  return m_sysBaseDir;
}

std::string ConfigPath::baseDir()
{
  return m_baseDir;
}

void ConfigPath::setBaseDir(const std::string &path)
{
  m_baseDir = path;
  if(!m_baseDir.empty() && m_baseDir.back() != '/')
    m_baseDir += "/";
}

std::string ConfigPath::pathToConfig(const std::string &name)
{
  if(name.empty())
    return std::string();
  return m_baseDir + name + "." + extension();
}

std::string ConfigPath::pathToSysConfig(const std::string &name)
{
  if(name.empty())
    return std::string();
  return m_sysBaseDir + name + "." + extension();
}

std::string ConfigPath::defaultPath()
{
  return m_baseDir + "default";
}

std::string ConfigPath::defaultConfigPath()
{
  std::ifstream file;
  file.open(defaultPath().c_str());
  std::cout << "Attempting to open " << defaultPath().c_str() << std::endl;
  if(!file.is_open()) {
    std::cerr << "Failed to open default file" << std::endl;
    return std::string();
  }
  
  std::string ret;
  std::getline(file, ret);
  file.close();
  return ret;
}

void ConfigPath::setDefaultConfig(const std::string &name)
{
  std::ofstream file;
  file.open(defaultPath().c_str());
  if(!file.is_open()) {
    std::cerr << "Failed to open default file" << std::endl;
    return;
  }
  
  const std::string path = ConfigPath::pathToConfig(name);
  file.write(path.c_str(), path.size());
  file.close();
}

void ConfigPath::ensureSysConfigExists(const std::string &name)
{
  const std::string filePath = m_sysBaseDir + name;
  if(std::ifstream(filePath))
    return;
  
  std::ofstream newFile(filePath);
}