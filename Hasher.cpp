#include "Hasher.h"
#include <algorithm>
#include <unordered_set>


const string CHARSET = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
const string Extended = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
"€‚ƒ„…†‡ˆ‰Š‹ŒŽ''""•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿"
"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
const string Combined = CHARSET + Extended;
const int BlockSize = 2;
int SBox[256] = { 1,133,84,252,17,126,159,177,34,59,138,108,24,185,131,174,249,243,171,112,5,
180,241,110,43,181,183,156,132,157,160,90,158,10,217,147,190,54,238,77,152,56,232,145,105,221,
149,37,125,109,55,226,250,220,96,204,22,165,162,196,85,28,134,124,137,142,8,229,52,11,16,114,
170,202,57,51,3,169,224,95,178,123,176,139,188,153,197,21,151,129,70,101,98,215,72,155,66,74,
113,107,150,44,206,89,40,205,172,29,198,20,75,128,223,245,19,167,47,41,182,228,64,26,27,62,179,
140,81,234,100,208,23,25,218,239,222,2,38,236,127,195,93,6,50,184,154,13,97,144,240,242,211,106
,87,39,175,82,148,166,187,32,191,213,122,67,130,244,49,35,46,143,78,76,116,168,141,247,94,103,163,
237,0,121,45,201,63,248,203,9,119,61,104,7,231,33,92,80,233,253,200,186,99,219,79,230,83,193,
146,214,235,14,60,73,209,31,255,18,117,12,227,210,194,4,164,42,192,225,71,69,118,212,135,88,30,
115,53,91,161,207,199,36,254,48,65,68,102,86,111,216,136,120,58,15,189,251,246,173 };

int foosBox[256] = { 1,133,84,252,17,126,159,177,34,59,138,108,24,185,131,174,249,243,171,112,5,
180,241,110,43,181,183,156,132,157,160,90,158,10,217,147,190,54,238,77,152,56,232,145,105,221,
149,37,125,109,55,226,250,220,96,204,22,165,162,196,85,28,134,124,137,142,8,229,52,11,16,114,
170,202,57,51,3,169,224,95,178,123,176,139,188,153,197,21,151,129,70,101,98,215,72,155,66,74,
113,107,150,44,206,89,40,205,172,29,198,20,75,128,223,245,19,167,47,41,182,228,64,26,27,62,179,
140,81,234,100,208,23,25,218,239,222,2,38,236,127,195,93,6,50,184,154,13,97,144,240,242,211,106
,87,39,175,82,148,166,187,32,191,213,122,67,130,244,49,35,46,143,78,76,116,168,141,247,94,103,163,
237,0,121,45,201,63,248,203,9,119,61,104,7,231,33,92,80,233,253,200,186,99,219,79,230,83,193,
146,214,235,14,60,73,209,31,255,18,117,12,227,210,194,4,164,42,192,225,71,69,118,212,135,88,30,
115,53,91,161,207,199,36,254,48,65,68,102,86,111,216,136,120,58,15,189,251,246,173 };;
int InvSbox[256];
uint64_t GRC = 0x9E3779B97F4A7C15ULL;

bool Hasher::IsPrime(int num) {
    if (num < 2) return false;
    if (num == 2 || num == 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    return true;
}

uint8_t rotate_right(uint8_t byte, unsigned int i) {
    i %= 8;
    return (byte >> i) | (byte << (8 - i));
}
uint8_t rotate_left(uint8_t byte, unsigned int i) {
    i %= 8; // just in case i > 8
    return (byte << i) | (byte >> (8 - i));
}

string Hasher::BytemixCorrupt(string Data)
{
    string ByteBlob;

    for (char& c : Data)
    {
        bitset<8> binary(c);
        ByteBlob += binary.to_string();
    }

    size_t seed = 0;
    for (char& c : Data) seed += (c * 31 + 17);
    mt19937 gen(seed);

    for (int round = 0; round < 5; round++)
    {
        uniform_int_distribution<> dist(0, ByteBlob.length() - 1);
        for (int i = 0; i < ByteBlob.length() / 2; i++)
        {
            int j = dist(gen);
            swap(ByteBlob[i], ByteBlob[j]);
        }

        for (int i = 0; i < ByteBlob.length(); i++)
        {
            if ((i + round * 7) % 2 == 0)
                ByteBlob[i] = (ByteBlob[i] == '0') ? '1' : '0';

            if (dist(gen) % 3 == 0)
                ByteBlob[i] = (ByteBlob[i] == '0') ? '1' : '0';
        }
    }

    string PackedResult;
    for (int i = 0; i + 7 < ByteBlob.length(); i += 8)
    {
        bitset<8> bits(ByteBlob.substr(i, 8));
        PackedResult += char(bits.to_ulong());
    }
    uniform_int_distribution<> byteDist(0, PackedResult.length() - 1);
    for (int i = 0; i < PackedResult.length() / 2; i++)
    {
        int j = byteDist(gen);
        swap(PackedResult[i], PackedResult[j]);
    }

    return PackedResult;
}

string Hasher::Base64Encode(const string& input) {
    static const string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string output;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (output.size() % 4) output.push_back('=');
    return output;
}

string Hasher::Base64Decode(const string& input) {
    static const string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

    string output;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return output;
}

string Hasher::stringToBinary(const string& input) {
    string binaryResult = "";
    for (char c : input) {
        binaryResult += bitset<8>((unsigned char)c).to_string();
    }
    return binaryResult;
}

string Hasher::binaryToString(const string& binary) {
    string text = "";
    for (size_t i = 0; i < binary.length(); i += 8) {
        if (i + 8 <= binary.length()) {
            bitset<8> bits(binary.substr(i, 8));
            text += char(bits.to_ulong());
        }
    }
    return text;
}

string Hasher::XORPairs(const string& input) {
    string result;
    for (size_t i = 0; i < input.length(); i += 2) {
        char a = input[i];
        char b = (i + 1 < input.length()) ? input[i + 1] : 0;
        char xorChar = a ^ b;
        result += xorChar;
    }
    return result;
}

string Hasher::RSProducer(string SEED) {
    if (SEED.empty()) SEED = "fallback";
    if (SEED.length() < 2) SEED += SEED;

    string SCRAPPY = Combined;

    for (size_t i = 0; i < SEED.length(); i++) {
        SEED[i] = (unsigned char)SEED[i] >> 2;
    }

    std::vector<uint32_t> seed_data;
    for (char c : SEED)
        seed_data.push_back(static_cast<uint32_t>((unsigned char)c));
    seed_seq S(seed_data.begin(), seed_data.end());
    mt19937 s(S);
    uniform_int_distribution<int> dist(100000, 1000000);

    int LPN = 0;
    do {
        LPN = dist(s);
    } while (!IsPrime(LPN));

    string strLPN = to_string(LPN);
    string final = strLPN;
    for (size_t i = 0; i < SEED.length(); i++) {
        SEED[i] ^= SEED[(i + 1) % SEED.length()];  
    }
    for (int iter = 1; iter <= 64; iter++) {
        for (size_t i = 0; i < SEED.length(); i++) {
            SEED[i] = (unsigned char)SEED[i] ^ ((((((unsigned char)SEED[i] >> (i % 6)) ^ (5 * i)) + 12 * i) * (i % SEED.length()) >> 1) % 256);
        }

        for (char& c : strLPN) {
            c = (unsigned char)c << 2;
        }

        if (strLPN.length() % 2 != 0)
            strLPN.push_back(SEED[0]);

        string ooga = XORPairs(strLPN);
        uniform_int_distribution<int> ndist(0, max(1, (int)SEED.length() - 1));
        uniform_int_distribution<int> sdist(0, (int)SCRAPPY.length() - 1);

        while (ooga.length() < SEED.length()) {
            ooga.push_back(SEED[ndist(s)]);
        }
        ooga = ooga.substr(0, SEED.length());

        string bug;
        for (size_t i = 0; i < SEED.length(); i++) {
            bug.push_back(SCRAPPY[(unsigned char)ooga[i % ooga.length()] % SCRAPPY.length()]);
        }

        for (size_t i = 0; i < SEED.length(); i++) {
            SEED[i] = (unsigned char)SEED[i] ^ (unsigned char)ooga[i];
            SEED[i] = (unsigned char)SEED[i] ^ (unsigned char)bug[i];
        }

        while (SEED.length() < final.length()) {
            if (bug.length() >= 4 && ooga.length() >= 2)
                SEED.push_back((unsigned char)bug[3] ^ (unsigned char)ooga[1]);
            else
                SEED.push_back(SEED[0]);
        }

        for (size_t i = 0; i < final.length(); i++) {
            final[i] = (unsigned char)final[i] ^ ((unsigned char)SEED[i] >> (i % 5));
        }
        
        while (SEED.length() <= final.length()) {
            SEED += SEED;
        }
        final.push_back(SEED[final.length() % SEED.length()]);
    }
    return final.substr(0, min((size_t)32, final.length()));
}

string Hasher::HASHER(string key, int lenny) {

    int tempSBox[256];
    for (int i = 0; i < 256; i++) tempSBox[i] = i;

    // Use key to shuffle it deterministically
    uint64_t seed = 0;
    for (size_t i = 0; i < key.length(); i++) {
        seed = ((seed << 5) + seed) + (unsigned char)key[i]; // Hash-like combination
    }
    mt19937 gen(seed);
    for (int i = 255; i > 0; i--) {
        uniform_int_distribution<int> dist(0, i);
        int j = dist(gen);
        swap(tempSBox[i], tempSBox[j]);
    }

    key += to_string(GRC ^ lenny + key.length());
    if (key.empty()) return "";

    for (size_t i = 0; i < key.length(); i++) {
        uint8_t ch = (uint8_t)key[i];
        ch = (uint8_t)(ch * 0x9E);
        ch ^= (uint8_t)((i + 1) * 0xA7);
        ch ^= (uint8_t)(0x5C ^ (i * 0x2B));
        ch = (uint8_t)((ch << 3) | (ch >> 5));
        key[i] = ch;
    }

    for (char& c : key) c = (unsigned char)c << 2;

    string Predata = RSProducer(key);
    string SALT = Combined;
    for (int i = 0;i < SALT.length();i++) SALT[i] = tempSBox[(i + SALT[i]) % 256];
    for (char& c : SALT) c = (unsigned char)((c << 1) | (c >> 7));

    string AddSalt = RSProducer(SALT) + to_string(SALT.length());
    string PRESALT1 = RSProducer(AddSalt + key);
    string PRESALT2 = RSProducer(key + AddSalt);
    string PRESALT3 = RSProducer(key.substr(key.length() / 2) + AddSalt + key.substr(0, key.length() / 2));

    size_t presaltLen = PRESALT1.length();
    if (PRESALT2.length() < presaltLen) presaltLen = PRESALT2.length();
    if (PRESALT3.length() < presaltLen) presaltLen = PRESALT3.length();
    string PRESALT;
    PRESALT.reserve(presaltLen);
    for (size_t i = 0; i < presaltLen; i++)
        PRESALT.push_back(PRESALT1[i] ^ PRESALT2[i] ^ PRESALT3[i]);

    string prevIntermediate;
    string intermediate;
    string post;

    for (int round = 1; round <= 16; round++) {
        while (Predata.length() < key.length()) Predata += Predata;
        if (Predata.length() > key.length()) Predata.resize(key.length());

        while (SALT.length() < key.length()) SALT += SALT;
        if (SALT.length() > key.length()) SALT.resize(key.length());

        intermediate.resize(key.length());
        for (size_t i = 0; i < key.length(); i++) {
            intermediate[i] = (((unsigned char)key[i] ^ (unsigned char)Predata[i]) >> (round % 4))
                ^ ((unsigned char)SALT[i] >> 1);
        }

        if (!prevIntermediate.empty()) {
            for (size_t i = 0; i < intermediate.size(); i++)
                intermediate[i] ^= prevIntermediate[i % prevIntermediate.size()];
        }
        for (int i = 0;i < intermediate.length();i++) intermediate[i] = tempSBox[255 - intermediate[i]];
        prevIntermediate = intermediate;
        string binary = stringToBinary(intermediate);
        string currBits;
        currBits.reserve(binary.length());
        for (size_t i = 0; i + 1 < binary.length(); i++) {
            if (binary[i] == '0' && binary[i + 1] == '0') currBits += "11";
            else if (binary[i] == '1' && binary[i + 1] == '1') currBits += "00";
            else if (binary[i] == '1' && binary[i + 1] == '0') currBits += "01";
            else currBits += "10";
        }

        if (post.empty()) {
            post = currBits;
        }
        else {
            string newPost;
            newPost.reserve(currBits.size());
            for (size_t i = 0; i < currBits.size(); i++) {
                char existing = (i < post.size()) ? post[i] : 0;
                newPost.push_back(((existing - '0') ^ (currBits[i] - '0')) + '0');
            }
            post = std::move(newPost);
        }
        post = Bytemix(post);
        string entropySeed;
        size_t esLen = 10;
        for (size_t j = 0; j < post.size() && j < esLen; j++) entropySeed += post[j];
        for (size_t j = 0; j < AddSalt.size() && j < esLen; j++) entropySeed += AddSalt[j];
        for (size_t j = 0; j < PRESALT.size() && j < esLen; j++) entropySeed += PRESALT[j];

        for (size_t i = 0; i < post.size(); i++)
            post[i] ^= intermediate[i % intermediate.size()] ^ PRESALT[i % PRESALT.size()];

        if (round % 9 == 0) {
            for (char& c : post) {
                unsigned char uc = (unsigned char)c;
                c = (char)((uc >> 1) | (uc << 7));
            }
        }
        if (round % 16 == 0) post += to_string(GRC - 6 * post.length() ^ ((round + 1) * 4));
    }

    // --- Final expansion to exact length ---
    size_t targetLength = static_cast<size_t>(lenny);
    size_t iteration = 0;
    const size_t maxIterations = 4;
    while (post.length() < targetLength && iteration < maxIterations) {
        size_t len = (post.length() < 32) ? post.length() : 32;
        if (len == 0) break;
        string segment = post.substr(0, len);
        post += RSProducer(segment);
        iteration++;
    }
    if (post.length() > targetLength) post.resize(targetLength);

    return Base64Encode(post);
}
string Hasher::KDFRSARIPOFF(string content, string key) {
    if (content.empty() || key.empty()) return content;

    string SEED = RSProducer(key);
    vector<uint32_t> entropy;
    for (char c : SEED)
        entropy.push_back(static_cast<uint32_t>((unsigned char)c));
    seed_seq seq(entropy.begin(), entropy.end());
    mt19937 gen(seq);
    uniform_int_distribution<> distr(100000, 1000000);
    uniform_int_distribution<> mnDist(10, 10000);

    long long k = 0;
    do {
        k = distr(gen);
    } while (!IsPrime((int)k));

    int m = mnDist(gen), n = mnDist(gen);
    while (m == n) n = mnDist(gen);

    long long r = (1LL * k * k * (m - n) * (m - n) - 1LL * k * (m + n + 2)) / 2;

    string stream = KDFProduceEncryptStream(r  * r * r * r + (k - m), (int)content.length(), content);

    return stream;
}

string Hasher::KDFProduceEncryptStream(long long r, int len, string content) {
    string rS = to_string(r);
    while (rS.length() < content.length()) {
        rS.append(RSProducer(rS));
    }
    rS = rS.substr(0, content.length());

    for (size_t i = 0; i < content.length(); i++) {
        content[i] = (unsigned char)content[i] ^ (unsigned char)rS[i];
        content[i] = (unsigned char)(
            ((unsigned char)content[i] << ((i + 1) % 3)) |
            ((unsigned char)content[i] >> (8 - ((i + 1) % 3)))
            );
    }

    string SALTV2 = RSProducer(content);
    if (SALTV2.length() < content.length()) {
        int ToAdd = content.length() - SALTV2.length();
        string Additive = HASHER(SALTV2, ToAdd);
        SALTV2 += Additive;
    }

    for (size_t m = 0; m < content.length(); m++) {
        content[m] = (unsigned char)content[m] ^ (unsigned char)SALTV2[m];
        content[m] = (unsigned char)content[m] >> ((m + 1) % 2);
    }

    return content;
}

string Hasher::REVERSIBLEKDFRSARIPOFF(string Orginal, string KEY)
{
    
    if (Orginal.empty() || KEY.empty()) return Orginal;

    string SEED = RSProducer(KEY);
    vector<uint32_t> entropy;
    for (char c : SEED)
        entropy.push_back(static_cast<uint32_t>((unsigned char)c));
    seed_seq seq(entropy.begin(), entropy.end());
    mt19937 gen(seq);
    uniform_int_distribution<> distr(100000, 1000000);
    uniform_int_distribution<> mnDist(10, 10000);

    long long k = 0;
    do {
        k = distr(gen);
    } while (!IsPrime((int)k));

    int m = mnDist(gen), n = mnDist(gen);
    while (m == n) n = mnDist(gen);

    long long r = (1LL * k * k * (m - n) * (m - n) - 1LL * k * (m + n + 2)) / 2;
    long long use = r * r * r * r + (k - m);
    string stream = to_string(use);
    stream += HASHER(stream, KEY.length() - stream.length());
    stream = stream.substr(0, KEY.length());
    string workingKey = KEY;
    int x = 0;
    for(char &c: workingKey)
    {
        c ^= stream[x];
        x++;
    }
    if (workingKey.length() < Orginal.length()) workingKey += HASHER(workingKey, Orginal.length() - workingKey.length());
    x = 0;
    for (char& c : Orginal)
    {
        c ^= workingKey[x];
        x++;
    }

    return Orginal;
}

/*string Hasher::MINIHASHER(string key, int lenny) {

    key = (to_string(GRC).substr(0, lenny)) + key;
    for (size_t i = 0; i < key.length(); i++)
    {
        uint8_t ch = (uint8_t)key[i];
        ch = (uint8_t)(ch * 0x9E);
        ch ^= (uint8_t)((i + 1) * 0xA7);  
        ch ^= (uint8_t)(0x5C ^ (i * 0x2B)); 
        ch = (uint8_t)((ch << 3) | (ch >> 5));
        key[i] = ch;
    }
    for (int i = 0;i < key.length();i++) key[i] ^= (0xAB + i * 0x17) ^ 0xAB;
    for (int i = 0;i < key.length();i++) key[i] = SBox[((256 - i - lenny) + 3) % 256];
    if (key.empty()) return "";

    for (char& c : key) {
        c = (unsigned char)c << 2;
    }

    string Predata = RSProducer(key);
    key = RSProducer(key);

    string intermediate;
    string prevIntermediate;
    string binary;
    string post;
    string SALT = Combined;

    for (char& c : SALT) {
        c = (unsigned char)((c << 1) | (c >> 7));
    }

    string AddSalt = ((RSProducer(SALT) + to_string(SALT.length())));
    string PRESALT1 = RSProducer(AddSalt + key);
    string PRESALT2 = RSProducer(key + AddSalt);
    string PRESALT3 = RSProducer(key.substr(key.length() / 2) + AddSalt + key.substr(0, key.length() / 2));
    string PRESALT = "";
    for (size_t i = 0; i < min({ PRESALT1.length(), PRESALT2.length(), PRESALT3.length() }); i++) {
        PRESALT += char(PRESALT1[i] ^ PRESALT2[i] ^ PRESALT3[i]);
    }

    for (int i = 1; i <= 16; i++) {

        while (Predata.length() < key.length()) Predata += Predata;
        Predata = Predata.substr(0, key.length());

        while (SALT.length() < key.length()) SALT += SALT;
        SALT = SALT.substr(0, key.length());

        string currIntermediate;
        for (size_t y = 0; y < key.length(); y++) {
            currIntermediate.push_back((((unsigned char)key[y] ^ rotate_left((unsigned char)Predata[y], (i + 4) % 8) ^ ((unsigned char)SALT[y] >> 1))));
        }

        if (!prevIntermediate.empty()) {
            for (size_t j = 0; j < currIntermediate.length() && j < prevIntermediate.length(); j++) {
                currIntermediate[j] = (unsigned char)currIntermediate[j] ^ (unsigned char)prevIntermediate[j];
            }
        }
       for(int j = 0;j<prevIntermediate.length();j++)
       {
           prevIntermediate[j] ^= key[(i + j) % key.length()];
       }
        prevIntermediate = currIntermediate;
        intermediate = currIntermediate;
        binary = stringToBinary(intermediate);

        string currentBits;
        for (size_t m = 0; m < binary.length() - 1; m++) {
            if (binary[m] == '0' && binary[m + 1] == '0')
                currentBits += "11";
            else if (binary[m] == '1' && binary[m + 1] == '1')
                currentBits += "00";
            else if (binary[m] == '1' && binary[m + 1] == '0')
                currentBits += "01";
            else if (binary[m] == '0' && binary[m + 1] == '1')
                currentBits += "10";
        }

        if (post.empty()) {
            post = currentBits;
        }
        else {
            for (size_t z = 0; z < currentBits.size(); z++) {
                if (z >= post.size())
                    post.push_back(currentBits[z]);
                else
                    post[z] = ((post[z] - '0') ^ (currentBits[z] - '0')) + '0';
            }
        }

        vector<uint32_t> entropySeed;
        entropySeed.push_back(static_cast<uint32_t>(post.length()));
        entropySeed.push_back(static_cast<uint32_t>(AddSalt.length()));
        entropySeed.push_back(static_cast<uint32_t>(PRESALT.length()));

        // Add round number for uniqueness
        entropySeed.push_back(static_cast<uint32_t>(i));

        // Add original key bytes to entropy
        entropySeed.push_back(static_cast<uint32_t>(key.length()));
        for (int idx = 0; idx < min(8, (int)key.length()); ++idx)
            entropySeed.push_back(static_cast<uint32_t>((unsigned char)key[idx]));

        for (int idx = 0; idx < 10 && idx < (int)post.length(); ++idx)
            entropySeed.push_back(static_cast<uint32_t>((unsigned char)post[idx]));
        for (int idx = 0; idx < 10 && idx < (int)AddSalt.length(); ++idx)
            entropySeed.push_back(static_cast<uint32_t>((unsigned char)AddSalt[idx]));
        for (int idx = 0; idx < 10 && idx < (int)PRESALT.length(); ++idx)
            entropySeed.push_back(static_cast<uint32_t>((unsigned char)PRESALT[idx]));

        seed_seq S(entropySeed.begin(), entropySeed.end());
        mt19937 s(S);
        uniform_int_distribution<int> dist(1, 7);
        uniform_int_distribution<int> ndist(1, 4);
        uniform_int_distribution<int> fdist(0, 2);

        post = PRESALT + post;
        post += AddSalt;
        SALT = RSProducer(intermediate);

        for (char& c : SALT) {
            c = c << (i % 8);
        }

        AddSalt = RSProducer(binary);
        for (char& c : AddSalt) {
            int shift = 1 + (binary[(i + 1) % binary.length()] - '0'); 
            unsigned char uc = static_cast<unsigned char>(c);
            uc = static_cast<unsigned char>(uc >> shift);
            c = static_cast<char>(uc);
        }

        PRESALT = RSProducer(AddSalt);
        for (char& c : PRESALT) {
            c = (unsigned char)c >> 1; 
        }

        int saltLen = dist(s);
        AddSalt = AddSalt.substr(0, min(saltLen, (int)AddSalt.length()));
        for (int i = 0;i < AddSalt.length();i++) AddSalt[i] = SBox[(((i + 13) - 2 * lenny) + 200) % 256];
        PRESALT = PRESALT.substr(0, min(saltLen, (int)PRESALT.length()));
        for (int j = 0;j < AddSalt.length();j++)
        {
            AddSalt[j] ^= key[(i + j) % key.length()];
        }

        for (size_t m = 0; m < PRESALT.length(); m++) {
            PRESALT[m] = ((((((((unsigned char)PRESALT[m] << 1) * i % 251) >> (m % 8)) + (i - (int)m) * 2 % max(1, (int)PRESALT.length())) >> (i % 2)) + 13) >> 1) % 256;
        }

        for (size_t m = 0; m < post.length(); m++) {
            post[m] = (unsigned char)post[m] ^ ((((((unsigned char)intermediate[m % intermediate.size()] << 1) | ((unsigned char)intermediate[m % intermediate.size()] >> 7) + (i * 3) << 1) % 256)));
        }

        if (post.length() % 8 == 0) {
            if (post.length() < 7) {
                post.append("!@3?f-+R}{|" + char((PRESALT[i % PRESALT.length()])) - 2);
            }
            else {
                int reduction = ndist(s);
                post = post.substr(0, post.length() - min(reduction, (int)post.length()));
            }
        }

        if (post.length() % 16 == 0) {
            post = KDFRSARIPOFF(post, AddSalt);
            int ind = 0;
            for (char& c : post)
            {
                c = c - (ndist(s) + 2);
            }
        }
        if (i % 8) {
            for (char& c : PRESALT) {
                c = (unsigned char)c << (1 + fdist(s));
            }
        }
        if (i % 9) {
            for (char& c : post) {
                unsigned char uc = (unsigned char)c;
                uc = (unsigned char)((uc >> 1) | (uc << 7)); 
                c = (char)uc;
            }
        }
        for (int j = 0;j < post.length();j++)
        {
            post[j] ^= key[(i + j) % key.length()];
        }
        
        if (i % 16 == 0) post = BytemixCorrupt(post);
        for (int i = 0;i < post.length();i++) post[i] = SBox[(((i + 11) - 2 * lenny) + 201) % 256];
    }
    while (post.length() < lenny) {
        post += RSProducer(post + post[40 % (post.length() / 2)]);
    }

    return Base64Encode(post.substr(0, lenny));
}*/
string Hasher::DimensionalMix(string Original, string KEY)
{
    int dataPerBlock = (BlockSize * BlockSize) / 2;
    int NoBlocks = (Original.length() + dataPerBlock - 1) / dataPerBlock;
    string final;

    int Seed = 0;
    for (char& c : KEY) Seed += ((c << 1) * 134) % 124 + 124;
    mt19937 gen(Seed);
    uniform_int_distribution<> dist(0, 255);
    string Matrixproduct;
    for (int i = 0;i < KEY.length();i++)
    {
        Matrixproduct += (SBox[dist(gen)]);
    }
    Matrixproduct = HASHER(Matrixproduct, Matrixproduct.length());

    for (int block = 0; block < NoBlocks; block++)
    {
        vector<vector<char>> MBlock(BlockSize, vector<char>(BlockSize, 0));
        int startIdx = block * dataPerBlock;

        for (int j = 0; j < BlockSize; j++) {
            int globalIdx = startIdx + j;
            if (globalIdx < Original.length()) {
                MBlock[0][j] = Original[globalIdx];
            }
            else {
                MBlock[0][j] = 0;
            }
        }

        for (int j = 0; j < BlockSize; j++) {
            int keyIdx = (startIdx + j) % KEY.length();
            MBlock[1][j] = KEY[keyIdx] ^ Matrixproduct[keyIdx];
        }

        int Sze = MBlock.size();

        for (int round = 0; round < 9; round++)
        {
            if (round % 2 == 0)
            {
                
                for (int k = 0;k + 1 < Sze;k++)
                    for (int i = 0;i < Sze;i++)
                        for (int j = 0;j < Sze;j++)
                        {
                            int Offset = (i * Sze + j + k + round) % 127;
                            int multiplier = ((Matrixproduct[k] + Matrixproduct[k + 1]) + Offset) % 255;
                            MBlock[i][j] *= (multiplier | 1);
                        }
            }
            for (int i = 0; i < Sze; i++)
                for (int j = 0; j < Sze; j++)
                    MBlock[i][j] = SBox[(unsigned char)MBlock[i][j]];

            for (int i = 0; i + 1 < Sze; i++)
                for (int j = 0; j + 1 < Sze; j++)
                    swap(MBlock[i][j], MBlock[i + 1][j + 1]);

            for (int i = 0; i < Sze; i++)
                for (int j = i + 1; j < Sze; j++)
                    swap(MBlock[i][j], MBlock[j][i]);

            for (int i = 0; i < Sze; i++)
                for (int j = 0; j < i; j++)
                    MBlock[i][j] ^= MBlock[j][i];

            int roundKeyOffset = (round * 7 + block * 3) % KEY.length(); 
            for (int i = 0; i < Sze; i++) {
                for (int j = 0; j < Sze; j++) {
                    int keyIdx = (roundKeyOffset + i * Sze + j) % KEY.length();
                    MBlock[i][j] ^= KEY[keyIdx];
                }
            }

            MBlock[0][0] ^= ((MBlock[1][1] ^ MBlock[0][1]) ^ MBlock[1][0]) + round + 1;
            MBlock[0][0] += round * 7 + BlockSize / 3 + block + MBlock[1][1] - MBlock[1][0];
            MBlock[0][0] ^= MBlock[1][0];
            MBlock[0][1] ^= MBlock[1][1];

        }

        for (int i = 0; i < Sze; i++)
            for (int j = 0; j < Sze; j++)
                final.push_back(MBlock[i][j]);
    }
    return final;
}

string Hasher::RDimensionalMix(string Mixed, string KEY) {
    int Seed = 0;
    for (char& c : KEY) Seed += ((c << 1) * 134) % 124 + 124;
    mt19937 gen(Seed);
    uniform_int_distribution<> dist(0, 255);
    string Matrixproduct;
    for (int i = 0; i < KEY.length(); i++) {
        Matrixproduct += (SBox[dist(gen)]);
    }
    Matrixproduct = HASHER(Matrixproduct, Matrixproduct.length());

    int blockDataSize = BlockSize * BlockSize;
    int NoBlocks = Mixed.length() / blockDataSize;
    string final;

    for (int block = 0; block < NoBlocks; block++) {
        vector<vector<char>> MBlock(BlockSize, vector<char>(BlockSize));
        int index = block * blockDataSize;

        for (int i = 0; i < BlockSize; i++) {
            for (int j = 0; j < BlockSize; j++) {
                MBlock[i][j] = Mixed[index++];
            }
        }

        int Sze = MBlock.size();

        for (int round = 8; round >= 0; round--) {
            MBlock[0][1] ^= MBlock[1][1];
            MBlock[0][0] ^= MBlock[1][0];
            MBlock[0][0] -= round * 7 + BlockSize / 3 + block + MBlock[1][1] - MBlock[1][0];
            MBlock[0][0] ^= ((MBlock[1][1] ^ MBlock[0][1]) ^ MBlock[1][0]) + round + 1;

            int roundKeyOffset = (round * 7 + block * 3) % KEY.length();
            for (int i = 0; i < Sze; i++) {
                for (int j = 0; j < Sze; j++) {
                    int keyIdx = (roundKeyOffset + i * Sze + j) % KEY.length();
                    MBlock[i][j] ^= KEY[keyIdx];
                }
            }

            for (int i = Sze - 1; i >= 0; i--)
                for (int j = i - 1; j >= 0; j--)
                    MBlock[i][j] ^= MBlock[j][i];

            for (int i = 0; i < Sze; i++)
                for (int j = i + 1; j < Sze; j++)
                    swap(MBlock[i][j], MBlock[j][i]);

            for (int i = 0; i + 1 < Sze; i++)
                for (int j = 0; j + 1 < Sze; j++)
                    swap(MBlock[i][j], MBlock[i + 1][j + 1]);

            for (int i = 0; i < Sze; i++)
                for (int j = 0; j < Sze; j++)
                    MBlock[i][j] = InvSbox[(unsigned char)MBlock[i][j]];

            if (round % 2 == 0) {
                for (int k = Sze - 2; k >= 0; k--) {
                    for (int i = Sze - 1; i >= 0; i--) {
                        for (int j = Sze - 1; j >= 0; j--) {
                            int Offset = (i * Sze + j + k + round) % 127;
                            int multiplier = ((Matrixproduct[k] + Matrixproduct[k + 1]) + Offset) % 255;
                            int actualMultiplier = multiplier | 1; 
                            int inverse = modInverse(actualMultiplier, 256);
                            if (inverse != -1) {
                                MBlock[i][j] = (MBlock[i][j] * inverse) & 0xFF;
                            }
                        }
                    }
                }
            }
        }

        for (int j = 0; j < BlockSize; j++) {
            final.push_back(MBlock[0][j]);
        }
    }
    return final;
}

int Hasher::modInverse(int a, int m) {
    for (int x = 1; x < m; x++) {
        if (((a % m) * (x % m)) % m == 1) {
            return x;
        }
    }
    return -1;  
}
void Hasher::GenerateInvSBox() {
    for (int i = 0; i < 256; i++) {
        InvSbox[i] = 0;
    }

    for (int i = 0; i < 256; i++) {
        InvSbox[(unsigned char)SBox[i]] = i;
    }
}

string Hasher::Bytemix(string Data)
{
    vector<char> bits(Data.size() * 8);

    // Fill bit vector directly
    for (size_t i = 0; i < Data.size(); ++i)
    {
        for (int b = 0; b < 8; ++b)
            bits[i * 8 + b] = ((Data[i] >> (7 - b)) & 1) ? 1 : 0;
    }

    // 4-bit swap
    for (size_t i = 0; i + 3 < bits.size(); i += 4)
    {
        swap(bits[i], bits[i + 3]);
        swap(bits[i + 1], bits[i + 2]);
    }

    // Reverse and flip bits in-place
    size_t n = bits.size();
    for (size_t i = 0; i < n / 2; ++i)
        swap(bits[i], bits[n - 1 - i]);
    for (size_t i = 0; i < n; ++i)
        bits[i] ^= 1;

    // Pack bits back into bytes
    for (size_t i = 0; i < Data.size(); ++i)
    {
        char val = 0;
        for (int b = 0; b < 8; ++b)
            val |= bits[i * 8 + b] << (7 - b);
        Data[i] = val;
    }

    return Data; // no extra copies created
}

string Hasher::ReverseByteMix(string Data)
{
    vector<char> bits(Data.size() * 8);

    // Expand bytes into bits
    for (size_t i = 0; i < Data.size(); ++i)
        for (int b = 0; b < 8; ++b)
            bits[i * 8 + b] = ((Data[i] >> (7 - b)) & 1) ? 1 : 0;

    // Flip bits and reverse
    for (size_t i = 0; i < bits.size(); ++i)
        bits[i] ^= 1;
    size_t n = bits.size();
    for (size_t i = 0; i < n / 2; ++i)
        swap(bits[i], bits[n - 1 - i]);

    // Reverse 4-bit swaps
    for (size_t i = 0; i + 3 < bits.size(); i += 4)
    {
        swap(bits[i], bits[i + 3]);
        swap(bits[i + 1], bits[i + 2]);
    }

    // Pack bits back into bytes
    for (size_t i = 0; i < Data.size(); ++i)
    {
        char val = 0;
        for (int b = 0; b < 8; ++b)
            val |= bits[i * 8 + b] << (7 - b);
        Data[i] = val;
    }
    return Data;
}

string Hasher::DataShuffle(string Original)
{
    if (Original.length() <= 6) {
        int needed = 6 - Original.length();
        if (needed > 2) {
            Original += "❖⚹" + HASHER(Original, needed - 2);
        }
        else {
            Original += "❖⚹";
        }
    }
    for (int k = 0;k < 10;k++)
    {
        for (int i = 0; i + 1 < (int)Original.length(); ++i) {
            uint8_t xi = (uint8_t)Original[i];
            uint8_t xk = (uint8_t)(xi + (uint8_t)k);

            uint8_t rot = (uint8_t)(((xk >> 3) | (xk << 5)) & 0xFF);

            uint8_t mask = (uint8_t)(((((uint64_t)(uint8_t)Original[i + 1]) + (uint64_t)k * (uint64_t)GRC) >> 4) & 0xFF);
            mask |= 1;

            Original[i] = (char)(rot ^ mask);
        }

        Original = Bytemix(Original);

        for (int i = (int)Original.length() - 1; i >= 1; --i) {
            uint8_t left = (uint8_t)Original[i - 1];
            uint8_t mask2 = (uint8_t)((((uint64_t)left * (((uint64_t)GRC / 2) >> 2)) & 0xFF) | 1);
            Original[i] = (char)(((uint8_t)Original[i]) ^ mask2);
        }
    }
    return Original;
}

string Hasher::RDataShuffle(string final)
{
    for (int k = 9; k >= 0; --k) {
        for (int i = 1; i < (int)final.length(); ++i) {
            uint8_t left = (uint8_t)final[i - 1];
            uint8_t mask2 = (uint8_t)((((uint64_t)left * (((uint64_t)GRC / 2) >> 2)) & 0xFF) | 1);
            final[i] = (char)(((uint8_t)final[i]) ^ mask2);
        }

        final = ReverseByteMix(final);


        for (int i = (int)final.length() - 2; i >= 0; --i) {
            uint8_t c = (uint8_t)final[i];

            uint8_t mask = (uint8_t)(((((uint64_t)(uint8_t)final[i + 1]) + (uint64_t)k * (uint64_t)GRC) >> 4) & 0xFF);
            mask |= 1;

            c ^= mask;

            c = (uint8_t)(((c << 3) | (c >> 5)) & 0xFF);
            c = (uint8_t)((c - (uint8_t)k) & 0xFF);

            final[i] = (char)c;
        }
        size_t npos = final.find("❖⚹");
        if (npos != std::string::npos) {
            final = final.substr(0, npos); 
        }
    }

    return final;
}

string Hasher::DecryptGraph(string decodedCipher, string KEY)
{
    vector<uint64_t> da;
    vector<uint64_t> ky;
    string key = KEY;

    if (decodedCipher.empty()) return "";

    for (int i = 0; i < decodedCipher.length(); i++) {
        da.push_back((unsigned char)decodedCipher[i]); 
    }
    for (int i = 0; i < decodedCipher.length(); i++) {
        ky.push_back((unsigned char)key[i % key.length()]);
    }

    uint64_t seedk = ky[0] ^ ky[ky.size() - 1];
    for (int i = 0; i < ky.size(); i++) {
        int pos = Nmgen(seedk) % ky.size();
        swap(ky[i], ky[pos]);
        seedk = Nmgen(seedk);
    }

    int bpos = ky.size() - 13;
    if (bpos < 0) bpos = 0; 
    for (int i = 0; i < da.size(); i++) {
        uint64_t mult = (ky[i % ky.size()] + ky[bpos % ky.size()]) | 1;
        uint64_t inv = 1;
        for (int x = 1; x < 256; x++) {
            if (((mult % 256) * x) % 256 == 1) {
                inv = x;
                break;
            }
        }
        da[i] = (da[i] * inv) % 256;
    }

    for (int i = 0; i + 1 < da.size(); i++) {
        uint64_t seed = ky[i % ky.size()] ^ ky[(i + 1) % ky.size()];
        int dx = Nmgen(seed) % 256;
        da[i] ^= dx;
    }

    vector<pair<int, int>> swaps;
    uint64_t seedd = ky[7 % ky.size()] ^ ky[(ky.size() - 4) % ky.size()];
    for (int i = 0; i < da.size(); i++) {
        int pos = Nmgen(seedd) % da.size();
        swaps.push_back({ i, pos });
        seedd = Nmgen(seedd);
    }
    for (int i = swaps.size() - 1; i >= 0; i--) {
        swap(da[swaps[i].first], da[swaps[i].second]);
    }

    for (int i = 0; i < da.size(); i++) {
        da[i] = 255 - InvSbox[da[i]];
    }

    string result = "";
    for (int i = 0; i < da.size(); i++) {
        result += (char)da[i];
    }

    if (result == ":-|") {
        return "";
    }
    return result;
}

string Hasher::Graph(string data, string key)
{
    vector<uint64_t> da;
    vector<uint64_t> ky;

    if (data.empty()) data = ":-|";

    for (int i = 0; i < data.length(); i++) {
        da.push_back((unsigned char)data[i]);
    }
    for (int i = 0; i < data.length(); i++) {
        ky.push_back((unsigned char)key[i % key.length()]);
    }

    for (int i = 0; i < da.size(); i++) {
        da[i] = SBox[255 - da[i]];
    }

    uint64_t seedk = ky[0] ^ ky[ky.size() - 1];
    for (int i = 0; i < ky.size(); i++) {
        int pos = Nmgen(seedk) % ky.size();
        swap(ky[i], ky[pos]);
        seedk = Nmgen(seedk);
    }

    uint64_t seedd = ky[7 % ky.size()] ^ ky[(ky.size() - 4) % ky.size()];
    for (int i = 0; i < da.size(); i++) {
        int pos = Nmgen(seedd) % da.size();
        swap(da[i], da[pos]);
        seedd = Nmgen(seedd);
    }

    for (int i = 0; i + 1 < da.size(); i++) {
        uint64_t seed = ky[i % ky.size()] ^ ky[(i + 1) % ky.size()];
        int dx = Nmgen(seed) % 256;
        da[i] ^= dx;
    }

    int bpos = ky.size() - 13;
    if (bpos < 0) bpos = 0;
    vector<uint64_t> final;
    for (int i = 0; i < da.size(); i++) {
        uint64_t mult = (ky[i % ky.size()] + ky[bpos % ky.size()]) | 1;
        final.push_back(((da[i] * mult)) % 256);
    }

    string ret;
    for (int i = 0; i < final.size(); i++) {
        ret += (char)final[i];
    }
    return ret;
}

void Hasher::GenSBox(string prekey) 
{
    string use = HASHER(prekey, prekey.length());
    use = use.substr(0, prekey.length());
    int gbox[256];
    for (int i = 0;i < 256;i++) gbox[i] = i;
    for (int j = 0; j < 4; j++) 
    {
        for (int i = 0; i + 1 < use.length(); i++) 
        {
            prekey[i] ^= (foosBox[rotate_right(use[i], (i + 1) % 6)] * (13 + i)) 
                          ^ rotate_left((use[i + 1]) * (GRC / 4) * (i * 13), ((i + 2) % 8));

            int digit = (prekey[i + 1] % 10) + 1;

            prekey[i] += prekey[(digit + 1 ^ GRC * 3) / (((digit * digit * 3) * GRC / 4) + 1) % prekey.length()];

            prekey[i] = prekey[i] ^ rotate_right(prekey[i + 1], ((j + 1 + i) % 8)) * ((GRC ^ 13 + (i + 2 - i)));

            prekey[i] %= 256;
        }
    }
    seed_seq seq(prekey.begin(), prekey.end());
    mt19937 gen(seq);
    uniform_int_distribution<int> dist(0, 255);
    vector<int> fbox;
    for (int i = 255;i >= 0;i--) fbox.push_back(i);
    shuffle(fbox.begin(), fbox.end(), gen);
    for (int i = 0;i < 256;i++) SBox[i] = fbox[i];
    GenerateInvSBox();
       
}

uint64_t rotate_left_64(uint64_t value, unsigned int positions) {
    positions %= 64;
    return (value << positions) | (value >> (64 - positions));
}

uint64_t rotate_right_64(uint64_t value, unsigned int positions) {
    positions %= 64;
    return (value >> positions) | (value << (64 - positions));
}


uint64_t Hasher:: Nmgen(uint64_t num)
{
    num *= 0xC6A4A7935BD1E995ULL;
    uint64_t saltVal = 0x9E3779B97F4A7C15ULL;

    for(int i = 0;i<10;i++)
    {
        num ^= GRC;
        num = rotate_left_64(num, 8);
        num ^= saltVal ^ (GRC ^ num);
        num = rotate_right_64(num, i + 1);
        num *= 0xDEADBEEFCAFEBABEULL;
        num = rotate_left_64(num, i + 2);
        num -= GRC;
        if (i % 3 == 0) num *= (i+2);
        else num *= (2 * i + i);
        saltVal ^= rotate_right_64((((GRC * i + 3) | 1) + 13), (i + 3));
    }
    return num;
}
int Hasher::Field(int no)
{
    return no;
}
