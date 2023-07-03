#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#define CASE_INSENSITIVE 0  //Set to '1', if you would like the 'ConfigFile' to be case insensitive.

  /*********************/
 /*   Declarations!   */
/*********************/

namespace mrt
{
    template <class F, class L>
    class Pair
    {
    public:
        Pair();
        Pair(const F& _first, const L& _last);

        F _first;
        L _last;
    };

    template <class F, class L>
    [[nodiscard]] Pair<F, L> makePair(const F& _first, const L& _last) noexcept;

    template <class F, class M, class L>
    class Trio
    {
    public:
        Trio();
        Trio(const F& _first, const M& _mid, const L& _last);

        F _first;
        M _mid;
        L _last;
    };

    template <class F, class M, class L>
    [[nodiscard]] Trio<F, M, L> makeTrio(const F& _first, const M& _mid, const L& _last) noexcept;

    enum class ConfigFile_Error
    {
        Success = 0,
        Failed_To_Open = 1,
        Failed_To_Output = 2,
        Failed_To_Input = 3,
        Path_Not_Found = 4
    };

    class ConfigFile_Handler
    {
    private:
        std::fstream* _fs = nullptr;

        ConfigFile_Error _error = ConfigFile_Error::Success;
    public:
        ConfigFile_Handler(const std::string& _path, std::vector<Trio<std::string, std::string, std::string>>* const _vec, bool _output = false);

        ~ConfigFile_Handler();

        void clear() noexcept;

        [[nodiscard]] constexpr ConfigFile_Error error() const noexcept;

        [[nodiscard]] ConfigFile_Error inFile(std::vector<Trio<std::string, std::string, std::string>>* const _vec) const noexcept;

        [[nodiscard]] ConfigFile_Error outFile(const std::vector<Trio<std::string, std::string, std::string>>* const _vec) const noexcept;
    };

    class ConfigFile
    {
    private:
        std::string _path;

        static const std::string DEFAULT_FILE_NAME;

        ConfigFile_Error _error = ConfigFile_Error::Success;

        std::vector<Trio<std::string, std::string, std::string>> _configData;
    public:
        ConfigFile();

        ConfigFile(const std::string& _path);

        ~ConfigFile();

        [[nodiscard]] std::vector<Trio<std::string, std::string, std::string>>::const_iterator find(const std::string& _group, const std::string& _name) const noexcept;

        template <class T>
        void write(const std::string& _group, const std::string& _name, const T& _value, bool _updateIfPresent = false) noexcept;

        template <class T>
        [[nodiscard]] T read(const std::string& _group, const std::string& _name) const noexcept;

        template <class T>
        void read(const std::string& _group, const std::string& _name, T* const _variable) const noexcept;

        void remove(const std::string& _group, const std::string& _name) noexcept;

        void save() noexcept;

        void clear() noexcept;

        [[nodiscard]] constexpr ConfigFile_Error error() const noexcept;
    };
}

  /*******************/
 /*   Pair Class!   */
/*******************/

template <class F, class L>
mrt::Pair<F, L>::Pair() : _first(F()), _last(L()) { }

template <class F, class L>
mrt::Pair<F, L>::Pair(const F& _first, const L& _last) : _first(_first), _last(_last) { }

template <class F, class L>
mrt::Pair<F, L> mrt::makePair(const F& _first, const L& _last) noexcept
{
    return mrt::Pair<F, L>(_first, _last);
}

  /*******************/
 /*   Trio Class!   */
/*******************/

template <class F, class M, class L>
mrt::Trio<F, M, L>::Trio() : _first(F()), _mid(M()), _last(L()) { }

template <class F, class M, class L>
mrt::Trio<F, M, L>::Trio(const F& _first, const M& _mid, const L& _last) : _first(_first), _mid(_mid), _last(_last) {  }

template <class F, class M, class L>
mrt::Trio<F, M, L> mrt::makeTrio(const F& _first, const M& _mid, const L& _last) noexcept
{
    return mrt::Trio<F, M, L>(_first, _mid, _last);
}

  /*********************************/
 /*   ConfigFile_Handler Class!   */
/*********************************/

mrt::ConfigFile_Handler::ConfigFile_Handler(const std::string& _path, std::vector<Trio<std::string, std::string, std::string>>* const _vec, bool _output)
    : _fs(((_output) ? new std::fstream(_path, std::ios::out | std::ios::trunc) : new std::fstream(_path, std::ios::in)))
{
    _error = (_output) ? outFile(_vec) : inFile(_vec);
}

constexpr mrt::ConfigFile_Error mrt::ConfigFile_Handler::error() const noexcept
{
    return _error; //Returns anything but success, if an error has occurred.
}

void mrt::ConfigFile_Handler::clear() noexcept
{
    _error = ConfigFile_Error::Success;
}

mrt::ConfigFile_Error mrt::ConfigFile_Handler::inFile(std::vector<Trio<std::string, std::string, std::string>>* const _vec) const noexcept
{
    if (!_fs->is_open())
    {
        return ConfigFile_Error::Failed_To_Open;
    }
    std::string line, group;

    while (getline(*_fs, line))
    {
        if (_fs->fail())
        {
            return ConfigFile_Error::Failed_To_Input;
        }
        else if (line[0] == '[' && line[line.size() - 1] == ']')
        {
            group = line;
        }
        else
        {
            std::istringstream ss(line);
            std::string name, value;

            getline(ss, name, '='); 
            getline(ss, value, '\n'); 

            if ((name != "") || (value != ""))
            {

#if CASE_INSENSITIVE == 1
                transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
                transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

                _vec->emplace_back(group, name, value); //Moves the data into a 'Trio' object, and then into the vector.
            }
        }
    }
    return ConfigFile_Error::Success;
}

mrt::ConfigFile_Error mrt::ConfigFile_Handler::outFile(const std::vector<Trio<std::string, std::string, std::string>>* const _vec) const noexcept
{
    if (!_fs->is_open())
    {
        return ConfigFile_Error::Failed_To_Open;
    }
    std::string group;

    for (auto it = _vec->cbegin(); it != _vec->cend(); ++it)
    {
        if (group != (*it)._first)
        {
            if (it != _vec->cbegin())
            {
                *_fs << std::endl;
            }

            *_fs << (*it)._first << std::endl;
        }

        group = (*it)._first;

        if (group == (*it)._first)
        {
            *_fs << (*it)._mid << "=" << (*it)._last;

            if (it != (_vec->cend() - 1))
            {
                *_fs << std::endl;
            }
        }

        if (_fs->fail())
        {
            return ConfigFile_Error::Failed_To_Output;
        }
    }
    return ConfigFile_Error::Success;
}

mrt::ConfigFile_Handler::~ConfigFile_Handler()
{
    _fs->close();
    delete (_fs);
}

  /*************************/
 /*   ConfigFile Class!   */
/*************************/

const std::string mrt::ConfigFile::DEFAULT_FILE_NAME = "\\FileConfig.ini";

mrt::ConfigFile::ConfigFile() : _path(std::filesystem::current_path().string())
{
    _path.append(DEFAULT_FILE_NAME);

    if (std::filesystem::exists(_path))
    {
        ConfigFile_Handler in(_path, &_configData);
        _error = in.error();
    }
    else
    {
        _error = ConfigFile_Error::Path_Not_Found;
    }
}

mrt::ConfigFile::ConfigFile(const std::string& _path) : _path(_path)
{
    if (std::filesystem::exists(this->_path))
    {
        ConfigFile_Handler in(this->_path, &_configData);
        _error = in.error();
    }
    else
    {
        _error = ConfigFile_Error::Path_Not_Found;
    }
}

std::vector<mrt::Trio<std::string, std::string, std::string>>::const_iterator mrt::ConfigFile::find(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group(_group), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return tolower(c); });
#endif

    return std::find_if(_configData.cbegin(), _configData.cend(), [&group, &name](const Trio<std::string, std::string, std::string>& trio)
        {
            return trio._first == group && trio._mid == name;
        }
    );
}

template <class T>
void mrt::ConfigFile::write(const std::string& _group, const std::string& _name, const T& _value, bool _updateIfPresent) noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return tolower(c); });
#endif

    auto it = find(group, name);

    std::ostringstream ss;
    ss << _value;

    if (it == _configData.cend())   //If setting doesn't exist creates a new one.
    {
        _configData.emplace_back(group, name, ss.str());
    }
    else if (it != _configData.cend() && _updateIfPresent)   //If setting does exist updates the value.
    {
        _configData[std::distance(_configData.cbegin(), it)]._last = std::move(ss.str());
    }
}

template <>
std::string mrt::ConfigFile::read<std::string>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? (*it)._last : "";
}

template <>
char mrt::ConfigFile::read<char>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? (*it)._last[0] : ' ';
}

template <>
int mrt::ConfigFile::read<int>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? std::stoi(((*it)._last)) : 0;
}

template <>
long mrt::ConfigFile::read<long>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? std::stol((*it)._last) : 0;
}

template <>
long long mrt::ConfigFile::read<long long>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? std::stoll((*it)._last) : 0;
}

template <>
unsigned long long mrt::ConfigFile::read<unsigned long long>(const std::string& _group, const std::string& _name) const noexcept
{
	std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? std::stoull((*it)._last) : 0;
}

template <>
double mrt::ConfigFile::read<double>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? std::stod((*it)._last) : 0;
}

template <>
long double mrt::ConfigFile::read<long double>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? std::stold((*it)._last) : 0;
}

template <>
bool mrt::ConfigFile::read<bool>(const std::string& _group, const std::string& _name) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    return (it != _configData.cend()) ? (((*it)._last == "1") ? true : false) : false;
}

template <>
void mrt::ConfigFile::read<std::string>(const std::string& _group, const std::string& _name, std::string* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? (*it)._last : "";
}

template <>
void mrt::ConfigFile::read<char>(const std::string& _group, const std::string& _name, char* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? (*it)._last[0] : ' ';
}

template <>
void mrt::ConfigFile::read<int>(const std::string& _group, const std::string& _name, int* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? std::stoi((*it)._last) : 0;
}

template <>
void mrt::ConfigFile::read<long>(const std::string& _group, const std::string& _name, long* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? std::stol((*it)._last) : 0;
}

template <>
void mrt::ConfigFile::read<long long>(const std::string& _group, const std::string& _name, long long* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? std::stoll((*it)._last) : 0;
}

template <>
void mrt::ConfigFile::read<unsigned long long>(const std::string& _group, const std::string& _name, unsigned long long* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? std::stoull((*it)._last) : 0;
}

template <>
void mrt::ConfigFile::read<double>(const std::string& _group, const std::string& _name, double* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? std::stod((*it)._last) : 0;
}

template <>
void mrt::ConfigFile::read<long double>(const std::string& _group, const std::string& _name, long double* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? std::stold((*it)._last) : 0;
}

template <>
void mrt::ConfigFile::read<bool>(const std::string& _group, const std::string& _name, bool* const _variable) const noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);
    *_variable = (it != _configData.cend()) ? (((*it)._last == "1") ? true : false) : false;
}

void mrt::ConfigFile::remove(const std::string& _group, const std::string& _name) noexcept
{
    std::string group("[" + _group + "]"), name(_name);

#if CASE_INSENSITIVE == 1
    transform(group.begin(), group.end(), group.begin(), [](char& c) { return std::tolower(c); });
    transform(name.begin(), name.end(), name.begin(), [](char& c) { return std::tolower(c); });
#endif

    auto it = find(group, name);

    if (it != _configData.cend())
    {
        _configData.erase(it);
    }
}

void mrt::ConfigFile::save() noexcept
{
    ConfigFile_Handler out(_path, &_configData, true);
    _error = out.error();
}

void mrt::ConfigFile::clear() noexcept
{
    _error = ConfigFile_Error::Success;
}

constexpr mrt::ConfigFile_Error mrt::ConfigFile::error() const noexcept
{
    return _error;
}

mrt::ConfigFile::~ConfigFile() { }