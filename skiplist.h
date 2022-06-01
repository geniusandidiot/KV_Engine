#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "./store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";

template <typename K, typename V>
class Node
{

public:
    Node() {}

    Node(K key, V value, int);

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);

    //用于保存指向不同level的数组
    Node<K, V> **forward;

    int nodeLevel;

private:
    K key;
    V value;
};

template <typename K, typename V>
Node<K, V>::Node(const K key, const V value, int level)
{
    this->key = key;
    this->value = value;
    this->nodeLevel = level;

    // level + 1，level从0开始
    this->forward = new Node<K, V> *[level + 1];

    // 将其初始化为0
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
};

template <typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
};

template <typename K, typename V>
K Node<K, V>::get_key() const
{
    return key;
};

template <typename K, typename V>
V Node<K, V>::get_value() const
{
    return value;
};

template <typename K, typename V>
void Node<K, V>::set_value(V value)
{
    this->value=value;
};

template <typename K, typename V>
class SkipList
{

public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V> *create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    int change_element(K,V);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string &str, std::string *key, std::string *value);
    bool is_valid_string(const std::string &str);

private:
    //跳表最大level
    int _max_level;

    //当前跳表level
    int _skip_list_level;

    //指向头结点指针
    Node<K, V> *_header;

    //文件操作符
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    //跳表当前元素个数
    int _element_count;
};

//创建新结点
template <typename K, typename V>
Node<K, V> *SkipList<K, V>::create_node(const K k, const V v, int level)
{
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

//跳表大致就是一个二维结构，每一行是一条单向有序链表，level越高结点越少
template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    //互斥操作跳表
    mtx.lock();
    Node<K, V> *current = this->_header;

    //update数组用于存放后续需要操作的node->forward[i]的node
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    //从跳表最高层开始进行搜索
    for (int i = _skip_list_level; i >= 0; --i)
    {
        //当当前结点不为0且下一个结点还小于需要插入的key，继续向后遍历
        while (current->forward[i] && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        //找到小于insert_key的最大结点，更新update数组
        update[i] = current;
    }

    //此时current处于第一层，而它的下一个位置则为要插入的位置
    current = current->forward[0];
    //检查下一个结点是否存在且是否与insert_key相同
    if (current && current->get_key() == key)
    {
        std::cout << "Key: " << key << " has existed." << std::endl;
        mtx.unlock();
        return 1;
    }

    int random_level = get_random_level();
    // 若深度大于当前列表的深度，则更新深度，并更新 update 数组指向 head
    if (random_level > _skip_list_level)
    {
        for (int i = _skip_list_level + 1; i <= random_level; ++i)
        {
            update[i] = _header;
        }
        _skip_list_level = random_level;
    }

    Node<K, V> *inserted_node = create_node(key, value, random_level);
    for (int i = random_level; i >= 0; --i)
    {
        inserted_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = inserted_node;
    }

    std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
    _element_count++;

    mtx.unlock();
    return 0;
}

//打印跳表
template <typename K, typename V>
void SkipList<K, V>::display_list()
{

    std::cout << "\n*****Skip List*****"
              << "\n";
    for (int i = 0; i <= _skip_list_level; i++)
    {
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL)
        {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

//将内存数据保存至file中
template <typename K, typename V>
void SkipList<K, V>::dump_file()
{

    std::cout << "\n*****Dump file*****" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *current = _header->forward[0];
    while (current)
    {
        _file_writer << current->get_key() << delimiter << current->get_value() << std::endl;
        std::cout << current->get_key() << ":" << current->get_value() << std::endl;
        current = current->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return;
}

//从磁盘中加载数据
template <typename K, typename V>
void SkipList<K, V>::load_file()
{

    _file_reader.open(STORE_FILE);
    std::cout << "\n*****Load file*****" << std::endl;
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();

    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty())
        {
            std::cout << "error data ——  key:" << key << "value:" << value << std::endl;
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key <<'\t'<< "value:" << *value << std::endl;
    }
    _file_reader.close();
}

template <typename K, typename V>
int SkipList<K, V>::size()
{
    return _element_count;
}

template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string *key, std::string *value)
{
    if (!is_valid_string(str))
    {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str)
{

    if (str.empty())
    {
        return false;
    }
    if (str.find(delimiter) == std::string::npos)
    {
        return false;
    }
    return true;
}

//从跳表中删除元素
template <typename K, typename V>
void SkipList<K, V>::delete_element(K key)
{

    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; --i)
    {
        while (current->forward[i] && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (!current || current->get_key() != key)
    {
        std::cout << "Key: " << key << " do not existed." << std::endl;
        mtx.unlock();
        return;
    }

    for (int i = current->nodeLevel; i >= 0; --i)
    {
        update[i]->forward[i] = current->forward[i];
    }

    delete current;

    std::cout << "Successfully deleted key:" << key << "." << std::endl;
    --_element_count;

    mtx.unlock();
    return;
}

template <typename K, typename V>
int SkipList<K, V>::change_element(K key, V value)
{
    mtx.lock();

    Node<K, V> *current = this->_header;

    for (int i = _skip_list_level; i >= 0; --i)
    {
        while (current->forward[i] && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
    }
    // current 即为小于 key 的最大节点，其下一个节点为大于等于 key 的第一个结点

    // 接下来对 key 的存在做判定
    current = current->forward[0];
    if (!current || current->get_key() != key)
    {
        std::cout << "Key: " << key << " do not existed." << std::endl;
        mtx.unlock();
        return 1;
    }

    current->set_value(value);
    std::cout << "Successfully changed key:" << key << ", value:" << value << std::endl;

    mtx.unlock();
    return 0;
}

template <typename K, typename V>
bool SkipList<K, V>::search_element(K key)
{

    std::cout << "\n*****search_element*****" << std::endl;
    Node<K, V> *current = _header;

    for (int i = _skip_list_level; i >= 0; --i)
    {
        while (current->forward[i] && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if (current && current->get_key() == key)
    {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level) : _max_level(max_level), _skip_list_level(0), _element_count(0)
{
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

template <typename K, typename V>
SkipList<K, V>::~SkipList()
{

    if (_file_writer.is_open())
    {
        _file_writer.close();
    }
    if (_file_reader.is_open())
    {
        _file_reader.close();
    }
    
    delete _header;
}

template <typename K, typename V>
int SkipList<K, V>::get_random_level()
{

    int k = 0;
    while (rand() % 2)
    {
        ++k;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};