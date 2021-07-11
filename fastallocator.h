#include <iostream>
#include <memory>
#include <vector>
#include <type_traits>
#include <cassert>
#include <iterator>

template <size_t chunkSize>
struct FixedAllocator
{
public:
    struct Chunk
    {
        int8_t data[chunkSize];
        Chunk* nextFree;
    };

    FixedAllocator();
    ~FixedAllocator();

    void* allocate();
    void deallocate(void* ptr);

private:
    std::vector<Chunk*> blocks;
    Chunk* freeMemory;
    size_t blockSz = 512;

    void addBlock();
};

template <size_t chunkSize>
void FixedAllocator<chunkSize>::addBlock()
{
    auto* block = new Chunk[blockSz];
    for (size_t i = 0; i < blockSz - 1; ++i)
    {
        block[i].nextFree = block + i + 1;
    }
    block[blockSz - 1].nextFree = nullptr;
    blocks.push_back(block);
    freeMemory = blocks[blocks.size() - 1];
}

template <size_t chunkSize>
FixedAllocator<chunkSize>::FixedAllocator()
{
    addBlock();
}

template <size_t chunkSize>
FixedAllocator<chunkSize>::~FixedAllocator()
{
    for (auto ptr : blocks)
    {
        delete ptr;
    }
}

template <size_t chunkSize>
void* FixedAllocator<chunkSize>::allocate()
{
    Chunk* memory = freeMemory;
    if (freeMemory -> nextFree != nullptr)
    {
        freeMemory = freeMemory -> nextFree;
    }
    else
    {
        addBlock();
    }
    return reinterpret_cast<void*>(memory);
}

template <size_t chunkSize>
void FixedAllocator<chunkSize>::deallocate(void* ptr)
{
    Chunk* newFreeMemory = reinterpret_cast<Chunk*> (ptr);
    newFreeMemory -> nextFree = freeMemory;
    freeMemory = newFreeMemory;
}

//////////////////////////////////////////////////////////
template<typename T>
struct FastAllocator
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;

    template <typename U>
    struct rebind
    {
        typedef FastAllocator<U> other;
    };

    FastAllocator() = default;
    FastAllocator(const FastAllocator&) {}
    template<typename U>
    FastAllocator(const FastAllocator<U>&) {}
    ~FastAllocator() = default;

    T* allocate(size_t n);
    void deallocate(T *ptr, size_t n);
    template<typename... Args>
    void construct(T *ptr, const Args &... args);
    void destroy(T *ptr);

    bool operator==(const FastAllocator& another);
    bool operator!=(const FastAllocator& another);
    FastAllocator<T>& operator=(const FastAllocator<T>&);

private:
    FixedAllocator <sizeof(T)> fixAlloc;
};

template<typename T>
T* FastAllocator<T>::allocate(size_t n)
{
    if (n == 1)
    {
        return reinterpret_cast<T*>(fixAlloc.allocate());
    }
    return reinterpret_cast<T*>(operator new(n * sizeof(T)));
}

template<typename T>
void FastAllocator<T>::deallocate(T *ptr, size_t n)
{
    if (n == 1)
    {
        fixAlloc.deallocate(ptr);
    }
    else
    {
        operator delete(ptr);
    }
}

template<typename T>
template<typename... Args>
void FastAllocator<T>::construct(T *ptr, const Args &... args)
{
    new(ptr) T(args...);
}

template<typename T>
void FastAllocator<T>::destroy(T *ptr)
{
    ptr->~T();
}

template<typename T>
bool FastAllocator<T>::operator==(const FastAllocator& another)
{
    return (fixAlloc == another.fixAlloc);
}

template<typename T>
bool FastAllocator<T>::operator!=(const FastAllocator& another)
{
    return !(*this == another);
}

template<typename T>
FastAllocator<T>& FastAllocator<T>::operator=(const FastAllocator<T>&)
{
    return *this;
}

///////////////////////////////////////////////////////////////////
template<typename T, typename Allocator =  FastAllocator<T>>
class List
{
private:
    struct Node
    {
        explicit Node(const T& val): value(val) {}

        Node () = default;

        T value;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    Node* head;
    Node* fakeTail;
    size_t sz = 0;

    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<Node> AllocNode;
    AllocNode alloc;

    using AllocTraits = std::allocator_traits<AllocNode>;

    void createFakeTail();

public:
    explicit List(const Allocator& alloc = Allocator());
    explicit List(size_t count, const Allocator& alloc = Allocator());
    explicit List(size_t count, const T& value, const Allocator& alloc = Allocator());
    List (const List& another);
    ~List();

    template <bool isConst>
    class common_iterator
    {
    public:
        friend List;

        using difference_type = std::ptrdiff_t;
        using value_type = typename std::conditional<isConst, const T, T>::type;
        using pointer = typename std::conditional<isConst, const T*, T*>::type;
        using reference = typename std::conditional<isConst, const T&, T&>::type;
        using iterator_category = std::bidirectional_iterator_tag;

        common_iterator(const common_iterator& another);
        ~common_iterator() = default;

        std::conditional_t<isConst, const T&, T&> operator*() const;
        std::conditional_t<isConst, const T*, T*> operator->();
        common_iterator& operator++();
        common_iterator operator++(int);
        common_iterator& operator--();
        common_iterator operator--(int);
        common_iterator& operator=(const common_iterator& another);

        bool operator==(const common_iterator &another) const;
        bool operator!=(const common_iterator &another) const;

        operator common_iterator<true>() const;

    private:
        Node* ptr = nullptr;

        explicit common_iterator(Node* p);
        Node* getPointer() const;
    };
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

    List& operator=(const List& another);
    AllocNode get_allocator() const;
    void insert(const_iterator it, const T& val);
    void erase(const_iterator it);
    void push_back(const T &value);
    void push_front(const T& value);
    void pop_back();
    void pop_front();
    size_t size() const;
};

//////////////////////////////////////////
template<typename T, typename Allocator>
void List<T, Allocator>::createFakeTail()
{
    fakeTail = AllocTraits::allocate(alloc, 1);
    fakeTail -> next = nullptr;
    fakeTail -> prev = nullptr;
    head = fakeTail;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const Allocator& allocator) : sz(0), alloc(allocator)
{
    createFakeTail();
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& allocator) : List(allocator)
{
    for (size_t i = 0; i < count; ++i)
    {
        Node *newNode = AllocTraits::allocate(alloc, 1);
        AllocTraits::construct(alloc, newNode);

        newNode->next = fakeTail;
        newNode->prev = fakeTail->prev;
        newNode->next->prev = newNode;

        if (newNode->prev != nullptr)
        {
            newNode->prev->next = newNode;
        }
        else
        {
            head = newNode;
        }
        sz++;
    }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc) : List(alloc)
{
    for (size_t i = 0; i < count; ++i)
    {
        push_back(value);
    }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const List& another) : List(std::allocator_traits<Allocator>::select_on_container_copy_construction(another.alloc))
{
    Node *node = another.head;
    for (size_t i = 0; i < another.sz; ++i)
    {
        push_back(node->value);
        node = node->next;
    }
}

template<typename T, typename Allocator>
typename List<T, Allocator>::List& List<T, Allocator>::operator=(const List& another)
{
    size_t cnt = sz;
    for (size_t i = 0; i < cnt; ++i)
    {
        pop_back();
    }

    if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value)
    {
        alloc = another.alloc;
    }
    fakeTail -> next = nullptr;
    fakeTail -> prev = nullptr;
    head = fakeTail;
    sz = 0;
    Node* node = another.head;
    for (size_t i = 0; i < another.sz; ++i)
    {
        push_back(node -> value);
        node = node -> next;
    }
    return *this;
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
    AllocTraits::deallocate(alloc, fakeTail, 1);
}

template<typename T, typename Allocator>
void List<T, Allocator>::insert(const_iterator it, const T& val)
{
    Node* newNode = AllocTraits::allocate(alloc, 1);
    AllocTraits::construct(alloc, newNode, val);

    newNode -> next = it.getPointer();
    newNode -> prev = it.getPointer() -> prev;
    newNode -> next -> prev = newNode;

    if (newNode -> prev != nullptr)
    {
        newNode -> prev -> next = newNode;
    }
    else
    {
        head = newNode;
    }
    sz++;
}

template<typename T, typename Allocator>
void List<T, Allocator>::erase(const_iterator it)
{
    Node* node = it.getPointer();

    node -> next -> prev = node -> prev;
    if (node -> prev != nullptr)
    {
        node -> prev -> next = node -> next;
    }
    else
    {
        head = node -> next;
    }
    AllocTraits::destroy(alloc, node);
    AllocTraits::deallocate(alloc, node, 1);
    sz--;
}

template<typename T, typename Allocator>
void List<T, Allocator>::push_back(const T &value)
{
    insert(cend(), value);
}

template<typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value)
{
    insert(cbegin(), value);
}

template<typename T, typename Allocator>
void List<T, Allocator>::pop_back()
{
    const_iterator it = cend();
    it--;
    erase(it);
}

template<typename T, typename Allocator>
void List<T, Allocator>::pop_front()
{
    erase(cbegin());
}

template<typename T, typename Allocator>
size_t List<T, Allocator>::size() const
{
    return sz;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::AllocNode List<T, Allocator>::get_allocator() const
{
    return alloc;
}

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
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin()
{
    return std::make_reverse_iterator(end());
}

template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend()
{
    return std::make_reverse_iterator(begin());
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin() const
{
    return std::make_reverse_iterator(end());
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend() const
{
    return std::make_reverse_iterator(begin());
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crbegin() const
{
    return std::make_reverse_iterator(end());
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend() const
{
    return std::make_reverse_iterator(begin());
}

///////////////////////////////////////////////
template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(Node* p)
{
    ptr = p;
}

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(const common_iterator& another)
{
    ptr = another.ptr;
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
    return ptr;
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

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::operator common_iterator<true>() const
{
    return common_iterator<true>(ptr);
}