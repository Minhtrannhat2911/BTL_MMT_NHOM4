/*
 * File:   XArrayList.h
 */

#ifndef XARRAYLIST_H
#define XARRAYLIST_H
#include "list/IList.h"
#include <memory.h>
#include <sstream>
#include <iostream>
#include <type_traits>
#include <stdexcept> 
using namespace std;

template <class T>
class XArrayList : public IList<T>
{
public:
    class Iterator; // forward declaration

protected:
    T *data;                                 // dynamic array to store the list's items
    int capacity;                            // size of the dynamic array
    int count;                               // number of items stored in the array
    bool (*itemEqual)(T &lhs, T &rhs);       // function pointer: test if two items (type: T&) are equal or not
    void (*deleteUserData)(XArrayList<T> *); // function pointer: be called to remove items (if they are pointer type)

public:
    XArrayList(
        void (*deleteUserData)(XArrayList<T> *) = 0,
        bool (*itemEqual)(T &, T &) = 0,
        int capacity = 10);
    XArrayList(const XArrayList<T> &list);
    XArrayList<T> &operator=(const XArrayList<T> &list);
    ~XArrayList();

    // Inherit from IList: BEGIN
    void add(T e);
    void add(int index, T e);
    T removeAt(int index);
    bool removeItem(T item, void (*removeItemData)(T) = 0);
    bool empty();
    int size();
    void clear();
    T &get(int index);
    int indexOf(T item);
    bool contains(T item);
    string toString(string (*item2str)(T &) = 0);
    // Inherit from IList: BEGIN

    void println(string (*item2str)(T &) = 0)
    {
        cout << toString(item2str) << endl;
    }
    void setDeleteUserDataPtr(void (*deleteUserData)(XArrayList<T> *) = 0)
    {
        this->deleteUserData = deleteUserData;
    }

    Iterator begin()
    {
        return Iterator(this, 0);
    }
    Iterator end()
    {
        return Iterator(this, count);
    }

    /** free:
     * if T is pointer type:
     *     pass THE address of method "free" to XArrayList<T>'s constructor:
     *     to:  remove the user's data (if needed)
     * Example:
     *  XArrayList<Point*> list(&XArrayList<Point*>::free);
     *  => Destructor will call free via function pointer "deleteUserData"
     */
    static void free(XArrayList<T> *list)
    {
        typename XArrayList<T>::Iterator it = list->begin();
        while (it != list->end())
        {
            delete *it;
            it++;
        }
    }

protected:
    void checkIndex(int index);     // check validity of index for accessing
    void ensureCapacity(int index); // auto-allocate if needed

    /** equals:
     * if T: primitive type:
     *      indexOf, contains: will use native operator ==
     *      to: compare two items of T type
     * if T: object type:
     *      indexOf, contains: will use native operator ==
     *      to: compare two items of T type
     *      Therefore, class of type T MUST override operator ==
     * if T: pointer type:
     *      indexOf, contains: will use function pointer "itemEqual"
     *      to: compare two items of T type
     *      Therefore:
     *      (1): must pass itemEqual to the constructor of XArrayList
     *      (2): must define a method for comparing
     *           the content pointed by two pointers of type T
     *          See: definition of "equals" of class Point for more detail
     */
    static bool equals(T &lhs, T &rhs, bool (*itemEqual)(T &, T &))
    {
        if (itemEqual == 0)
            return lhs == rhs;
        else
            return itemEqual(lhs, rhs);
    }

    void copyFrom(const XArrayList<T> &list);

    void removeInternalData();

    //////////////////////////////////////////////////////////////////////
    ////////////////////////  INNER CLASSES DEFNITION ////////////////////
    //////////////////////////////////////////////////////////////////////
public:
    // Iterator: BEGIN
    class Iterator
    {
    private:
        int cursor;
        XArrayList<T> *pList;

    public:
        Iterator(XArrayList<T> *pList = 0, int index = 0)
        {
            this->pList = pList;
            this->cursor = index;
        }
        Iterator &operator=(const Iterator &iterator)
        {
            cursor = iterator.cursor;
            pList = iterator.pList;
            return *this;
        }
        void remove(void (*removeItemData)(T) = 0)
        {
            T item = pList->removeAt(cursor);
            if (removeItemData != 0)
                removeItemData(item);
            cursor -= 1; // MUST keep index of previous, for ++ later
        }

        T &operator*()
        {
            return pList->data[cursor];
        }
        bool operator!=(const Iterator &iterator)
        {
            return cursor != iterator.cursor;
        }
        // Prefix ++ overload
        Iterator &operator++()
        {
            this->cursor++;
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
    // Iterator: END
};

//////////////////////////////////////////////////////////////////////
////////////////////////     METHOD DEFNITION      ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
XArrayList<T>::XArrayList(
    void (*deleteUserData)(XArrayList<T> *),
    bool (*itemEqual)(T &, T &),
    int capacity)
{
    this->deleteUserData = deleteUserData;
    this->itemEqual = itemEqual;
    this->capacity = capacity;
    this->count = 0;
    this->data = new T[capacity]; // Cấp phát mảng động với dung lượng được chỉ định
}



template <class T>
void XArrayList<T>::copyFrom(const XArrayList<T> &list)
{
    if (this != &list) // Kiểm tra xem có sao chép chính đối tượng không
    {
        // Xóa dữ liệu hiện có
        removeInternalData();
        
        // Sao chép thông tin từ list
        this->capacity = list.capacity;
        this->count = list.count;
        this->itemEqual = list.itemEqual;
        this->deleteUserData = list.deleteUserData;
        
        // Cấp phát lại mảng và sao chép dữ liệu
        this->data = new T[this->capacity];
        for (int i = 0; i < this->count; ++i)
        {
            this->data[i] = list.data[i];
        }
    }
}

template <class T>
void XArrayList<T>::removeInternalData()
{
    // Giả sử deleteUserData chỉ là một hàm xóa cho kiểu T
    if (deleteUserData) 
    {
        for (int i = 0; i < this->count; ++i)
        {
            deleteUserData(reinterpret_cast<XArrayList<T> *>(&this->data[i])); // Gọi hàm xóa nếu được cung cấp
        }
    }

    delete[] this->data; // Giải phóng mảng động
    this->data = nullptr; // Đặt con trỏ mảng về nullptr
    this->count = 0;      // Đặt số lượng phần tử về 0
    this->capacity = 0;   // Đặt dung lượng về 0
}

template <class T>
XArrayList<T>::XArrayList(const XArrayList<T> &list)
{
    this->data = nullptr; // Tránh lỗi bộ nhớ nếu có
    copyFrom(list); // Gọi hàm copyFrom để sao chép dữ liệu
}

template <class T>
XArrayList<T> &XArrayList<T>::operator=(const XArrayList<T> &list)
{
    if (this != &list) // Tránh tự gán
    {
        removeInternalData(); // Xóa dữ liệu hiện có
        copyFrom(list);       // Sao chép từ list
    }
    return *this;
}


template <class T>
XArrayList<T>::~XArrayList()
{
    // Gọi hàm xóa nội dung nếu có
    removeInternalData();
    
    // Giải phóng bộ nhớ cho mảng dữ liệu
    delete[] data; // Giải phóng bộ nhớ cho mảng dữ liệu
}

template <class T>
void XArrayList<T>::add(T e)
{
    // Kiểm tra xem có cần mở rộng mảng hay không
    if (count >= capacity) {
        // Gấp đôi dung lượng hiện tại
        capacity *= 2;
        // Tạo mảng mới với dung lượng đã mở rộng
        T* newData = new T[capacity];
        
        // Sao chép dữ liệu từ mảng cũ sang mảng mới
        for (int i = 0; i < count; ++i) {
            newData[i] = data[i];
        }
        
        // Giải phóng bộ nhớ cho mảng cũ
        delete[] data;
        
        // Cập nhật con trỏ data để trỏ tới mảng mới
        data = newData;
    }

    // Thêm phần tử mới vào mảng
    data[count] = e;
    ++count; // Tăng số lượng phần tử trong danh sách
}

template <class T>
void XArrayList<T>::add(int index, T e)
{
    // Kiểm tra xem index có hợp lệ không
    if (index < 0 || index > count) {
        throw std::out_of_range("Index is out of range");
    }

    // Kiểm tra xem có cần mở rộng mảng hay không
    if (count >= capacity) {
        // Gấp đôi dung lượng hiện tại
        capacity *= 2;
        // Tạo mảng mới với dung lượng đã mở rộng
        T* newData = new T[capacity];
        
        // Sao chép dữ liệu từ mảng cũ sang mảng mới
        for (int i = 0; i < count; ++i) {
            newData[i] = data[i];
        }
        
        // Giải phóng bộ nhớ cho mảng cũ
        delete[] data;
        
        // Cập nhật con trỏ data để trỏ tới mảng mới
        data = newData;
    }

    // Dịch chuyển các phần tử từ vị trí index trở đi sang bên phải
    for (int i = count; i > index; --i) {
        data[i] = data[i - 1];
    }

    // Thêm phần tử mới vào vị trí index
    data[index] = e;
    ++count; // Tăng số lượng phần tử trong danh sách
}

template <class T>
T XArrayList<T>::removeAt(int index)
{
    // Kiểm tra xem index có hợp lệ không
    if (index < 0 || index >= count) {
        throw std::out_of_range("Index is out of range");
    }

    T removedItem = data[index]; // Lưu trữ phần tử bị xóa

    // Dịch chuyển các phần tử từ vị trí index + 1 đến count sang trái một vị trí
    for (int i = index; i < count - 1; ++i) {
        data[i] = data[i + 1];
    }

    --count; // Giảm số lượng phần tử trong danh sách

    // Xóa phần tử cuối cùng (không cần thiết nhưng tốt để tránh dữ liệu thừa)
    data[count] = T(); // Đặt giá trị mặc định cho phần tử cuối cùng

    return removedItem; // Trả về phần tử đã xóa
}


template <class T>
bool XArrayList<T>::removeItem(T item, void (*removeItemData)(T))
{
    for (int i = 0; i < count; ++i) {
        // Kiểm tra xem phần tử hiện tại có bằng với phần tử cần xóa không
        if (data[i] == item) {
            // Nếu có hàm xóa dữ liệu người dùng, gọi hàm này
            if (removeItemData) {
                removeItemData(data[i]); // Giải phóng dữ liệu nếu cần
            }

            // Gọi hàm removeAt để xóa phần tử tại vị trí i
            removeAt(i);
            return true; // Trả về true nếu xóa thành công
        }
    }

    return false; // Trả về false nếu không tìm thấy phần tử
}

template <class T>
bool XArrayList<T>::empty()
{
    return count == 0; // Trả về true nếu danh sách rỗng, ngược lại trả về false
}

template <class T>
int XArrayList<T>::size()
{
    return count; // Trả về số lượng phần tử trong danh sách
}

template <class T>
void XArrayList<T>::clear()
{
    // Nếu có hàm xóa dữ liệu người dùng, gọi hàm đó cho mỗi phần tử
    if (deleteUserData) {
        for (int i = 0; i < count; ++i) {
            // Chuyển đổi kiểu dữ liệu cho phù hợp với deleteUserData
            deleteUserData(reinterpret_cast<XArrayList<T>*>(&data[i])); 
        }
    }

    // Đặt lại count về 0
    count = 0;

    // Nếu cần, có thể giải phóng bộ nhớ của mảng data nếu không còn sử dụng
    // delete[] data; // Xóa mảng nếu cần thiết
    // data = nullptr; // Đặt lại con trỏ nếu đã xóa
}

template <class T>
T &XArrayList<T>::get(int index)
{
    // Kiểm tra tính hợp lệ của chỉ số
    if (index < 0 || index >= count) {
        throw std::out_of_range("Index is out of range."); // Ném ngoại lệ nếu chỉ số không hợp lệ
    }
    return data[index]; // Trả về tham chiếu đến phần tử tại chỉ số index
}

template <class T>
int XArrayList<T>::indexOf(T item)
{
    for (int i = 0; i < count; ++i) {
        if (data[i] == item) {
            return i; // Trả về chỉ số nếu tìm thấy item
        }
    }
    return -1; // Trả về -1 nếu không tìm thấy item
}

template <class T>
bool XArrayList<T>::contains(T item)
{
    return indexOf(item) != -1; // Trả về true nếu tìm thấy item, false nếu không tìm thấy
}


template <class T>
string XArrayList<T>::toString(string (*item2str)(T &))
{
    stringstream ss; // Sử dụng stringstream để xây dựng chuỗi
    ss << "["; // Bắt đầu chuỗi với dấu mở ngoặc vuông

    for (int i = 0; i < size(); ++i) {
        if (item2str) {
            // Nếu có hàm chuyển đổi, sử dụng hàm đó
            ss << item2str(data[i]); // Giả sử data là mảng lưu trữ phần tử
        } else {
            // Nếu không có hàm chuyển đổi, sử dụng chuyển đổi mặc định
            ss << data[i];
        }

        // Nếu không phải là phần tử cuối cùng, thêm dấu phẩy
        if (i < size() - 1) {
            ss << ", ";
        }
    }

    ss << "]"; // Kết thúc chuỗi với dấu đóng ngoặc vuông
    return ss.str(); // Trả về chuỗi kết quả
}

//////////////////////////////////////////////////////////////////////
//////////////////////// (private) METHOD DEFNITION //////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
void XArrayList<T>::checkIndex(int index)
{
    /**
     * Validates whether the given index is within the valid range of the list.
     * Throws an std::out_of_range exception if the index is negative or exceeds the number of elements.
     * Ensures safe access to the list's elements by preventing invalid index operations.
     */

    // Kiểm tra xem chỉ số có âm hay không
    if (index < 0) {
        throw std::out_of_range("Index cannot be negative.");
    }
    
    // Kiểm tra xem chỉ số có vượt quá kích thước danh sách hay không
    if (index >= size()) {
        throw std::out_of_range("Index exceeds the number of elements in the list.");
    }
}

#include <stdexcept> // Để sử dụng std::out_of_range
#include <new>       // Để sử dụng std::bad_alloc

template <class T>
void XArrayList<T>::ensureCapacity(int index) {
    /**
     * Ensures that the list has enough capacity to accommodate the given index.
     * If the index is out of range, it throws an std::out_of_range exception. If the index exceeds the current capacity,
     * reallocates the internal array with increased capacity, copying the existing elements to the new array.
     * In case of memory allocation failure, catches std::bad_alloc.
     */

    // Kiểm tra xem chỉ số có hợp lệ không
    if (index < 0) {
        throw std::out_of_range("Index cannot be negative.");
    }

    // Kiểm tra xem chỉ số có lớn hơn hoặc bằng dung lượng hiện tại không
    if (index >= capacity) { // Giả sử 'capacity' là dung lượng hiện tại của mảng
        // Tăng dung lượng, có thể là gấp đôi dung lượng hiện tại
        int newCapacity = capacity * 2; // Tăng dung lượng lên gấp đôi
        if (newCapacity <= index) { // Đảm bảo dung lượng đủ lớn
            newCapacity = index + 1; // Đảm bảo dung lượng đủ cho chỉ số
        }

        try {
            T* newData = new T[newCapacity]; // Cấp phát mảng mới

            // Sao chép các phần tử từ mảng cũ sang mảng mới
            for (int i = 0; i < count; ++i) {
                newData[i] = data[i]; // Sao chép dữ liệu
            }

            delete[] data; // Giải phóng mảng cũ
            data = newData; // Gán mảng mới cho mảng nội bộ
            capacity = newCapacity; // Cập nhật dung lượng
        } catch (const std::bad_alloc& e) {
            // Xử lý lỗi cấp phát bộ nhớ
            throw std::runtime_error("Memory allocation failed: " + std::string(e.what()));
        }
    }
}



#endif /* XARRAYLIST_H */
