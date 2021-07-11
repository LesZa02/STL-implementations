#include <iostream>
#include <type_traits>

template<typename... Types>
class Tuple;
///////////////////////////////////////////////////

template<>
class Tuple<>
{
public:
    Tuple() = default;
    Tuple(const Tuple&) {}
    Tuple(Tuple&&) {}
    ~Tuple() = default;

    void swap(Tuple&) {}
    Tuple& operator=(const Tuple&);
    Tuple& operator=(Tuple&&);

    bool operator==(const Tuple&) const;
    bool operator!=(const Tuple&) const;
    bool operator<(const Tuple&) const;
    bool operator>(const Tuple&) const;
    bool operator<=(const Tuple&) const;
    bool operator>=(const Tuple&) const;
};
Tuple<>& Tuple<>::operator=(const Tuple<>&)
{
    return *this;
}
Tuple<>& Tuple<>::operator=(Tuple<>&&)
{
    return *this;
}
bool Tuple<>::operator==(const Tuple<>&) const
{
    return true;
}
bool Tuple<>::operator!=(const Tuple<>&) const
{
    return false;
}
bool Tuple<>::operator<(const Tuple<>&) const
{
    return false;
}
bool Tuple<>::operator>(const Tuple<>&) const
{
    return false;
}
bool Tuple<>::operator<=(const Tuple<>&) const
{
    return true;
}
bool Tuple<>::operator>=(const Tuple<>&) const
{
    return true;
}
///////////////////////////////////////////////////

template <typename Type>
struct TupleType;

template <typename Head, typename... Tail>
struct TupleType<const Tuple<Head, Tail...>&>;

template <typename Head, typename... Tail>
struct TupleType<Tuple<Head, Tail...>&>;

template <typename Head, typename... Tail>
struct TupleType<Tuple<Head, Tail...>&&>;

template <typename Head, typename... Tail>
struct TupleType<Tuple<Head, Tail...>>;
//////////////////////////////////////////

template <typename HeadType, typename... TailTypes>
class Tuple<HeadType, TailTypes...>
{
public:
    Tuple() : firstElem(), otherElem() {}

    Tuple(const HeadType& value, const TailTypes&... args) :
            firstElem(value),
            otherElem(args...) {}

    template<typename Type, typename... OtherTypes,
            typename = typename std::enable_if<sizeof...(TailTypes) == sizeof...(OtherTypes)>::type>
    explicit Tuple(Type&& value, OtherTypes&&... args) :
            firstElem(std::forward<Type>(value)),
            otherElem(std::forward<OtherTypes>(args)...) {}

    Tuple(const Tuple& other) :
            firstElem(other.firstElem),
            otherElem(other.otherElem) {}

    Tuple(Tuple&& other) :
            firstElem(std::forward<typename TupleType<Tuple>::FirstType>(other.firstElem)),
            otherElem(std::forward<typename TupleType<Tuple>::OtherType>(other.otherElem)) {}

    template<typename Type, typename... OtherTypes>
    explicit Tuple(const Tuple<Type, OtherTypes...>& other) :
            firstElem(other.firstElem),
            otherElem(other.otherElem) {}

    template<typename Type, typename... OtherTypes>
    explicit Tuple(Tuple<Type, OtherTypes...>&& other) :
            firstElem(std::forward<Type>(other.firstElem)),
            otherElem(std::forward<OtherTypes...>(other.otherElem)) {}

    ~Tuple() = default;

    template<typename Type, typename... OtherTypes>
    void swap(Tuple<Type, OtherTypes...>& other);
    Tuple& operator=(const Tuple& other);
    Tuple& operator=(Tuple&& other);
    template<typename Type, typename... OtherTypes>
    Tuple& operator=(const Tuple<Type, OtherTypes...>& other);
    template<typename Type, typename... OtherTypes>
    Tuple& operator=(Tuple<Type, OtherTypes...>&& other);

    bool operator==(const Tuple& other) const;
    bool operator!=(const Tuple& other) const;
    bool operator<(const Tuple& other) const;
    bool operator>(const Tuple& other) const;
    bool operator<=(const Tuple& other) const;
    bool operator>=(const Tuple& other) const;
    /////////////////////////////////////////////////////

    template <typename FirstType, typename SecondType, typename... OtherTypes>
    friend class GetType;

    template <size_t ind, typename Type, typename... OtherTypes>
    friend class GetInd;

    template<typename FirstTuple, typename SecondTuple, typename>
    friend auto tupleCat(FirstTuple&& firstTuple, SecondTuple&& secondTuple);

    template <typename... OtherTypes, typename Type>
    friend auto addElemFront(Type&& value, Tuple<OtherTypes...>&& tuple);

    template <typename... OtherTypes, typename Type>
    friend auto addElemFront(Type&& value, const Tuple<OtherTypes...>& tuple);

    HeadType& getFirstElement() &
    {
        return firstElem;
    }
    HeadType&& getFirstElement() &&
    {
        return std::move(firstElem);
    }
    const HeadType& getFirstElement() const &
    {
        return firstElem;
    }
    Tuple<TailTypes...>& getOtherElements() &
    {
        return otherElem;
    }
    Tuple<TailTypes...>&& getOtherElements() &&
    {
        return std::move(otherElem);
    }
    const Tuple<TailTypes...>& getOtherElements() const &
    {
        return otherElem;
    }

private:
    HeadType firstElem;
    Tuple<TailTypes...> otherElem;

    template <typename Type>
    Tuple(Type&& value, const Tuple<TailTypes...>& tuple) :
            firstElem(std::forward<Type>(value)),
            otherElem(tuple) {}

    template <typename Type>
    Tuple(Type&& value, Tuple<TailTypes...>&& tuple) :
            firstElem(std::forward<Type>(value)),
            otherElem(std::move(tuple)) {}
};
///////////////////////////////////////////////////

template <typename... OtherTypes, typename Type>
auto addElemFront(Type&& value, const Tuple<OtherTypes...>& tuple)
{
    return Tuple<std::remove_reference_t<Type>, OtherTypes...>(std::forward<Type>(value), tuple);
}
template <typename... OtherTypes, typename Type>
auto addElemFront(Type&& value, Tuple<OtherTypes...>&& tuple)
{
    return Tuple<std::remove_reference_t<Type>, OtherTypes...>(std::forward<Type>(value), std::move(tuple));
}
////////////////////////////////////////////////////////////////////

template <typename Type>
struct TupleType;

template <typename Type, typename... OtherTypes>
struct TupleType<const Tuple<Type, OtherTypes...>&>
{
    using FirstType = Type&;
    using OtherType = Tuple<OtherTypes...>&;
};

template <typename Type, typename... OtherTypes>
struct TupleType<Tuple<Type, OtherTypes...>&>
{
    using FirstType = Type&;
    using OtherType = Tuple<OtherTypes...>&;
};

template <typename Type, typename... OtherTypes>
struct TupleType<Tuple<Type, OtherTypes...>&&>
{
    using FirstType = Type&&;
    using OtherType = Tuple<OtherTypes...>&&;
};

template <typename Type, typename... OtherTypes>
struct TupleType<Tuple<Type, OtherTypes...>>
{
    using FirstType = Type&&;
    using OtherType = Tuple<OtherTypes...>&&;
};
////////////////////////////////////////////////////////////////////

template <typename Head, typename... Tail>
template<typename Type, typename... OtherTypes>
void Tuple<Head, Tail...>::swap(Tuple<Type, OtherTypes...>& other)
{
    std::swap(firstElem, other.getFirstElement());
    otherElem.swap(other.getOtherElements());
}

template <typename Head, typename... Tail>
Tuple<Head, Tail...>& Tuple<Head, Tail...>::operator=(const Tuple& other)
{
    Tuple copy = other;
    swap(copy);
    return *this;
}

template <typename Head, typename... Tail>
Tuple<Head, Tail...>& Tuple<Head, Tail...>::operator=(Tuple&& other)
{
    Tuple copy = std::move(other);
    swap(copy);
    return *this;
}

template <typename Head, typename... Tail>
template<typename Type, typename... OtherTypes>
Tuple<Head, Tail...>& Tuple<Head, Tail...>::operator=(const Tuple<Type, OtherTypes...>& other)
{
    Tuple<Type, OtherTypes...> copy = other;
    swap(copy);
    return *this;
}

template <typename Head, typename... Tail>
template<typename Type, typename... OtherTypes>
Tuple<Head, Tail...>& Tuple<Head, Tail...>::operator=(Tuple<Type, OtherTypes...>&& other)
{
    Tuple<Type, OtherTypes...> copy = std::move(other);
    swap(copy);
    return *this;
}

template <typename Head, typename... Tail>
bool Tuple<Head, Tail...>::operator==(const Tuple& other) const
{
    if(firstElem != other.firstElem)
    {
        return false;
    }
    return otherElem == other.otherElem;
}

template <typename Head, typename... Tail>
bool Tuple<Head, Tail...>::operator!=(const Tuple& other) const
{
    return !(*this == other);
}

template <typename Head, typename... Tail>
bool Tuple<Head, Tail...>::operator<(const Tuple& other) const
{
    if(firstElem != other.firstElem)
    {
        return firstElem < other.firstElem;
    }
    return otherElem < other.otherElem;
}

template <typename Head, typename... Tail>
bool Tuple<Head, Tail...>::operator>(const Tuple& other) const
{
    if(firstElem != other.firstElem)
    {
        return firstElem > other.firstElem;
    }
    return otherElem > other.otherElem;
}

template <typename Head, typename... Tail>
bool Tuple<Head, Tail...>::operator<=(const Tuple& other) const
{
    return !(*this > other);
}

template <typename Head, typename... Tail>
bool Tuple<Head, Tail...>::operator>=(const Tuple& other) const
{
    return !(*this < other);
}
//////////////////////////////////////////////////

template<typename ...Types>
Tuple<typename std::decay_t<Types>...> makeTuple(Types&& ...args)
{
    return Tuple<typename std::decay_t<Types>...>(std::forward<Types>(args)...);
}

template<typename Tuple>
auto tupleCat(Tuple&& tuple)
{
    return std::forward<Tuple>(tuple);
}

template<typename SecondTuple>
auto tupleCat(Tuple<>&&, SecondTuple&& secondTuple)
{
    return std::forward<SecondTuple>(secondTuple);
}

template<typename SecondTuple>
auto tupleCat(const Tuple<>&, SecondTuple&& secondTuple)
{
    return std::forward<SecondTuple>(secondTuple);
}

template<typename FirstTuple, typename SecondTuple, typename = typename std::enable_if<sizeof(FirstTuple) != sizeof(Tuple<>)>::type>
auto tupleCat(FirstTuple&& firstTuple, SecondTuple&& secondTuple)
{
    return addElemFront(std::forward<typename TupleType<FirstTuple>::FirstType>(firstTuple.firstElem),
                        tupleCat(std::forward<typename TupleType<FirstTuple>::OtherType>(firstTuple.otherElem), std::forward<SecondTuple>(secondTuple)));
}

template<typename FirstTuple, typename SecondTuple, typename... Tuples, typename = typename std::enable_if<sizeof...(Tuples) != 0>::type>
auto tupleCat(FirstTuple&& firstTuple, SecondTuple&& secondTuple, Tuples&&... other)
{
    return tupleCat(std::forward<FirstTuple>(firstTuple),
                    tupleCat(std::forward<SecondTuple>(secondTuple), std::forward<Tuples>(other)...));
}
///////////////////////////////////////////////////////

template <size_t ind, typename Type, typename... OtherTypes>
class GetInd
{
public:
    static decltype(auto) get(Tuple<Type, OtherTypes...>& tuple)
    {
        return GetInd<ind - 1, OtherTypes...>::get(tuple.getOtherElements());
    }
    static decltype(auto) get(const Tuple<Type, OtherTypes...>& tuple)
    {
        return GetInd<ind - 1, OtherTypes...>::get(tuple.getOtherElements());
    }
    static decltype(auto) get(Tuple<Type, OtherTypes...>&& tuple)
    {
        return GetInd<ind - 1, OtherTypes...>::get(std::move(tuple.getOtherElements()));
    }
    static decltype(auto) get(const Tuple<Type, OtherTypes...>&& tuple)
    {
        return GetInd<ind - 1, OtherTypes...>::get(tuple.getOtherElements());
    }
};

template <typename Type, typename... OtherTypes>
class GetInd<0, Type, OtherTypes...>
{
public:
    static decltype(auto) get(Tuple<Type, OtherTypes...>& tuple)
    {
        return tuple.getFirstElement();
    }
    static decltype(auto) get(const Tuple<Type, OtherTypes...>& tuple)
    {
        return tuple.getFirstElement();
    }
    static decltype(auto) get(Tuple<Type, OtherTypes...>&& tuple)
    {
        return static_cast<Tuple<Type, OtherTypes...>&&>(tuple).getFirstElement();
    }
    static decltype(auto) get(const Tuple<Type, OtherTypes...>&& tuple)
    {
        return tuple.getFirstElement();
    }
};
///////////////////////////////////////////////////////////////////////////

template<size_t I, typename... Types>
decltype(auto) get(Tuple<Types...>& tuple)
{
    return GetInd<I, Types...>::get(tuple);
}
template<size_t I, typename... Types>
decltype(auto) get(const Tuple<Types...>& tuple)
{
    return GetInd<I, Types...>::get(tuple);
}
template<size_t I, typename... Types>
decltype(auto) get(Tuple<Types...>&& tuple)
{
    return GetInd<I, Types...>::get(std::move(tuple));
}
template<size_t I, typename... Types>
decltype(auto) get(const Tuple<Types...>&& tuple)
{
    return GetInd<I, Types...>::get(tuple);
}
/////////////////////////////////////////////////////////

template <typename FirstType, typename SecondType, typename... OtherTypes>
class GetType
{
public:
    static decltype(auto) get(Tuple<SecondType, OtherTypes...>& tuple)
    {
        return GetType<FirstType, OtherTypes...>::get(tuple.otherElem);
    }
    static decltype(auto) get(const Tuple<SecondType, OtherTypes...>& tuple)
    {
        return GetType<FirstType, OtherTypes...>::get(tuple.otherElem);
    }
    static decltype(auto) get(Tuple<SecondType, OtherTypes...>&& tuple)
    {
        return GetType<FirstType, OtherTypes...>::get(std::move(tuple.otherElem));
    }
    static decltype(auto) get(const Tuple<SecondType, OtherTypes...>&& tuple)
    {
        return GetType<FirstType, OtherTypes...>::get(tuple.otherElem);
    }
};

template <typename FirstType, typename... OtherTypes>
class GetType<FirstType, FirstType, OtherTypes...>
{
public:
    static FirstType& get(Tuple<FirstType, OtherTypes...>& tuple)
    {
        return tuple.getFirstElement();;
    }
    static const FirstType& get(const Tuple<FirstType, OtherTypes...>& tuple)
    {
        return tuple.getFirstElement();;
    }
    static FirstType&& get(Tuple<FirstType, OtherTypes...>&& tuple)
    {
        return static_cast <Tuple<FirstType, OtherTypes...>&&>(tuple).getFirstElement();
    }
    static const FirstType&& get(const Tuple<FirstType, OtherTypes...>&& tuple)
    {
        return tuple.getFirstElement();;
    }
};
////////////////////////////////////////////////////////

template<typename T, typename... Types>
decltype(auto) get(Tuple<Types...>& tuple)
{
    return GetType<T, Types...>::get(tuple);
}
template<typename T, typename... Types>
decltype(auto) get(const Tuple<Types...>& tuple)
{
    return GetType<T, Types...>::get(tuple);
}
template<typename T, typename... Types>
decltype(auto) get(Tuple<Types...>&& tuple)
{
    return GetType<T, Types...>::get(std::move(tuple));
}
template<typename T, typename... Types>
decltype(auto) get(const Tuple<Types...>&& tuple)
{
    return GetType<T, Types...>::get(tuple);
}