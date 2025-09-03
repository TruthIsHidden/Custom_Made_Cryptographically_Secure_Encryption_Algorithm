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
    string stringToBinary(const string& input);
    string binaryToString(const string& binary);
    string RSProducer(string SEED);
    string KDFProduceEncryptStream(long long r, int len, string content);
    string XORPairs(const string& input);
    int modInverse(int a, int m);
public:
    // Public interface - only declarations
    string HASHER(string key, int lenny);
    string KDFRSARIPOFF(string content, string key);
    string Base64Encode(const string& input);
    string Base64Decode(const string& input);
    string REVERSIBLEKDFRSARIPOFF(string Orginal, string KEY);
    string MINIHASHER(string key, int lenny);
    string DimensionalMix(string Orginal, string KEY);
    string RDimensionalMix(string Mixed, string KEY);
    void GenerateInvSBox();    
    string ReverseByteMix(string Data);
    string Bytemix(string Data);
    string DataShuffle(string Original);
    string RDataShuffle(string final);
    string BytemixCorrupt(string Data);
    string Graph(string data, string KEY);
    string DecryptGraph(string decodedCipher, string KEY);
    void GenSBox(string prekey);
    // void RunCollisionTest(int l);  // Future implementation
};
