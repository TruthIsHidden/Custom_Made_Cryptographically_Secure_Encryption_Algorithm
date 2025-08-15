#pragma once
#include <iostream>
#include <string>
#include <bitset>
#include <algorithm>
#include <random>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <chrono>

using namespace std;

class Hasher
{
private:
    // Private helper methods - only declarations
    bool IsPrime(int num);
    string Base64Encode(const string& input);
    string Base64Decode(const string& input);
    string compressor(const string& input);
    string decompressor(const string& input);
    string toHex(const string& input);
    string fromHex(const string& hex);
    string stringToBinary(const string& input);
    string binaryToString(const string& binary);
    string RSProducer(string SEED);
    string XORPairs(const string& input);
    string KDFProduceEncryptStream(long long r, int len, string content);


public:
    // Public interface - only declarations
    string HASHER(string key, int lenny);
    string KDFRSARIPOFF(string content, string key);
    // void RunCollisionTest(int l);  // Future implementation
};