#include <iostream>
#include <cstring>

using namespace std;

class String
{
public:
    String () {};
    String(const char* a);
    String(char a) : String(1, a) {};
    String(size_t size, char a) : sz(size), capacity(size), beginStr(new char[capacity])
    {
		memset(beginStr, a, sz);
    }
    String(const String& s) : sz(s.sz), capacity(s.capacity), beginStr(new char[capacity])
    {
		memcpy(beginStr, s.beginStr, sz);
    }

	 ~String()
    {
        delete[] beginStr;
    }

    char& operator[](size_t i);
    const char& operator[](size_t i) const;
    size_t length() const;
    bool empty() const;
    char& front();
    char& back();
    const char& front() const;;
    const char& back() const;
    void clear();
    String substr(size_t start, size_t count) const;
    size_t find(const String& s) const;
    size_t rfind(const String& s) const;
    void push_back(char a);
    void pop_back ();
    String& operator=(String s);
    String& operator+=(const String& s);
    String& operator+=(const char s);
    bool operator==(const String& s) const;

private:
    size_t sz = 0;
    size_t capacity = 4;
    char* beginStr = new char [capacity];

    void updateCapacity(size_t newCapacity);
};

void String::updateCapacity(size_t newCapacity)
{
    capacity = newCapacity;
    char* oldBeginStr = beginStr;
    beginStr = new char [capacity];
    memcpy(beginStr, oldBeginStr, sz);
    delete[] oldBeginStr;
}

String::String(const char* a) : sz(0), capacity (4), beginStr(new char[capacity])
{
    int i = 0;
    while(a[i] != '\0')
    {
        push_back(a[i]);
        i ++;
    }
}

char& String::operator[](size_t i)
{
    return beginStr[i];
}

const char& String::operator[](size_t i) const
{
    return beginStr[i];
}

size_t String::length() const
{
    return sz;
}

bool String::empty() const
{
    return (sz == 0)? true : false;
}

char& String::front()
{
    return beginStr[0];
}

char& String::back()
{
    return beginStr[sz - 1];
}

const char& String::front() const
{
    return beginStr[0];
}

const char& String::back() const
{
    return beginStr[sz - 1];
}

void String::clear()
{
    updateCapacity(4);
    sz = 0;
}

String String::substr(size_t start, size_t count) const
{
    String s;
    s.sz = count;
    s.updateCapacity(count);
    memcpy(s.beginStr, beginStr + start, count);

    return s;
}

size_t String::find(const String& s) const
{
    for (size_t i = 0; i <= sz - s.sz; ++i)
    {
        if (s == substr(i, s.sz))
        {
            return i;
        }
    }
    return length();
}

size_t String::rfind(const String& s) const
{
    for (size_t i = sz - s.sz + 1; i > 0; --i)
        if (s == substr(i - 1, s.sz))
            return i - 1;

    return length();
}

void String::push_back(char a)
{
    if (sz == capacity)
        updateCapacity(capacity * 2);
    beginStr[sz] = a;
    sz++;
}

void String::pop_back()
{
    sz--;
    if (capacity/4 > sz && capacity >= 4)
        updateCapacity(capacity/2);
}

String& String::operator=(String s)
{
    delete[] beginStr;
    sz = s.sz;
    capacity = s.capacity;
    beginStr = new char[capacity];
    memcpy (beginStr, s.beginStr, sz);
    return *this;
}

String& String::operator+=(const String& s)
{
    if (capacity - sz < s.sz)
        updateCapacity(sz + s.sz);

    memcpy(beginStr + sz, s.beginStr, s.sz);
    sz += s.sz;
    return *this;
}

String& String::operator+=(const char s)
{
    push_back(s);
    return *this;
}

bool String::operator==(const String& s) const
{
    if (sz != s.sz)
        return false;

    for (size_t i = 0; i < sz; ++i)
    {
        if (beginStr[i] != s.beginStr[i])
        {
            return false;
        }
    }
    return true;
}

String operator+(const String& s, const String& ss)
{
    String copy = s;
    copy += ss;
    return copy;
}

ostream& operator<<(ostream& out, const String& s)
{
    for (size_t i = 0; i < s.length(); ++i)
        out << s[i];
    return out;
}

istream& operator>>(istream& in, String& s)
{
    char c;
    s.clear();
    do
    {
        c = in.get();
    }
    while (isspace(c));

    while (!in.eof() && !isspace(c))
    {
        s.push_back(c);
        c = in.get();
    }
    return in;
}

