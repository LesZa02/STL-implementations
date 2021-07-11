#include <iostream>
#include <vector>

const int elemCnt = 8;

template <typename T>
class Deque
{
public:
    template <bool isConst>
    class common_iterator
    {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename std::conditional<isConst, const T, T>::type;
        using pointer = typename std::conditional<isConst, const T *, T *>::type;
        using reference = typename std::conditional<isConst, const T &, T &>::type;
        using iterator_category = std::random_access_iterator_tag;

        common_iterator(T**ptr, int bucket, int cell);
        ~common_iterator() = default;
        std::conditional_t<isConst, const T&, T&> operator*() const;
        std::conditional_t<isConst, const T*, T*> operator->();
        common_iterator& operator+=(int n);
        common_iterator& operator-=(int n);
        common_iterator& operator++();
        const common_iterator operator++(int);
        common_iterator& operator--();
        const common_iterator operator--(int);
        common_iterator& operator=(const common_iterator& another);
        bool operator==(const common_iterator &another) const;
        bool operator!=(const common_iterator &another) const;
        bool operator>(const common_iterator &another) const;
        bool operator>=(const common_iterator &another) const;
        bool operator<(const common_iterator &another) const;
        bool operator<=(const common_iterator &another) const;
        long long operator-(const common_iterator &another) const;
    private:
        int bucketInd = 0;
        int cellInd = 0;
        T** bucketPtr = nullptr;
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /////////////////////////////////////////////////////////////////////
    Deque();
    Deque(const Deque& another);
    explicit Deque(int n);
    Deque(int n, const T& val);
    ~Deque();
    Deque& operator=(const Deque& another);
    void push_back(const T& val);
    void push_front(const T& val);
    void pop_back();
    void pop_front();
    size_t size() const;
    T& operator[](int i);
    T& at(int i);
    const T& operator[](int i) const;
    const T& at(int i) const;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;
    void insert(iterator it, const T& val);
    void erase(iterator it);

private:
    T** bucket = nullptr;
    int capacity = 3;
    int bucketCnt = 0;
    int sz = 0;
    int firstBucketInd = 1;
    int lastBucketInd = 1;
    int firstElemInd = 0;
    int lastElemInd = elemCnt - 1;

    void swap(Deque& another);
    void resize(int n);
    void updateCapacity();
    bool needMoreCapFront() const;
    bool needMoreCapBack() const;
    void addBucketFront();
    void addBucketBack();
    void deleteBucketFront();
    void deleteBucketBack();
    bool isFullBack() const;
    bool isFullFront() const;

    void deleteAll(int i);
    void construct(int i, T val);
    int getBucketInd(int i) const;
    int getElemInd(int i) const;
    int getNextInd(int i) const;
    int getPrevInd(int i) const;
};

template<typename T>
void Deque<T>::deleteAll(int i)
{
    for (int j = 0; j < i; ++j)
    {
        (bucket[getBucketInd(j)] + getElemInd(j)) -> ~T();
    }
    for (int j = firstBucketInd; j <= lastBucketInd; ++j)
    {
        delete[] reinterpret_cast<int8_t*>(bucket[j]);
    }
    delete[] bucket;
}

template<typename T>
void Deque<T>::construct(int i, const T val)
{
    new(bucket[getBucketInd(i)] + getElemInd(i)) T(val);
}

template<typename T>
int Deque<T>::getBucketInd(int i) const
{
    return firstBucketInd + (firstElemInd + i) / elemCnt;
}

template<typename T>
int Deque<T>::getElemInd(int i) const
{
    return (firstElemInd + i) % elemCnt;
}

template<typename T>
int Deque<T>::getNextInd(int i) const
{
    return (i + 1) % elemCnt;
}

template<typename T>
int Deque<T>::getPrevInd(int i) const
{
    return (elemCnt + i - 1) % elemCnt;
}

template<typename T>
Deque<T>::Deque()
{
    resize(0);
}

template<typename T>
Deque<T>::Deque(const Deque &another)
{
    resize(static_cast<int>(another.size()));
    int i = 0;
    try
    {
        for (; i < sz; ++i)
        {
            construct(i, another[i]);
        }
    }
    catch (...)
    {
        deleteAll(i);
        throw;
    }
}

template<typename T>
Deque<T>::Deque(int n) : Deque(n, T()){}

template<typename T>
Deque<T>::Deque(int n, const T &val)
{
    resize(n);
    int i = 0;
    try
    {
        for (; i < sz; ++i)
        {
            construct(i, val);
        }
    }
    catch (...)
    {
        deleteAll(i);
        throw;
    }
}

template<typename T>
Deque<T>::~Deque()
{
    deleteAll(sz);
}

template<typename T>
Deque<T> &Deque<T>::operator=(const Deque &another)
{
    if (this == &another)
    {
        return *this;
    }
    try
    {
        Deque<T> newThis = another;
        swap(newThis);
        return *this;
    }
    catch (...)
    {
        throw;
    }
}

template<typename T>
void Deque<T>::swap(Deque &another)
{
    std::swap(bucket, another.bucket);
    std::swap(capacity, another.capacity);
    std::swap(bucketCnt, another.bucketCnt);
    std::swap(sz, another.sz);
    std::swap(firstBucketInd, another.firstBucketInd);
    std::swap(lastBucketInd, another.lastBucketInd);
    std::swap(firstElemInd, another.firstElemInd);
    std::swap(lastElemInd, another.lastElemInd);
}

template<typename T>
void Deque<T>::resize(int n)
{
    if (n == 0)
    {
        bucket = new T* [3];
        bucketCnt = 1;
        bucket[1] = reinterpret_cast<T*>(new int8_t[elemCnt * sizeof(T)]);
        sz = n;
        return;
    }
    bucketCnt = (n + elemCnt - 1) / elemCnt;
    capacity = 4 * bucketCnt;
    bucket = new T* [capacity];
    firstBucketInd = capacity / 4;
    lastBucketInd = bucketCnt + firstBucketInd - 1;
    for (int i = firstBucketInd; i <= lastBucketInd; ++i)
    {
        bucket[i] = reinterpret_cast<T*>(new int8_t[elemCnt * sizeof(T)]);
    }
    firstElemInd = 0;
    lastElemInd = n % elemCnt - 1 ;
    sz = n;
}

template<typename T>
void Deque<T>::updateCapacity()
{
    T** copy = bucket;
    capacity = 4 * bucketCnt;
    bucket = new T* [capacity];
    for (int i = 0; i < bucketCnt; ++i)
    {
        bucket[i + capacity / 4] = copy[i + firstBucketInd];
    }
    firstBucketInd = capacity / 4;
    lastBucketInd = firstBucketInd + bucketCnt - 1;
    delete[] copy;
}

template<typename T>
bool Deque<T>::needMoreCapFront() const
{
    return firstBucketInd == 0;
}

template<typename T>
bool Deque<T>::needMoreCapBack() const
{
    return lastBucketInd == capacity - 1;
}

template<typename T>
void Deque<T>::addBucketFront()
{
    if (needMoreCapFront())
    {
        updateCapacity();
    }
    bucket[firstBucketInd - 1] = reinterpret_cast<T*>(new int8_t[elemCnt * sizeof(T)]);
    bucketCnt++;
}

template<typename T>
void Deque<T>::addBucketBack()
{
    if (needMoreCapBack())
    {
        updateCapacity();
    }
    bucket[lastBucketInd + 1] = reinterpret_cast<T*>(new int8_t[elemCnt * sizeof(T)]);
    bucketCnt++;
}

template<typename T>
void Deque<T>::deleteBucketFront()
{
    delete[] reinterpret_cast<int8_t*>(bucket[firstBucketInd - 1]);
    bucketCnt--;
}

template<typename T>
void Deque<T>::deleteBucketBack()
{
    delete[] reinterpret_cast<int8_t*>(bucket[lastBucketInd + 1]);
    bucketCnt--;
}

template<typename T>
bool Deque<T>::isFullBack() const
{
    return (lastElemInd == elemCnt - 1 && sz != 0);
}

template<typename T>
bool Deque<T>::isFullFront() const
{
    return (firstElemInd == 0 && sz != 0);
}

template<typename T>
void Deque<T>::push_back(const T &val)
{
    int newLastBucketInd = lastBucketInd;
    int newLastElemInd = getNextInd(lastElemInd);
    if (isFullBack())
    {
        addBucketBack();
        newLastBucketInd = lastBucketInd + 1;
    }
    try
    {
        new(bucket[newLastBucketInd] + newLastElemInd) T(val);
    }
    catch (...)
    {
        throw;
    }
    lastBucketInd = newLastBucketInd;
    lastElemInd = newLastElemInd;
    sz++;
    if (sz == 1)
    {
        firstElemInd = lastElemInd;
    }
}

template<typename T>
void Deque<T>::push_front(const T &val)
{
    int newFirstBucketInd = firstBucketInd;
    int newFirstElemInd = getPrevInd(firstElemInd);
    if (isFullFront())
    {
        addBucketFront();
        newFirstBucketInd = firstBucketInd - 1;
    }
    try
    {
        new(bucket[newFirstBucketInd] + newFirstElemInd) T(val);
    }
    catch (...)
    {
        throw;
    }
    firstBucketInd = newFirstBucketInd;
    firstElemInd = newFirstElemInd;
    sz++;
    if (sz == 1)
    {
        lastElemInd = firstElemInd;
    }
}

template<typename T>
void Deque<T>::pop_back()
{
    (bucket[lastBucketInd] + lastElemInd) -> ~ T();
    lastBucketInd = lastBucketInd - (elemCnt - lastElemInd) / elemCnt;
    lastElemInd = getPrevInd(lastElemInd);
    sz--;
    if (isFullBack())
    {
        deleteBucketBack();
    }
}

template<typename T>
void Deque<T>::pop_front()
{
    (bucket[firstBucketInd] + firstElemInd) -> ~T();
    firstBucketInd = firstBucketInd + (firstElemInd + 1) / elemCnt;
    firstElemInd = getNextInd(firstElemInd);
    sz--;
    if (isFullFront())
    {
        deleteBucketFront();
    }
}

template<typename T>
size_t Deque<T>::size() const
{
    return sz;
}

template<typename T>
T& Deque<T>::operator[](int i)
{
    return bucket[getBucketInd(i)][getElemInd(i)];
}

template<typename T>
T& Deque<T>::at(int i)
{
    if (i >= static_cast<int>(size()))
    {
        throw std::out_of_range("out_of_range");
    }
    else
    {
        return bucket[getBucketInd(i)][getElemInd(i)];
    }
}

template<typename T>
const T& Deque<T>::operator[](int i) const
{
    return bucket[getBucketInd(i)][getElemInd(i)];
}

template<typename T>
const T& Deque<T>::at(int i) const
{
    if (i >= static_cast<int>(size()))
    {
        throw std::out_of_range("out_of_range");
    }
    else
    {
        return bucket[getBucketInd(i)][getElemInd(i)];
    }
}

///////////////////////////////////////////////////////////////////
template<typename T>
template<bool isConst>
Deque<T>::common_iterator<isConst>::common_iterator(T**ptr, int bucket, int cell)
{
    bucketPtr = ptr;
    bucketInd = bucket;
    cellInd = cell;
}

template<typename T>
template<bool isConst>
std::conditional_t<isConst, const T&, T&>  Deque<T>::common_iterator<isConst>::operator*() const
{
    return bucketPtr[bucketInd][cellInd];
}

template<typename T>
template<bool isConst>
std::conditional_t<isConst, const T*, T*> Deque<T>::common_iterator<isConst>::operator->()
{
    return bucketPtr[bucketInd] + cellInd;
}

template<typename T>
template<bool isConst>
typename Deque<T>::template common_iterator<isConst>& Deque<T>::common_iterator<isConst>::operator+=(int n)
{
    bucketInd = bucketInd + (cellInd + n) / elemCnt;
    cellInd = (cellInd + n) % elemCnt;
    return *this;
}

template<typename T>
template<bool isConst>
typename Deque<T>::template common_iterator<isConst>& Deque<T>::common_iterator<isConst>::operator-=(int n)
{
    int copyBucketInd = bucketInd;
    bucketInd = (elemCnt * bucketInd + cellInd - n) / elemCnt;
    cellInd = (elemCnt * copyBucketInd + cellInd - n) - elemCnt * bucketInd;
    return *this;
}

template<typename T>
template<bool isConst>
typename Deque<T>::template common_iterator<isConst>& Deque<T>::common_iterator<isConst>::operator++()
{
    *this += 1;
    return *this;
}

template<typename T>
template<bool isConst>
const typename Deque<T>::template common_iterator<isConst> Deque<T>::common_iterator<isConst>::operator++(int)
{
    common_iterator copy = *this;
    *this += 1;
    return copy;
}

template<typename T>
template<bool isConst>
typename Deque<T>::template common_iterator<isConst>& Deque<T>::common_iterator<isConst>::operator--()
{
    *this -= 1;
    return *this;
}

template<typename T>
template<bool isConst>
const typename Deque<T>::template common_iterator<isConst> Deque<T>::common_iterator<isConst>::operator--(int)
{
    common_iterator copy = *this;
    *this -= 1;
    return copy;
}

template<typename T>
template<bool isConst>
typename Deque<T>::template common_iterator<isConst>& Deque<T>::common_iterator<isConst>::operator=(const common_iterator& another)
{
    bucketInd = another.bucketInd;
    cellInd = another.cellInd;
    bucketPtr = another.bucketPtr;
}

template<typename T>
template<bool isConst>
bool Deque<T>::common_iterator<isConst>::operator==(const common_iterator &another) const
{
    return (bucketPtr == another.bucketPtr) && (bucketInd == another.bucketInd) &&
           (cellInd == another.cellInd);
}

template<typename T>
template<bool isConst>
bool Deque<T>::common_iterator<isConst>::operator!=(const common_iterator &another) const
{
    return !(*this == another);
}

template<typename T>
template<bool isConst>
bool Deque<T>::common_iterator<isConst>::operator>(const common_iterator &another) const
{
    return (elemCnt * bucketInd + cellInd) > (elemCnt * another.bucketInd + another.cellInd);
}

template<typename T>
template<bool isConst>
bool Deque<T>::common_iterator<isConst>::operator>=(const common_iterator &another) const
{
    return (*this == another) || (*this > another);
}

template<typename T>
template<bool isConst>
bool Deque<T>::common_iterator<isConst>::operator<(const common_iterator &another) const
{
    return !(*this >= another);
}
template<typename T>
template<bool isConst>
bool Deque<T>::common_iterator<isConst>::operator<=(const common_iterator &another) const
{
    return !(*this > another);
}

template<typename T>
template<bool isConst>
long long Deque<T>::common_iterator<isConst>::operator-(const common_iterator &another) const
{
    return elemCnt * bucketInd + cellInd - elemCnt * another.bucketInd - another.cellInd;
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename T>
typename Deque<T>::iterator Deque<T>::begin()
{
    return iterator(bucket, firstBucketInd, firstElemInd);
}

template<typename T>
typename Deque<T>::iterator Deque<T>::end()
{
    return iterator(bucket, lastBucketInd + (lastElemInd + 1) / elemCnt, (lastElemInd + 1) % elemCnt);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const
{
    return cbegin();
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::end() const
{
    return cend();
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const
{
    return const_iterator(bucket, firstBucketInd, firstElemInd);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const
{
    return const_iterator(bucket, lastBucketInd + (lastElemInd + 1) / elemCnt, (lastElemInd + 1) % elemCnt);
}

template<typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin()
{
    return reverse_iterator(end());
}

template<typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend()
{
    return reverse_iterator(begin());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const
{
    return const_reverse_iterator(begin());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const
{
    return const_reverse_iterator(end());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const
{
    return const_reverse_iterator(begin());
}

template<typename T>
void Deque<T>::insert(Deque<T>::iterator it, const T& val)
{
    (*this).push_back((*this)[sz - 1]);
    int index = it - (*this).begin();
    for (int i = static_cast<int>((*this).size() - 1); i > index; --i)
    {
        (*this)[i] = (*this)[i - 1];
    }
    (*this)[index] = val;
}

template<typename T>
void Deque<T>::erase(Deque<T>::iterator it)
{
    int index = it - (*this).begin();
    for (int i = index; i < static_cast<int>((*this).size() - 1); ++i)
    {
        (*this)[i] = (*this)[i + 1];
    }
    (*this).pop_back();
}

template<typename C>
C operator+(const C& it, const int n)
{
    C copy = it;
    copy += n;
    return copy;
}

template<typename C>
C operator+(int n, const C& it)
{
    C copy = it;
    copy += n;
    return copy;
}

template<typename C>
C operator-(const C& it, const int n)
{
    C copy = it;
    copy -= n;
    return copy;
}


