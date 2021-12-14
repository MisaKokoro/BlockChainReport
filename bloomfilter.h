#pragma once
#define BLOOMFILTER
#include <bitset>
#include <utility>
#include <string>
using std::string;
#define BLOOMSIZE 10000
class BloomFilter
{
private:
    std::bitset<BLOOMSIZE> bitMap;
    std::hash<string> hashFun;

public:
    BloomFilter() = default;
    BloomFilter(const BloomFilter &a, const BloomFilter &b) { bitMap = a.bitMap | b.bitMap; }
    void add(string *str)
    {
        size_t hash_val = hashFun(*str);
        bitMap[hash_val % BLOOMSIZE] = true;
    }
    bool isExist(string *str) //返回false一定不存在，返回true可能存在
    {
        size_t hash_val = hashFun(*str);
        return bitMap[hash_val % BLOOMSIZE];
    }
};