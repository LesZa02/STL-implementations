#include <vector>
#include <functional>
#include <cmath>

float defaultMaxLoadFactor = 0.75;

template<typename T, typename Allocator>
class List
{
public:
    struct Node
    {
    public:
        template<typename... Args>
        Node(Args&& ... args) : value(std::forward<Args>(args)...) {}

        ~Node() = default;

        T value;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    Node* head;
    Node* fakeTail;
    size_t sz = 0;

public:
    Allocator typeAlloc;
    using AllocTraits = std::allocator_traits<Allocator>;

    using AllocNode = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    AllocNode nodeAlloc;
    using NodeAllocTraits = std::allocator_traits<AllocNode>;

    explicit List(const Allocator& alloc = Allocator());
    explicit List(size_t count, const T& value = T(), const Allocator& alloc = Allocator());
    List (const List& another);
    List (List&& another);
    void Swap(List& another);
    ~List();

    template <bool isConst>
    class common_iterator : public std::iterator<std::bidirectional_iterator_tag, T>
    {
    public:
        common_iterator() = default;
        explicit common_iterator(Node* p);
        template<bool isC = false>
        common_iterator(const common_iterator<false>& another);
        common_iterator(const common_iterator<isConst>& another);
        ~common_iterator() = default;

        std::conditional_t<isConst, const T&, T&> operator*() const;
        std::conditional_t<isConst, const T*, T*> operator->();
        common_iterator& operator++();
        common_iterator operator++(int);
        common_iterator& operator--();
        common_iterator operator--(int);
        common_iterator& operator=(const common_iterator& another);
        template<bool isC = false>
        common_iterator& operator=(const common_iterator<false>& another);
        bool operator==(const common_iterator &another) const;
        bool operator!=(const common_iterator &another) const;
        Node* getPointer() const;

    private:
        Node* ptr = nullptr;
    };
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    size_t size();

    template<typename... Args>
    Node* makeNode(Args&& ... args);
    void deleteNode(Node* node);
    iterator insert(const_iterator it, Node *node);
    void erase(const_iterator it);
    Node* extractNode(iterator it);
};
////////////////////////////////////////////////////////////////

template<typename T, typename Allocator>
template<typename... Args>
typename List<T, Allocator>::Node* List<T, Allocator>::makeNode(Args&& ... args)
{
    Node* node = NodeAllocTraits::allocate(nodeAlloc, 1);
    AllocTraits::construct(typeAlloc, &(node -> value), std::forward<Args>(args)...);
    node -> prev = nullptr;
    node -> next = nullptr;
    return node;
}

template<typename T, typename Allocator>
void List<T, Allocator>::deleteNode(Node* node)
{
    NodeAllocTraits::destroy(nodeAlloc, node);
    NodeAllocTraits::deallocate(nodeAlloc, node, 1);
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const Allocator& alloc) : sz(0), typeAlloc(alloc), nodeAlloc(alloc)
{
    fakeTail = reinterpret_cast<Node*> (new int8_t[sizeof(Node)]);
    fakeTail -> next = nullptr;
    fakeTail -> prev = nullptr;
    head = fakeTail;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc) : List(alloc)
{
    for (size_t i = 0; i < count; ++i)
    {
        insert(end(), value);
    }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const List& another) : List(std::allocator_traits<Allocator>::select_on_container_copy_construction(another.typeAlloc))
{
    Node *node = another.head;
    for (size_t i = 0; i < another . sz; ++i)
    {
        Node* newNode = makeNode(node -> value);
        insert(cend(), newNode);
        node = node -> next;
    }
}

template<typename T, typename Allocator>
void List<T, Allocator>::Swap(List& another)
{
    std::swap(head, another.head);
    std::swap(fakeTail, another.fakeTail);
    std::swap(sz, another.sz);
    std::swap(typeAlloc, another.typeAlloc);
    std::swap(nodeAlloc, another.nodeAlloc);
}

template<typename T, typename Allocator>
List<T, Allocator>::List(List&& another) : List()
{
    Swap(another);
}

template<typename T, typename Allocator>
List<T, Allocator>::~List()
{
    size_t cnt = sz;
    for (size_t i = 0; i < cnt; ++i)
    {
        iterator it = end();
        it--;
        erase(it);
    }
    delete[] reinterpret_cast<int8_t*>(fakeTail);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::insert(const_iterator it, Node* node)
{
    sz++;
    node -> next = it.getPointer();
    node -> prev = it.getPointer() -> prev;
    node -> next -> prev = node;

    if (node -> prev != nullptr)
    {
        node -> prev -> next = node;
    }
    else
    {
        head = node;
    }
    return iterator(node);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::extractNode(iterator it)
{
    Node* node = it.getPointer();
    sz--;
    node -> next -> prev = node -> prev;
    if (node -> prev != nullptr)
    {
        node -> prev -> next = node -> next;
    }
    else
    {
        head = node->next;
    }
    node->next = nullptr;
    node->prev = nullptr;
    return node;
}

template<typename T, typename Allocator>
void List<T, Allocator>::erase(const_iterator it)
{
    Node* node = it.getPointer();
    sz--;

    node -> next -> prev = node -> prev;
    if (node -> prev != nullptr)
    {
        node -> prev -> next = node -> next;
    }
    else
    {
        head = node -> next;
    }
    NodeAllocTraits::destroy(nodeAlloc, node);
    NodeAllocTraits::deallocate(nodeAlloc, node, 1);
}

////////////////////////////////////////////////////////////////
template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(Node* p)
{
    ptr = p;
}

template<typename T, typename Allocator>
template<bool isConst>
template<bool>
List<T, Allocator>::common_iterator<isConst>::common_iterator(const common_iterator<false>& another)
{
    ptr = another.getPointer();
}

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(const common_iterator& another)
{
    ptr = another.getPointer();
}

template<typename T, typename Allocator>
template<bool isConst>
std::conditional_t<isConst, const T&, T&> List<T, Allocator>::common_iterator<isConst>::operator*() const
{
    return ptr -> value;
}

template<typename T, typename Allocator>
template<bool isConst>
std::conditional_t<isConst, const T*, T*> List<T, Allocator>::common_iterator<isConst>::operator->()
{
    return &(ptr -> value);
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::template common_iterator<isConst>& List<T, Allocator>::common_iterator<isConst>::operator++()
{
    ptr = ptr -> next;
    return *this;
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::template common_iterator<isConst> List<T, Allocator>::common_iterator<isConst>::operator++(int)
{
    common_iterator copyPtr = *this;
    ptr = ptr -> next;
    return copyPtr;
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::template common_iterator<isConst>& List<T, Allocator>::common_iterator<isConst>::operator--()
{
    ptr = ptr -> prev;
    return *this;
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::template common_iterator<isConst> List<T, Allocator>::common_iterator<isConst>::operator--(int)
{
    common_iterator copyPtr = *this;
    ptr = ptr -> prev;
    return copyPtr;
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::template common_iterator<isConst>& List<T, Allocator>::common_iterator<isConst>::operator=(const common_iterator<isConst>& another)
{
    ptr = another.ptr;
    return *this;
}

template<typename T, typename Allocator>
template<bool isConst>
template<bool>
typename List<T, Allocator>::template common_iterator<isConst>& List<T, Allocator>::common_iterator<isConst>::operator=(const common_iterator<false>& another)
{
    ptr = another.ptr;
    return *this;
}

template<typename T, typename Allocator>
template<bool isConst>
bool List<T, Allocator>::common_iterator<isConst>::operator==(const common_iterator &another) const
{
    return (ptr == another.ptr);
}

template<typename T, typename Allocator>
template<bool isConst>
bool List<T, Allocator>::common_iterator<isConst>::operator!=(const common_iterator &another) const
{
    return !(*this == another);
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::Node* List<T, Allocator>::common_iterator<isConst>::getPointer() const
{
    return ptr;
}

/////////////////////////////////////////////////////////////
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin()
{
    return iterator(head);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end()
{
    return iterator(fakeTail);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const
{
    return const_iterator(head);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const
{
    return const_iterator(fakeTail);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const
{
    return const_iterator(head);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const
{
    return const_iterator(fakeTail);
}

template<typename T, typename Allocator>
size_t List<T, Allocator>::size()
{
    return sz;
}

////////////////////////////////////////////////
template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap
{
public:
    using NodeType = std::pair<const Key, Value>;
    using ListIterator = typename List<NodeType, Alloc>::iterator;
    using ConstListIterator = typename List<NodeType, Alloc>::const_iterator;
    using ListNode = typename List<NodeType, Alloc>::Node;

    UnorderedMap();
    UnorderedMap(const UnorderedMap& another);
    UnorderedMap(UnorderedMap&& another);
    explicit UnorderedMap(size_t bucketsCount);
    ~UnorderedMap() = default;
    UnorderedMap& operator=(const UnorderedMap& another);
    UnorderedMap& operator=(UnorderedMap&& another);

    template <bool isConst>
    class MapIterator
    {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename std::conditional<isConst, const NodeType, NodeType>::type;
        using pointer = typename std::conditional<isConst, const NodeType*, NodeType*>::type;
        using reference = typename std::conditional<isConst, const NodeType&, NodeType&>::type;
        using iterator_category = std::forward_iterator_tag;

        MapIterator(ListIterator another) : listIter(another) {}
        MapIterator(ConstListIterator another) : listIter(another) {}
        template<bool isC = false>
        MapIterator(const MapIterator<false>& another): listIter(another.listIter) {}
        MapIterator(const MapIterator& another) : listIter(another.listIter) {}

        ~MapIterator() = default;

        std::conditional_t<isConst, const NodeType&, NodeType&> operator*() const;
        std::conditional_t<isConst, const NodeType*, NodeType*> operator->();
        MapIterator& operator++();
        MapIterator operator++(int);
        MapIterator& operator=(const MapIterator& another);
        template<bool isC = false>
        MapIterator& operator=(const MapIterator<false>& another);
        bool operator==(const MapIterator &another) const;
        bool operator!=(const MapIterator &another) const;
        std::conditional_t<isConst, ConstListIterator&, ListIterator&> getListIter();

    private:
        std::conditional_t<isConst, ConstListIterator, ListIterator> listIter;
    };
    using Iterator = MapIterator<false>;
    using ConstIterator = MapIterator<true>;

    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;
    ConstIterator cbegin() const;
    ConstIterator cend() const;

    Iterator find(const Key& key);
    ConstIterator find(const Key& key) const;
    Value& operator[](const Key& key);
    Value& at(const Key& key);
    const Value& at(const Key& key) const;

    void rehash(size_t count);
    void reserve(size_t count);
    float load_factor() const;
    float max_load_factor() const;
    void max_load_factor(float newLoadFactor);
    size_t max_size() const;
    size_t size() const;

    std::pair<Iterator,bool> insert(NodeType&& node);
    template<typename Pair>
    std::pair<Iterator, bool> insert(Pair&& pair);
    std::pair<Iterator,bool> insert(ListNode* node);

    template<typename InputIt>
    void insert(InputIt first, InputIt last);
    template< class... Args >
    std::pair<Iterator,bool> emplace( Args&&... args );
    Iterator erase(Iterator pos);
    Iterator erase(Iterator first, Iterator last);

private:
    struct Chain
    {
        ListIterator firstElem;
        size_t chainSz = 0;

        Chain() = default;
        Chain(const ListIterator& it, const size_t sz) : firstElem(it), chainSz(sz) {}
        ~Chain() = default;
    };

    using chainAlloc = typename Alloc::template rebind<Chain>::other;

    List<NodeType, Alloc> mainList;
    std::vector<Chain, chainAlloc> buckets;
    size_t listSz;
    size_t bucketsCnt;
    float maxLoadFactor;
    Hash hashFunction;
    Equal equalFunction;

    void Swap(UnorderedMap& another);
    size_t getHash(const Key& key);
};
///////////////////////////////////////////////////////

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap() :
        mainList(),
        buckets(4, Chain(mainList.end(), 0)),
        listSz(0),
        bucketsCnt(4),
        maxLoadFactor(defaultMaxLoadFactor) {}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(const UnorderedMap& another) :
        mainList(another.mainList),
        buckets(another.buckets),
        listSz(another.listSz),
        bucketsCnt(another.bucketsCnt),
        maxLoadFactor(another.maxLoadFactor),
        hashFunction(another.hashFunction),
        equalFunction(another.equalFunction) {}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(UnorderedMap&& another) :
        mainList(move(another.mainList)),
        buckets(move(another.buckets)),
        listSz(another.listSz),
        bucketsCnt(another.bucketsCnt),
        maxLoadFactor(another.maxLoadFactor),
        hashFunction(another.hashFunction),
        equalFunction(another.equalFunction)
{
    another.listSz = 0;
    another.bucketsCnt = 0;
    another.maxLoadFactor = defaultMaxLoadFactor;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(size_t bucketsCount) :
        mainList(),
        buckets(bucketsCount, Chain(mainList.end(), 0)),
        listSz(0),
        bucketsCnt(bucketsCount),
        maxLoadFactor(defaultMaxLoadFactor) {}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::Swap(UnorderedMap& another)
{
    mainList.Swap(another.mainList);
    std::swap(buckets, another.buckets);
    std::swap(listSz, another.listSz);
    std::swap(bucketsCnt, another.bucketsCnt);
    std::swap(maxLoadFactor, another.maxLoadFactor);
    std::swap(hashFunction, another.hashFunction);
    std::swap(equalFunction, another.equalFunction);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(const UnorderedMap& another)
{
    UnorderedMap copy = another;
    Swap(copy);
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(UnorderedMap&& another)
{
    UnorderedMap copy = std::move(another);
    Swap(copy);
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin()
{
    return Iterator(mainList.begin());
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::end()
{
    return Iterator(mainList.end());
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() const
{
    return ConstIterator(mainList.cbegin());
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() const
{
    return ConstIterator(mainList.cend());
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::cbegin() const
{
    return ConstIterator(mainList.begin());
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::cend() const
{
    return ConstIterator(mainList.end());
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key)
{
    size_t bucketInd = hashFunction(key) % bucketsCnt;
    size_t i = 0;
    for (auto it = buckets[bucketInd].firstElem; i < buckets[bucketInd].chainSz; ++it, ++i)
    {
        if (equalFunction(it -> first, key))
        {
            return Iterator(it);
        }
    }
    return end();
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) const
{
    size_t bucketInd = hashFunction(key) % bucketsCnt;
    size_t i = 0;
    for (auto it = buckets[bucketInd].firstElem; i < buckets[bucketInd].chainSz; ++it, ++i)
    {
        if (equalFunction(it -> first, key))
        {
            return Iterator(it);
        }
    }
    return cend();
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](const Key& key)
{
    Iterator it = find(key);
    if (it == end())
    {
        std::pair<Iterator, bool> insertResult = insert({key, Value()});
        return (insertResult.first) -> second;
    }
    else
    {
        return it -> second;
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key& key)
{
    Iterator it = find(key);
    if (it == end())
    {
        throw std::out_of_range("out_of_range");
    }
    else
    {
        return it -> second;
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
const Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key& key) const
{
    Iterator it = find(key);
    if (it == end())
    {
        throw std::out_of_range("out_of_range");
    }
    else
    {
        return it -> second;
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::size() const
{
    return listSz;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::getHash(const Key& key)
{
    return hashFunction(key) % bucketsCnt;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::rehash(size_t count)
{
    buckets.clear();
    buckets.resize(count, Chain(mainList.end(), 0));
    bucketsCnt = count;

    if (mainList.size() == 0)
    {
        return;
    }
    ListNode* node = mainList.head;
    ListNode* lastNode = mainList.fakeTail -> prev;
    ListNode* nextNode = node;
    while (node != lastNode)
    {
        node = nextNode;
        nextNode = node -> next;
        size_t newPos = getHash((node -> value).first);
        if (buckets[newPos].firstElem == mainList.end())
        {
            buckets[newPos].firstElem = mainList.insert(mainList.end(), mainList.extractNode(ListIterator(node)));
            ++buckets[newPos].chainSz;
        }
        else
        {
            mainList.insert(buckets[newPos].firstElem, mainList.extractNode(ListIterator(node)));
            ++buckets[newPos].chainSz;
        }
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reserve(size_t count)
{
    if (count > maxLoadFactor * bucketsCnt)
    {
        rehash(static_cast<size_t>(std::ceil(count / maxLoadFactor)));
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::load_factor() const
{
    return static_cast<float>(listSz) / bucketsCnt;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_load_factor() const
{
    return maxLoadFactor;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>:: max_load_factor(float newLoadFactor)
{
    maxLoadFactor = newLoadFactor;
    if (load_factor() > maxLoadFactor)
    {
        rehash(static_cast<size_t>(std::ceil(listSz / maxLoadFactor)));
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_size() const
{
    return maxLoadFactor * bucketsCnt;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(ListNode* node)
{
    Iterator it = find((node -> value).first);
    if (it != end())
    {
        mainList.deleteNode(node);
        return {it, false};
    }
    else
    {
        listSz++;
        if (load_factor() > maxLoadFactor)
        {
            rehash(2 * static_cast<size_t>(std::ceil(listSz / maxLoadFactor)));
        }
        size_t pos = getHash((node -> value).first);
        ListIterator& listPos = buckets[pos].firstElem;
        buckets[pos].firstElem = mainList.insert(listPos, node);
        buckets[pos].chainSz++;
        return {Iterator(listPos), true};
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(UnorderedMap::NodeType&& node)
{
    ListNode* newNode = mainList.makeNode(std::move(node));
    return insert(newNode);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename Pair>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(Pair&& pair)
{
    ListNode* node = mainList.makeNode(std::forward<Pair>(pair));
    return insert(node);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename InputIt>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(InputIt begin, InputIt end)
{
    for (InputIt it = begin; it != end; ++it)
    {
        insert(*it);
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename... Args>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::emplace(Args&& ... args)
{
    ListNode* node = mainList.makeNode(std::forward<Args>(args)...);
    return insert(node);
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(Iterator it)
{
    Iterator nextIt = it;
    ++nextIt;
    listSz--;
    size_t pos = getHash((*it).first);
    ListIterator& listPos = it.getListIter();
    mainList.erase(listPos);
    buckets[pos].chainSz--;
    if (buckets[pos].chainSz == 0)
    {
        buckets[pos].firstElem = mainList.end();
    }
    return nextIt;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(Iterator first, Iterator last)
{
    Iterator it = first;
    for (; it != last; it = erase(it));
    return it;
}

//////////////////////////////////////////////////////////////
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
std::conditional_t<isConst, const typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType&, typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType&>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator*() const
{
    return *listIter;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
std::conditional_t<isConst, const typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType*, typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType*>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator->()
{
    return &(*listIter);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::template MapIterator<isConst>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator++()
{
    ++listIter;
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::template MapIterator<isConst>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator++(int)
{
    MapIterator copyPtr = *this;
    ++listIter;
    return copyPtr;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::template MapIterator<isConst>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator=(const MapIterator<isConst>& another)
{
    listIter = another.listIter;
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
template<bool>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::template MapIterator<isConst>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator=(const MapIterator<false>& another)
{
    listIter = another.listIter;
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
bool UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator==(const MapIterator &another) const
{
    return (listIter == another.listIter);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
bool UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::operator!=(const MapIterator &another) const
{
    return !(*this == another);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<bool isConst>
std::conditional_t<isConst, typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstListIterator&, typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ListIterator&>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::MapIterator<isConst>::getListIter()
{
    return listIter;
}