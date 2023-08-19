#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#define MRT_ASSERT(msg, type) static_assert(std::is_same<type, std::string>::value || std::is_same<type, std::wstring>::value, msg)

  /*********************/
 /*   Declarations!   */
/*********************/

namespace mrt
{
    template <class F, class M, class L>
    class Trio
    {
    public:
        Trio();
        Trio(const F& first, const M& mid, const L& last);

        F m_First;
        M m_Mid;
        L m_Last;
    };

    template <class F, class M, class L>
    [[nodiscard]] Trio<F, M, L> makeTrio(const F& first, const M& mid, const L& last);

    enum class ConfigFile_Error
    {
        Success = 0,
        Failed_To_Open = 1,
        Failed_To_Output = 2,
        Failed_To_Input = 3,
        Path_Not_Found = 4
    };

    template <class _StrType>
    class ConfigFile
    {
    public:
        // Type Assertion
        MRT_ASSERT("ConfigFile only supports std::string and std::wstring!", _StrType);

        // Type Aliases
        using ValueType = _StrType::value_type;
        using IStrStream = std::basic_stringstream<ValueType>;
        using OStrStream = std::basic_ostringstream<ValueType>;
        using IFStream = std::basic_ifstream<ValueType>;
        using OFStream = std::basic_ofstream<ValueType>;
        using IStream = std::basic_istream<ValueType>;
        using OStream = std::basic_ostream<ValueType>;
        using VecType = std::vector<Trio<_StrType, _StrType, _StrType>>;
    private:
        // Private variables
        _StrType m_Path;
        VecType m_ConfigData;
        static const _StrType DEFAULT_FILE_NAME;
        ConfigFile_Error m_Error = ConfigFile_Error::Success;
    protected:
        // Static Functions
        static [[nodiscard]] ConfigFile_Error inFile(const _StrType& path, VecType& data);
        static [[nodiscard]] ConfigFile_Error outFile(const _StrType& path, const VecType& data);

        static [[nodiscard]] ConfigFile_Error outFileToStream(OStream* const stream, const _StrType& path, const VecType& data);
    public:
        // Constructors & Destructor
        ConfigFile();
        ConfigFile(const _StrType& path);

        ~ConfigFile();

        // Member Functions
        [[nodiscard]] VecType::iterator find(const _StrType& group, const _StrType& name);
        [[nodiscard]] VecType::const_iterator find(const _StrType& group, const _StrType& name) const;

        template <class _ValueType>
        void write(const _StrType& group, const _StrType& name, const _ValueType& value, bool updateIfPresent = false);

        template <class _ValueType>
        [[nodiscard]] _ValueType read(const _StrType& group, const _StrType& name) const;

        template <class _ValueType>
        void read(const _StrType& group, const _StrType& name, _ValueType* const variable) const;

        void remove(const _StrType& group, const _StrType& name);

        void save();
        void save(OStream* const stream);

        void clear();

        [[nodiscard]] constexpr ConfigFile_Error error() const;
    };
}

  /*******************/
 /*   Trio Class!   */
/*******************/

template <class F, class M, class L>
mrt::Trio<F, M, L>::Trio() : m_First(F()), m_Mid(M()), m_Last(L()) { }

template <class F, class M, class L>
mrt::Trio<F, M, L>::Trio(const F& first, const M& mid, const L& last) : m_First(first), m_Mid(mid), m_Last(last) {  }

template <class F, class M, class L>
mrt::Trio<F, M, L> mrt::makeTrio(const F& first, const M& mid, const L& last)
{
    return mrt::Trio<F, M, L>(first, mid, last);
}

  /*************************/
 /*   ConfigFile Class!   */
/*************************/

template <>
const std::string mrt::ConfigFile<std::string>::DEFAULT_FILE_NAME = "\\ConfigFile.ini";

template <>
const std::wstring mrt::ConfigFile<std::wstring>::DEFAULT_FILE_NAME = L"\\ConfigFile.ini";

template <>
mrt::ConfigFile<std::string>::ConfigFile() : m_Path(std::filesystem::current_path().string())
{
    m_Path.append(DEFAULT_FILE_NAME);

    if (std::filesystem::exists(m_Path))
    {
        m_Error = inFile(m_Path, m_ConfigData);
    }
    else
    {
        m_Error = ConfigFile_Error::Path_Not_Found;
    }
}

template <>
mrt::ConfigFile<std::wstring>::ConfigFile() : m_Path(std::filesystem::current_path().wstring())
{
    m_Path.append(DEFAULT_FILE_NAME);

    if (std::filesystem::exists(m_Path))
    {
        m_Error = inFile(m_Path, m_ConfigData);
    }
    else
    {
        m_Error = ConfigFile_Error::Path_Not_Found;
    }
}

template <class _StrType>
mrt::ConfigFile<_StrType>::ConfigFile(const _StrType& path) : m_Path(path)
{
    if (std::filesystem::exists(m_Path))
    {
        m_Error = inFile(m_Path, m_ConfigData);
    }
    else
    {
        m_Error = ConfigFile_Error::Path_Not_Found;
    }
}

template <class _StrType>
mrt::ConfigFile<_StrType>::VecType::iterator mrt::ConfigFile<_StrType>::find(const _StrType& group, const _StrType& name)
{
    return std::find_if(m_ConfigData.begin(), m_ConfigData.end(), [&group, &name](const Trio<_StrType, _StrType, _StrType>& trio)
        {
            return trio.m_First == group && trio.m_Mid == name;
        }
    );
}

template <class _StrType>
mrt::ConfigFile<_StrType>::VecType::const_iterator mrt::ConfigFile<_StrType>::find(const _StrType& group, const _StrType& name) const
{
    return std::find_if(m_ConfigData.cbegin(), m_ConfigData.cend(), [&group, &name](const Trio<_StrType, _StrType, _StrType>& trio)
        {
            return trio.m_First == group && trio.m_Mid == name;
        }
    );
}

template <class _StrType> template <class _ValueType>
void mrt::ConfigFile<_StrType>::write(const _StrType& group, const _StrType& name, const _ValueType& value, bool updateIfPresent)
{
    auto it = find(group, name);

    OStrStream ss;
    ss << value;

    if (it == m_ConfigData.cend())                              //If setting doesn't exist creates a new one.
    {
        m_ConfigData.emplace_back(group, name, ss.str());
    }
    else if (it != m_ConfigData.cend() && updateIfPresent)      //If setting does exist updates the value.
    {
        m_ConfigData[std::distance(m_ConfigData.cbegin(), it)]._last = std::move(ss.str());
    }
}

template <> template <>
std::string mrt::ConfigFile<std::string>::read<std::string>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? (*it).m_Last : "";
}

template <> template <>
char mrt::ConfigFile<std::string>::read<char>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? (*it).m_Last[0] : ' ';
}

template <> template <>
int mrt::ConfigFile<std::string>::read<int>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stoi(((*it).m_Last)) : 0;
}

template <> template <>
long mrt::ConfigFile<std::string>::read<long>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stol((*it).m_Last) : 0;
}

template <> template <>
long long mrt::ConfigFile<std::string>::read<long long>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stoll((*it).m_Last) : 0;
}

template <> template <>
unsigned long long mrt::ConfigFile<std::string>::read<unsigned long long>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stoull((*it).m_Last) : 0;
}

template <> template <>
double mrt::ConfigFile<std::string>::read<double>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stod((*it).m_Last) : 0;
}

template <> template <>
long double mrt::ConfigFile<std::string>::read<long double>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stold((*it).m_Last) : 0;
}

template <> template <>
bool mrt::ConfigFile<std::string>::read<bool>(const std::string& group, const std::string& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? (((*it).m_Last == "1") ? true : false) : false;
}

template <> template <>
std::wstring mrt::ConfigFile<std::wstring>::read<std::wstring>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? (*it).m_Last : L"";
}

template <> template <>
wchar_t mrt::ConfigFile<std::wstring>::read<wchar_t>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? (*it).m_Last[0] : ' ';
}

template <> template <>
int mrt::ConfigFile<std::wstring>::read<int>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stoi(((*it).m_Last)) : 0;
}

template <> template <>
long mrt::ConfigFile<std::wstring>::read<long>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stol((*it).m_Last) : 0;
}

template <> template <>
long long mrt::ConfigFile<std::wstring>::read<long long>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stoll((*it).m_Last) : 0;
}

template <> template <>
unsigned long long mrt::ConfigFile<std::wstring>::read<unsigned long long>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stoull((*it).m_Last) : 0;
}

template <> template <>
double mrt::ConfigFile<std::wstring>::read<double>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stod((*it).m_Last) : 0;
}

template <> template <>
long double mrt::ConfigFile<std::wstring>::read<long double>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? std::stold((*it).m_Last) : 0;
}

template <> template <>
bool mrt::ConfigFile<std::wstring>::read<bool>(const std::wstring& group, const std::wstring& name) const
{
    auto it = find(group, name);
    return (it != m_ConfigData.cend()) ? (((*it).m_Last == L"1") ? true : false) : false;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<std::string>(const std::string& group, const std::string& name, std::string* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? (*it).m_Last : "";
}

template <> template <>
void mrt::ConfigFile<std::string>::read<char>(const std::string& group, const std::string& name, char* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? (*it).m_Last[0] : char(' ');
}

template <> template <>
void mrt::ConfigFile<std::string>::read<int>(const std::string& group, const std::string& name, int* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stoi((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<long>(const std::string& group, const std::string& name, long* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stol((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<long long>(const std::string& group, const std::string& name, long long* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stoll((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<unsigned long long>(const std::string& group, const std::string& name, unsigned long long* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stoull((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<double>(const std::string& group, const std::string& name, double* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stod((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<long double>(const std::string& group, const std::string& name, long double* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stold((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::string>::read<bool>(const std::string& group, const std::string& name, bool* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? (((*it).m_Last == "1") ? true : false) : false;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<std::wstring>(const std::wstring& group, const std::wstring& name, std::wstring* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? (*it).m_Last : L"";
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<wchar_t>(const std::wstring& group, const std::wstring& name, wchar_t* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? (*it).m_Last[0] : wchar_t(' ');
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<int>(const std::wstring& group, const std::wstring& name, int* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stoi((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<long>(const std::wstring& group, const std::wstring& name, long* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stol((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<long long>(const std::wstring& group, const std::wstring& name, long long* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stoll((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<unsigned long long>(const std::wstring& group, const std::wstring& name, unsigned long long* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stoull((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<double>(const std::wstring& group, const std::wstring& name, double* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stod((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<long double>(const std::wstring& group, const std::wstring& name, long double* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? std::stold((*it).m_Last) : 0;
}

template <> template <>
void mrt::ConfigFile<std::wstring>::read<bool>(const std::wstring& group, const std::wstring& name, bool* const variable) const
{
    auto it = find(group, name);
    *variable = (it != m_ConfigData.cend()) ? (((*it).m_Last == L"1") ? true : false) : false;
}

template <class _StrType>
void mrt::ConfigFile<_StrType>::remove(const _StrType& group, const _StrType& name)
{
    auto it = find(group, name);

    if (it != m_ConfigData.cend())
    {
        m_ConfigData.erase(it);
    }
}

template <class _StrType>
void mrt::ConfigFile<_StrType>::save() { m_Error = outFile(m_Path, m_ConfigData); }

template <class _StrType>
void mrt::ConfigFile<_StrType>::save(OStream* const stream) { m_Error = outFileToStream(stream, m_Path, m_ConfigData); }

template <class _StrType>
void mrt::ConfigFile<_StrType>::clear() { m_Error = ConfigFile_Error::Success; }

template <class _StrType>
constexpr mrt::ConfigFile_Error mrt::ConfigFile<_StrType>::error() const { return m_Error; }

template <class _StrType>
mrt::ConfigFile<_StrType>::~ConfigFile() { }

template <class _StrType>
mrt::ConfigFile_Error mrt::ConfigFile<_StrType>::inFile(const _StrType& path, VecType& data)
{
    IFStream fs(path);

    if (!fs.is_open())
    {
        return ConfigFile_Error::Failed_To_Open;
    }

    _StrType line, group;

    while (std::getline(fs, line))
    {
        if (fs.fail())
        {
            return ConfigFile_Error::Failed_To_Input;
        }
        else if (line[0] == ValueType('[') && line[line.size() - 1] == ValueType(']'))
        {
            group = line;
        }
        else
        {
            IStrStream ss(line);
            _StrType name, value;

            std::getline(ss, name, ValueType('='));
            std::getline(ss, value, ValueType('\n'));

            if ((name != _StrType()) || (value != _StrType()))
            {
                data.emplace_back(group, name, value);     //Moves the data into a 'Trio' object, and then into the vector.
            }
        }
    }

    return ConfigFile_Error::Success;
}

template <class _StrType>
mrt::ConfigFile_Error mrt::ConfigFile<_StrType>::outFile(const _StrType& path, const VecType& data)
{
    OFStream fs(path);

    if (!fs.is_open())
    {
        return ConfigFile_Error::Failed_To_Open;
    }

    _StrType group;

    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        if (group != (*it).m_First)
        {
            if (it != data.cbegin())
            {
                fs << std::endl;
            }

            fs << (*it).m_First << std::endl;
        }

        group = (*it).m_First;

        if (group == (*it).m_First)
        {
            fs << (*it).m_Mid << "=" << (*it).m_Last;

            if (it != (data.cend() - 1))
            {
                fs << std::endl;
            }
        }

        if (fs.fail())
        {
            return ConfigFile_Error::Failed_To_Output;
        }
    }

    return ConfigFile_Error::Success;
}

template <class _StrType>
mrt::ConfigFile_Error mrt::ConfigFile<_StrType>::outFileToStream(OStream* const stream, const _StrType& path, const VecType& data)
{
    _StrType group;

    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        if (group != (*it).m_First)
        {
            if (it != data.cbegin())
            {
                *stream << std::endl;
            }

            *stream << (*it).m_First << std::endl;
        }

        group = (*it).m_First;

        if (group == (*it).m_First)
        {
            *stream << (*it).m_Mid << "=" << (*it).m_Last;

            if (it != (data.cend() - 1))
            {
                *stream << std::endl;
            }
        }

        if (stream->fail())
        {
            return ConfigFile_Error::Failed_To_Output;
        }
    }

    return ConfigFile_Error::Success;
}