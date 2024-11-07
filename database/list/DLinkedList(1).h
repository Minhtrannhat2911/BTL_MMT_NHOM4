/*
 * File:   DLinkedList.h
 */

#ifndef DLINKEDLIST_H
#define DLINKEDLIST_H

#include "list/IList.h"

#include <sstream>
#include <iostream>
#include <type_traits>
using namespace std;

template <class T>
class DLinkedList : public IList<T>
{
public:
    class Node;        // Forward declaration
    class Iterator;    // Forward declaration
    class BWDIterator; // Forward declaration

protected:
    Node *head; // this node does not contain user's data
    Node *tail; // this node does not contain user's data
    int count;
    bool (*itemEqual)(T &lhs, T &rhs);        // function pointer: test if two items (type: T&) are equal or not
    void (*deleteUserData)(DLinkedList<T> *); // function pointer: be called to remove items (if they are pointer type)

public:
    DLinkedList(
        void (*deleteUserData)(DLinkedList<T> *) = 0,
        bool (*itemEqual)(T &, T &) = 0);
    DLinkedList(const DLinkedList<T> &list);
    DLinkedList<T> &operator=(const DLinkedList<T> &list);
    ~DLinkedList();

    // Inherit from IList: BEGIN
    void add(T e);
    void add(int index, T e);
    T removeAt(int index);
    bool removeItem(T item, void (*removeItemData)(T) = 0);
    int findItemIndex(T item) ;
    bool performRemove(int index, void (*removeItemData)(T)) ;
    bool empty();
    int size();
    void clear();
    T &get(int index);
    int indexOf(T item);
    bool contains(T item);
    bool containsHelper(T item, Node* curr) ;
    string toString(string (*item2str)(T &) = 0);
    // Inherit from IList: END

    void println(string (*item2str)(T &) = 0)
    {
        cout << toString(item2str) << endl;
    }
    void setDeleteUserDataPtr(void (*deleteUserData)(DLinkedList<T> *) = 0)
    {
        this->deleteUserData = deleteUserData;
    }
    bool containsHelper(typename DLinkedList<T>::Iterator it, T array[], int idx, int size) {
    if (idx == size) {
        return true;
    }
    if (!equals(*it, array[idx], this->itemEqual)) {
        return false;
    }
    ++it;       
    idx++;       

    return containsHelper(it, array, idx, size);
    }

    bool contains(T array[], int size) {
    return containsHelper(begin(), array, 0, size);
    }   

    /*
     * free(DLinkedList<T> *list):
     *  + to remove user's data (type T, must be a pointer type, e.g.: int*, Point*)
     *  + if users want a DLinkedList removing their data,
     *      he/she must pass "free" to constructor of DLinkedList
     *      Example:
     *      DLinkedList<T> list(&DLinkedList<T>::free);
     */
    static void free(DLinkedList<T> *list)
    {
        typename DLinkedList<T>::Iterator it = list->begin();
        while (it != list->end())
        {
            delete *it;
            it++;
        }
    }

    /* begin, end and Iterator helps user to traverse a list forwardly
     * Example: assume "list" is object of DLinkedList

     DLinkedList<char>::Iterator it;
     for(it = list.begin(); it != list.end(); it++){
            char item = *it;
            std::cout << item; //print the item
     }
     */
    Iterator begin()
    {
        return Iterator(this, true);
    }
    Iterator end()
    {
        return Iterator(this, false);
    }

    /* last, beforeFirst and BWDIterator helps user to traverse a list backwardly
     * Example: assume "list" is object of DLinkedList

     DLinkedList<char>::BWDIterator it;
     for(it = list.last(); it != list.beforeFirst(); it--){
            char item = *it;
            std::cout << item; //print the item
     }
     */
    BWDIterator bbegin()
    {
        return BWDIterator(this, true);
    }
    BWDIterator bend()
    {
        return BWDIterator(this, false);
    }
    BWDIterator last() 
    {
        return BWDIterator(this, true);
    }
    BWDIterator beforeFirst() 
    {
        return BWDIterator(this, false);
    }

private:
    template<typename U, typename = void>
    struct has_equal_operator : std::false_type {};

    template<typename U>
    struct has_equal_operator<U, std::void_t<decltype(std::declval<U>() == std::declval<U>())>> : std::true_type {};

    template<typename U, typename = void>
    struct has_ostream_operator : std::false_type {};

    template<typename U>
    struct has_ostream_operator<U, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<U>())>> : std::true_type {};
static void copy(DLinkedList<T> *dest, const DLinkedList<T> *src) {
    for (Node *pNode = src->head->next, *pPrev = dest->head; pNode != src->tail; pNode = pNode->next) {
        Node *newNode = new Node(pNode->data);
        pPrev->next = newNode;
        newNode->prev = pPrev;
        newNode->next = dest->tail;
        dest->tail->prev = newNode;
        pPrev = newNode;
    }
}

protected:
    static bool equals(T &lhs, T &rhs, bool (*itemEqual)(T &, T &))
    {
        if (itemEqual == 0)
            return lhs == rhs;
        else
            return itemEqual(lhs, rhs);
    }
    void copyFrom(const DLinkedList<T> &list);
    void removeInternalData();
    Node *getPreviousNodeOf(int index);
    Node* traverseBackward(int index);
    Node* traverseForward(int index) ;
    void initializeList() ;
    void assignProperties(const DLinkedList<T> &list) ;

    //////////////////////////////////////////////////////////////////////
    ////////////////////////  INNER CLASSES DEFNITION ////////////////////
    //////////////////////////////////////////////////////////////////////
public:
    class Node
    {
    public:
        T data;
        Node *next;
        Node *prev;
        friend class DLinkedList<T>;

    public:
        Node(Node *next = 0, Node *prev = 0)
        {
            this->next = next;
            this->prev = prev;
        }
        Node(T data, Node *next = 0, Node *prev = 0)
        {
            this->data = data;
            this->next = next;
            this->prev = prev;
        }
    };

    //////////////////////////////////////////////////////////////////////
    class Iterator
    {
    private:
        DLinkedList<T> *pList;
        Node *pNode;

    public:
        Iterator(DLinkedList<T> *pList = 0, bool begin = true)
        {
            if (begin)
            {
                if (pList != 0)
                    this->pNode = pList->head->next;
                else
                    pNode = 0;
            }
            else
            {
                if (pList != 0)
                    this->pNode = pList->tail;
                else
                    pNode = 0;
            }
            this->pList = pList;
        }

        Iterator &operator=(const Iterator &iterator)
        {
            this->pNode = iterator.pNode;
            this->pList = iterator.pList;
            return *this;
        }
        void remove(void (*removeItemData)(T) = 0)
        {
            pNode->prev->next = pNode->next;
            pNode->next->prev = pNode->prev;
            Node *pNext = pNode->prev; // MUST prev, so iterator++ will go to end
            if (removeItemData != 0)
                removeItemData(pNode->data);
            delete pNode;
            pNode = pNext;
            pList->count -= 1;
        }

        T &operator*()
        {
            return pNode->data;
        }
        bool operator!=(const Iterator &iterator)
        {
            return pNode != iterator.pNode;
        }
        // Prefix ++ overload
        Iterator &operator++()
        {
            pNode = pNode->next;
            return *this;
        }
        // Postfix ++ overload
        Iterator operator++(int)
        {
            Iterator iterator = *this;
            ++*this;
            return iterator;
        }
    };
    class BWDIterator {
    private:
        DLinkedList<T> *pList;
        Node *pNode;

    public:
BWDIterator(DLinkedList<T> *pList = 0, bool last = true) {
    this->pList = pList;

    Node* nodeOptions[2][2] = {
        {nullptr, nullptr},                              
        {pList->head, pList->tail->prev}                 
    };

    this->pNode = nodeOptions[pList != nullptr][last];
}

        BWDIterator &operator=(const BWDIterator &iterator) {
            this->pNode = iterator.pNode;
            this->pList = iterator.pList;
            return *this;
        }
        void remove(void (*removeItemData)(T) = 0) {
            Node *temp = pNode->next;
            pList->removeItem(pNode->data, removeItemData);
            pNode = temp;
        }
        T &operator*() {
            return pNode->data;
        }
        bool operator!=(const BWDIterator &iterator) {
            return pNode != iterator.pNode;
        }
        BWDIterator &operator--() {
            pNode = pNode->prev;
            return *this;
        }

BWDIterator operator--(int) {
    BWDIterator iterator = *this;
    decrementIterator();
    return iterator;
}

void decrementIterator() {
    --*this;
}

    };
};
//////////////////////////////////////////////////////////////////////
// Define a shorter name for DLinkedList:

template <class T>
using List = DLinkedList<T>;

//////////////////////////////////////////////////////////////////////
////////////////////////     METHOD DEFNITION      ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
DLinkedList<T>::DLinkedList(
    void (*deleteUserData)(DLinkedList<T> *),
    bool (*itemEqual)(T &, T &))
{
    // Khá»Ÿi táº¡o danh sÃ¡ch trá»‘ng
    this->head = nullptr;
    this->tail = nullptr;
    this->count = 0;

    // GÃ¡n cÃ¡c hÃ m xÃ³a dá»¯ liá»‡u vÃ  so sÃ¡nh pháº§n tá»­
    this->deleteUserData = deleteUserData;
    this->itemEqual = itemEqual;
}



template <class T>
DLinkedList<T>::DLinkedList(const DLinkedList<T> &list) {
    initializeList();
    assignProperties(list);
    copy(this, &list);
}
template <class T>
void DLinkedList<T>::initializeList() {
    head = new Node();
    tail = new Node();
    head->next = tail;
    tail->prev = head;
}

template <class T>
void DLinkedList<T>::assignProperties(const DLinkedList<T> &list) {
    deleteUserData = list.deleteUserData;
    itemEqual = list.itemEqual;
    count = list.count;
}

template <class T>
DLinkedList<T> &DLinkedList<T>::operator=(const DLinkedList<T> &list) {
    void* actions[] = {
        reinterpret_cast<void*>(&deleteUserData),   
        reinterpret_cast<void*>(&head),            
        reinterpret_cast<void*>(&tail)             
    };

    int conditions[] = {
        (this == &list) ? 1 : 0,                             
        (count > 0 && head != list.head && tail != list.tail) ? 1 : 0,  
        (deleteUserData != nullptr) ? 1 : 0                  
    };

    (conditions[0]) ? static_cast<void>(0) : static_cast<void>(0);

    (conditions[1]) 
        ? ((*reinterpret_cast<void(*)(DLinkedList<T>*)>(actions[0]))(this), delete head, delete tail)
        : static_cast<void>(0);

    deleteUserData = list.deleteUserData;
    itemEqual = list.itemEqual;
    count = list.count;

    head = new Node();
    tail = new Node();
    head->next = tail;
    tail->prev = head;

    copy(this, &list);

    return *this;
}


template <class T>
DLinkedList<T>::~DLinkedList() {
    clear();  // Gá»i phÆ°Æ¡ng thá»©c clear Ä‘á»ƒ giáº£i phÃ³ng táº¥t cáº£ cÃ¡c nÃºt
}

template <class T>
void DLinkedList<T>::add(T e) {
    Node* newNode = new Node(e);  // Táº¡o nÃºt má»›i vá»›i dá»¯ liá»‡u

    if (count == 0) {
        // Náº¿u danh sÃ¡ch rá»—ng, cáº­p nháº­t head vÃ  tail
        head = tail = newNode; 
        head->prev = nullptr; // Äáº£m báº£o con trá» prev cá»§a head lÃ  nullptr
        tail->next = nullptr; // Äáº£m báº£o con trá» next cá»§a tail lÃ  nullptr
    } else {
        // GÃ¡n con trá» prev cá»§a nÃºt má»›i
        newNode->prev = tail;  
        tail->next = newNode;  // GÃ¡n con trá» next cá»§a nÃºt tail hiá»‡n táº¡i thÃ nh nÃºt má»›i
        newNode->next = nullptr; // Äáº£m báº£o con trá» next cá»§a nÃºt má»›i lÃ  nullptr
        tail = newNode;        // Cáº­p nháº­t tail thÃ nh nÃºt má»›i
    }

    count++;  // TÄƒng sá»‘ lÆ°á»£ng pháº§n tá»­
}

template <class T>
void DLinkedList<T>::add(int index, T e) {
    // Kiá»ƒm tra chá»‰ má»¥c há»£p lá»‡
    if (index < 0 || index > count) {
        throw std::out_of_range("Index out of range");  // NÃ©m ngoáº¡i lá»‡ náº¿u chá»‰ má»¥c khÃ´ng há»£p lá»‡
    }

    Node* newNode = new Node(e);  // Táº¡o nÃºt má»›i vá»›i dá»¯ liá»‡u

    // ThÃªm vÃ o Ä‘áº§u danh sÃ¡ch
    if (index == 0) {
        if (count == 0) {  // Danh sÃ¡ch rá»—ng
            head = tail = newNode;  // Cáº­p nháº­t head vÃ  tail
            head->prev = nullptr; // Äáº£m báº£o con trá» prev cá»§a head lÃ  nullptr
            tail->next = nullptr; // Äáº£m báº£o con trá» next cá»§a tail lÃ  nullptr
        } else {
            newNode->next = head;  // GÃ¡n nÃºt má»›i lÃ  nÃºt Ä‘áº§u
            head->prev = newNode;   // GÃ¡n con trá» prev cá»§a nÃºt Ä‘áº§u thÃ nh nÃºt má»›i
            head = newNode;         // Cáº­p nháº­t head
            head->prev = nullptr; // Äáº£m báº£o con trá» prev cá»§a head lÃ  nullptr
        }
    } 
    // ThÃªm vÃ o cuá»‘i danh sÃ¡ch
    else if (index == count) {
        newNode->prev = tail;  // GÃ¡n nÃºt trÆ°á»›c lÃ  tail hiá»‡n táº¡i
        newNode->next = nullptr; // Äáº£m báº£o con trá» next cá»§a nÃºt má»›i lÃ  nullptr
        if (tail) {
            tail->next = newNode;  // GÃ¡n con trá» next cá»§a tail thÃ nh nÃºt má»›i
        }
        tail = newNode;  // Cáº­p nháº­t tail
    } 
    // ThÃªm vÃ o giá»¯a danh sÃ¡ch
    else {
        Node* prevNode = getPreviousNodeOf(index);  // Láº¥y nÃºt trÆ°á»›c nÃºt táº¡i chá»‰ má»¥c
        newNode->next = prevNode->next;  // GÃ¡n next cá»§a nÃºt má»›i
        newNode->prev = prevNode;         // GÃ¡n prev cá»§a nÃºt má»›i
        if (prevNode->next != nullptr) { // Náº¿u nÃºt sau khÃ´ng pháº£i nullptr
            prevNode->next->prev = newNode; // Cáº­p nháº­t con trá» prev cá»§a nÃºt sau
        }
        prevNode->next = newNode;         // Cáº­p nháº­t con trá» next cá»§a nÃºt trÆ°á»›c
    }

    count++;  // TÄƒng sá»‘ lÆ°á»£ng pháº§n tá»­
}

template <class T>
typename DLinkedList<T>::Node *DLinkedList<T>::getPreviousNodeOf(int index) {
    // Kiá»ƒm tra chá»‰ má»¥c há»£p lá»‡
    if (index <= 0 || index >= count) {
        return nullptr;  // Náº¿u chá»‰ má»¥c khÃ´ng há»£p lá»‡, tráº£ vá» nullptr
    }

    Node* currentNode;

    // Náº¿u chá»‰ má»¥c náº±m trong ná»­a Ä‘áº§u cá»§a danh sÃ¡ch
    if (index < count / 2) {
        currentNode = head;  // Báº¯t Ä‘áº§u tá»« Ä‘áº§u danh sÃ¡ch
        for (int i = 0; i < index - 1; ++i) {
            currentNode = currentNode->next;  // Duyá»‡t Ä‘áº¿n nÃºt trÆ°á»›c nÃºt táº¡i chá»‰ má»¥c
        }
    } else {
        currentNode = tail;  // Báº¯t Ä‘áº§u tá»« cuá»‘i danh sÃ¡ch
        for (int i = count - 1; i > index - 1; --i) {
            currentNode = currentNode->prev;  // Duyá»‡t Ä‘áº¿n nÃºt trÆ°á»›c nÃºt táº¡i chá»‰ má»¥c
        }
    }

    return currentNode;  // Tráº£ vá» nÃºt trÆ°á»›c nÃºt táº¡i chá»‰ má»¥c Ä‘Ã£ cho
}

template <class T>
T DLinkedList<T>::removeAt(int index) {
    // Kiá»ƒm tra chá»‰ má»¥c há»£p lá»‡
    if (index < 0 || index >= count) {
        throw std::out_of_range("Index out of range");  // NÃ©m ngoáº¡i lá»‡ náº¿u chá»‰ má»¥c khÃ´ng há»£p lá»‡
    }

    Node* nodeToRemove;  // NÃºt sáº½ bá»‹ xÃ³a
    T removedData;       // Dá»¯ liá»‡u cá»§a nÃºt sáº½ bá»‹ xÃ³a

    // Duyá»‡t Ä‘áº¿n nÃºt cáº§n xÃ³a
    if (index == 0) {
        // Náº¿u lÃ  nÃºt Ä‘áº§u tiÃªn
        nodeToRemove = head;
        head = head->next;  // Cáº­p nháº­t head
        if (head) {         // Náº¿u danh sÃ¡ch khÃ´ng rá»—ng sau khi xÃ³a
            head->prev = nullptr;  // Cáº­p nháº­t con trá» prev cá»§a nÃºt má»›i Ä‘áº§u
        } else {
            tail = nullptr;  // Náº¿u danh sÃ¡ch rá»—ng, cáº­p nháº­t tail
        }
    } else if (index == count - 1) {
        // Náº¿u lÃ  nÃºt cuá»‘i cÃ¹ng
        nodeToRemove = tail;
        tail = tail->prev;  // Cáº­p nháº­t tail
        tail->next = nullptr;  // Cáº­p nháº­t con trá» next cá»§a nÃºt má»›i cuá»‘i
    } else {
        // Náº¿u lÃ  nÃºt á»Ÿ giá»¯a
        nodeToRemove = head;
        for (int i = 0; i < index; ++i) {
            nodeToRemove = nodeToRemove->next;  // Duyá»‡t Ä‘áº¿n nÃºt cáº§n xÃ³a
        }
        // Cáº­p nháº­t con trá» next vÃ  prev
        nodeToRemove->prev->next = nodeToRemove->next;
        nodeToRemove->next->prev = nodeToRemove->prev;
    }

    removedData = nodeToRemove->data;  // LÆ°u dá»¯ liá»‡u cá»§a nÃºt sáº½ bá»‹ xÃ³a
    delete nodeToRemove;  // Giáº£i phÃ³ng bá»™ nhá»› cá»§a nÃºt
    count--;  // Giáº£m sá»‘ lÆ°á»£ng pháº§n tá»­
    return removedData;  // Tráº£ vá» dá»¯ liá»‡u cá»§a nÃºt Ä‘Ã£ xÃ³a
}

template <class T>
bool DLinkedList<T>::empty() {
    return count == 0;  // Tráº£ vá» true náº¿u danh sÃ¡ch rá»—ng, false náº¿u khÃ´ng
}

template <class T>
int DLinkedList<T>::size() {
    return count;  // Tráº£ vá» sá»‘ lÆ°á»£ng pháº§n tá»­ trong danh sÃ¡ch
}

template <class T>
void DLinkedList<T>::clear() {
    Node* current = head;  // Báº¯t Ä‘áº§u tá»« nÃºt Ä‘áº§u tiÃªn

    // Duyá»‡t qua danh sÃ¡ch vÃ  xÃ³a tá»«ng nÃºt
    while (current != nullptr) {
        Node* nextNode = current->next;  // LÆ°u nÃºt tiáº¿p theo
        delete current;  // Giáº£i phÃ³ng bá»™ nhá»› cá»§a nÃºt hiá»‡n táº¡i
        current = nextNode;  // Di chuyá»ƒn Ä‘áº¿n nÃºt tiáº¿p theo
    }

    // Cáº­p nháº­t tráº¡ng thÃ¡i danh sÃ¡ch
    head = nullptr;  // Äáº·t head vá» nullptr
    tail = nullptr;  // Äáº·t tail vá» nullptr
    count = 0;       // Äáº·t sá»‘ lÆ°á»£ng pháº§n tá»­ vá» 0
}

#include <stdexcept> // Äá»ƒ sá»­ dá»¥ng std::out_of_range

template <class T>
T &DLinkedList<T>::get(int index) {
    // Kiá»ƒm tra chá»‰ má»¥c há»£p lá»‡
    if (index < 0 || index >= count) {
        throw std::out_of_range("Index out of range");  // NÃ©m ngoáº¡i lá»‡ náº¿u khÃ´ng há»£p lá»‡
    }

    Node* current = head;  // Báº¯t Ä‘áº§u tá»« nÃºt Ä‘áº§u tiÃªn

    // Duyá»‡t qua danh sÃ¡ch Ä‘áº¿n chá»‰ má»¥c yÃªu cáº§u
    for (int i = 0; i < index; ++i) {
        current = current->next;  // Di chuyá»ƒn Ä‘áº¿n nÃºt tiáº¿p theo
    }

    return current->data;  // Tráº£ vá» tham chiáº¿u Ä‘áº¿n dá»¯ liá»‡u cá»§a nÃºt táº¡i chá»‰ má»¥c
}

template <class T>
int DLinkedList<T>::indexOf(T item) {
    Node* current = head;  // Báº¯t Ä‘áº§u tá»« nÃºt Ä‘áº§u tiÃªn
    int index = 0;         // Khá»Ÿi táº¡o chá»‰ má»¥c

    // Duyá»‡t qua toÃ n bá»™ danh sÃ¡ch
    while (current != nullptr) {
        // So sÃ¡nh dá»¯ liá»‡u cá»§a nÃºt hiá»‡n táº¡i vá»›i item
        if (current->data == item) {
            return index;  // Náº¿u tÃ¬m tháº¥y, tráº£ vá» chá»‰ má»¥c
        }
        current = current->next;  // Tiáº¿p tá»¥c Ä‘áº¿n nÃºt tiáº¿p theo
        index++;  // TÄƒng chá»‰ má»¥c
    }

    return -1;  // Náº¿u khÃ´ng tÃ¬m tháº¥y, tráº£ vá» -1
}

template <class T>
bool DLinkedList<T>::removeItem(T item, void (*removeItemData)(T)) {
    Node* current = head;  // Báº¯t Ä‘áº§u tá»« nÃºt Ä‘áº§u tiÃªn

    // Duyá»‡t qua danh sÃ¡ch
    while (current != nullptr) {
        // So sÃ¡nh dá»¯ liá»‡u cá»§a nÃºt hiá»‡n táº¡i vá»›i item
        if (current->data == item) {
            // Gá»i hÃ m removeItemData Ä‘á»ƒ giáº£i phÃ³ng dá»¯ liá»‡u náº¿u cÃ³
            if (removeItemData != nullptr) {
                removeItemData(current->data);
            }

            // Cáº­p nháº­t liÃªn káº¿t Ä‘á»ƒ xÃ³a nÃºt
            if (current->prev != nullptr) {
                current->prev->next = current->next;  // Káº¿t ná»‘i nÃºt trÆ°á»›c vá»›i nÃºt tiáº¿p theo
            } else {
                head = current->next;  // Cáº­p nháº­t head náº¿u nÃºt Ä‘áº§u tiÃªn bá»‹ xÃ³a
            }

            if (current->next != nullptr) {
                current->next->prev = current->prev;  // Káº¿t ná»‘i nÃºt tiáº¿p theo vá»›i nÃºt trÆ°á»›c
            } else {
                tail = current->prev;  // Cáº­p nháº­t tail náº¿u nÃºt cuá»‘i cÃ¹ng bá»‹ xÃ³a
            }

            delete current;  // Giáº£i phÃ³ng bá»™ nhá»› cá»§a nÃºt Ä‘Ã£ xÃ³a
            count--;  // Giáº£m sá»‘ lÆ°á»£ng pháº§n tá»­ trong danh sÃ¡ch
            return true;  // Tráº£ vá» true vÃ¬ Ä‘Ã£ xÃ³a thÃ nh cÃ´ng
        }
        current = current->next;  // Chuyá»ƒn Ä‘áº¿n nÃºt tiáº¿p theo
    }

    return false;  // Tráº£ vá» false náº¿u khÃ´ng tÃ¬m tháº¥y pháº§n tá»­
}

template <class T>
bool DLinkedList<T>::contains(T item) {
    return containsHelper(item, head->next);
}
template <class T>
bool DLinkedList<T>::containsHelper(T item, Node* curr) {
    while (curr != tail) {
        if (equals(curr->data, item, itemEqual)) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

template <class T>
string DLinkedList<T>::toString(string (*item2str)(T&)) {
    if (head == nullptr) {
        // Danh sÃ¡ch rá»—ng
        return "[]";
    }

    // Báº¯t Ä‘áº§u vá»›i dáº¥u má»Ÿ ngoáº·c vuÃ´ng
    string result = "[";

    // Duyá»‡t qua danh sÃ¡ch vÃ  chuyá»ƒn Ä‘á»•i tá»«ng pháº§n tá»­ thÃ nh chuá»—i
    Node* current = head;
    while (current != nullptr) {
        // Sá»­ dá»¥ng hÃ m item2str náº¿u Ä‘Æ°á»£c cung cáº¥p
        if (item2str != nullptr) {
            result += item2str(current->data);
        } else {
            // Sá»­ dá»¥ng ostringstream Ä‘á»ƒ Ä‘á»‹nh dáº¡ng sá»‘ tháº­p phÃ¢n
            std::ostringstream oss;

            // Äáº·t sá»‘ chá»¯ sá»‘ tháº­p phÃ¢n cáº§n hiá»ƒn thá»‹
            oss << std::fixed << std::setprecision(1) << current->data;

            // ThÃªm káº¿t quáº£ vÃ o chuá»—i káº¿t quáº£
            result += oss.str();
        }

        // Náº¿u khÃ´ng pháº£i lÃ  pháº§n tá»­ cuá»‘i cÃ¹ng, thÃªm dáº¥u pháº©y vÃ  khoáº£ng tráº¯ng
        if (current->next != nullptr) {
            result += ", ";
        }

        // Chuyá»ƒn sang nÃºt tiáº¿p theo
        current = current->next;
    }

    // ThÃªm dáº¥u Ä‘Ã³ng ngoáº·c vuÃ´ng
    result += "]";

    return result;
}


template <class T>
void DLinkedList<T>::copyFrom(const DLinkedList<T> &list) {
    // XÃ³a danh sÃ¡ch hiá»‡n táº¡i
    clear();

    // Duyá»‡t qua tá»«ng nÃºt trong danh sÃ¡ch nguá»“n (list) vÃ  sao chÃ©p tá»«ng pháº§n tá»­
    Node* current = list.head;
    while (current != nullptr) {
        // ThÃªm pháº§n tá»­ cá»§a danh sÃ¡ch nguá»“n vÃ o danh sÃ¡ch hiá»‡n táº¡i
        add(current->data);
        
        // Tiáº¿p tá»¥c Ä‘áº¿n nÃºt tiáº¿p theo
        current = current->next;
    }
}


template <class T>
void DLinkedList<T>::removeInternalData()
{
    Node *current = head;
    while (current) {
        Node *nextNode = current->next;
        if (deleteUserData) {
            deleteUserData(current->data);
        }
        delete current;
        current = nextNode;
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
}

#endif /* DLINKEDLIST_H */