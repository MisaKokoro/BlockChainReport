#pragma once
#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include <deque>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
//#include "bloomfilter.h"
#define SHA256_SIZE 32
using cryByte = CryptoPP::byte;
using std::string;
using std::vector;
struct SHA256Hash
{
    cryByte hash_val[SHA256_SIZE];
    SHA256Hash(const string &str) //通过字符串初始化hash
    {
        CryptoPP::SHA256 hash;
        hash.Update((const cryByte *)str.data(), str.size());
        hash.Final(hash_val);
    }
    SHA256Hash(const SHA256Hash &var) //通过另一个hash初始化hash
    {
        memcpy(hash_val, var.hash_val, SHA256_SIZE);
    }
    SHA256Hash(string *data)
    {
        CryptoPP::SHA256 hash;
        hash.Update((const cryByte *)data->data(), data->size());
        hash.Final(hash_val);
    }

    SHA256Hash(const SHA256Hash &a, const SHA256Hash &b) //通过两个hash初始化一个hash，后面计算两个节点的hash时会i用到
    {
        CryptoPP::SHA256 hash;
        string str = string((char *)a.hash_val, SHA256_SIZE) +
                     string((char *)b.hash_val, SHA256_SIZE);
        hash.Update((const cryByte *)str.data(), str.size());
        hash.Final(hash_val);
        // SHA256Hash(std::string((char *)a.hash_val, SHA256_SIZE) +
        // std::string((char *)b.hash_val, SHA256_SIZE));
    }
    bool operator==(const SHA256Hash &right) //重载运算符，方便判断两个hash是否相等
    {
        // std::cout << "======\n";
        if (memcmp(hash_val, right.hash_val, SHA256_SIZE) == 0)
        {
            return true;
        }
        else
            return false;
    }
};
struct Node
{
    Node *left;
    Node *right;
    // string *data;
    SHA256Hash hash;
#ifdef BLOOMFILTER
    BloomFilter filter;
#endif

public:
    Node(string *str) : left(nullptr), right(nullptr), hash(str)
    {
#ifdef BLOOMFILTER
        filter.add(str);
#endif
    }
    Node(Node *l) : left(l), right(nullptr), hash(l->hash)
#ifdef BLOOMFILTER
                    ,
                    filter(l->filter)
#endif
    {
    }
    Node(Node *l, Node *r) : left(l), right(r), hash(l->hash, right->hash)

#ifdef BLOOMFILTER
                             ,
                             filter(l->filter, r->filter)
#endif
    {
    }
    bool isLeaf()
    {
        return left == nullptr;
    }
};
int a = sizeof(Node);
class MerkleTree
{
private:
    Node *root;
    size_t numNode;
    size_t byteUse;
    size_t level;
    // #ifdef BLOOMFILTER
    //     BloomFilter filter;
    // #endif

public:
    MerkleTree() : root(nullptr), numNode(0), byteUse(0), level(0) {}
    MerkleTree(vector<string *> vec)
    {
        byteUse = 0;
        std::deque<Node *> nodes; //创建一个双端队列用来方便一层一层构建默克尔树
        for (auto d : vec)
        {
            // #ifdef BLOOMFILTER
            //             filter.add(d);
            // #endif
            nodes.emplace_back(new Node(d));
        }
        byteUse += nodes.size() * sizeof(Node);
        numNode = nodes.size();
        level = 1;
        while (nodes.size() != 1)
        {
            buildUpperLevel(nodes);
            level++;
        }
        root = nodes.front();
    }
    void buildUpperLevel(std::deque<Node *> &nodes)
    {
        int is_odd = nodes.size() % 2;
        size_t even_size = is_odd ? (nodes.size() - 1) : nodes.size();
        for (size_t i = 0; i < even_size; i += 2) //一次把偶数个节点合并
        {
            Node *left = nodes.front();
            nodes.pop_front();
            Node *right = nodes.front();
            nodes.pop_front();
            nodes.push_back(new Node(left, right));
        }
        if (is_odd) //如果是奇数个节点，最后一个节点直接放入上层
        {
            nodes.push_back(new Node(nodes.front()));
            nodes.pop_front();
        }
        byteUse += (even_size / 2 + is_odd) * sizeof(Node); //计算新增的节点占用的内存
    }
    bool isExist_(Node *node, SHA256Hash &hash, string *str) //判断一个字符串是否在一个节点上
    {
        if (node == nullptr)
        {
            return false;
        }
#ifdef BLOOMFILTER
        else if (!node->filter.isExist(str))
        {
            return false;
        }
#endif
        else
        {
            if (node->isLeaf())
            {
                return hash == node->hash;
            }
            else
            {
                Node *left = node->left;
                Node *right = node->right;
                return isExist_(left, hash, str) || isExist_(right, hash, str);
            }
        }
        //         if (node == nullptr)
        //             return false;
        // #ifdef BLOOMFILTER
        //         if (!node->filter.MaybeExist(data))
        //             return false;
        // #endif
        //         if (node->IsLeaf())
        //             return (node->hash == target_hash) && (*data == *node->data);
        //         else
        //             return Exist_(node->left, target_hash, data) || Exist_(node->right, target_hash, data);
    }
    bool isExist(string *str) //判断一个字符串是否在默克尔树上m
    {
        // #ifdef BLOOMFILTER
        //         if (!filter.isExist(str))
        //             return false;
        //             // else
        //             //     return true;
        // #endif
        SHA256Hash tmp(str);
        return isExist_(root, tmp, str);
    }
    size_t levelSize() { return level; }
    size_t byteUseSize() { return byteUse; }
    size_t nodeSize() { return numNode; }
};