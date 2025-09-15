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

uint64_t rotate_left_64(uint64_t value, unsigned int positions) {
    positions %= 64;
    return (value << positions) | (value >> (64 - positions));
}

uint64_t rotate_right_64(uint64_t value, unsigned int positions) {
    positions %= 64;
    return (value >> positions) | (value << (64 - positions));
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
    string ogkey = key;
    int tempSBox[256];
    for (int i = 0; i < 256; i++) tempSBox[i] = i;

    uint64_t seed = 0;
    for (size_t i = 0; i < key.length(); i++) {
        seed = ((seed << 5) + seed) + (unsigned char)key[i];
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

    string SALT = Combined;

    for (int i = 0; i < SALT.length(); i++) {
        unsigned char salt_val = (unsigned char)SALT[i];

        int idx1 = (255 - i) % SALT.length();
        unsigned char temp_val = (unsigned char)SALT[idx1];

        int sbox_idx = temp_val % 256;
        int combined = (i + salt_val) % 256;
        int xor_result = combined ^ (tempSBox[sbox_idx] % 256);
        int final_calc = (xor_result + 13) % key.length();

        SALT[i] = tempSBox[final_calc];
    }

    string post = "";
    string Diffuser;
    vector<int> Salts;

    int seed_idx = (SALT.length() - 1) ^ 13;
    if (seed_idx >= SALT.length()) seed_idx = seed_idx % SALT.length();
    seed = (unsigned char)SALT[seed_idx];

    for (int i = 0; i < SALT.length(); i++) {
        Salts.push_back((unsigned char)SALT[i]);
        char c = Nmgen(seed);
        Diffuser += tempSBox[((unsigned char)c) % 256];

        if (i < Diffuser.length()) {
            Salts.push_back((unsigned char)Diffuser[i]);
        }
        else {
            Salts.push_back(0); 
        }
        seed ^= (unsigned char)c;
    }

    if (Salts.empty()) {
        Salts.push_back(42);
    }

    uint64_t initial = Nmgen((unsigned char)key[9]);
    string rs;
    for (int i = 0;i < 32;i++) rs += Nmgen(i ^ initial + GRC - 32 + i) % 256;
    for (int round = 0; round < 16; round++)
    {
        for (int i = 0;i < 32;i++) key += Nmgen(initial ^ i + GRC) % 256 ^ rs[i];
        for (int i = 0; i < key.length(); i++) {
            int salt_idx = (i + 3) % Salts.size();
            key[i] ^= tempSBox[((unsigned char)key[i]) ^ (Salts[salt_idx] % 256)];
        }

        for (int i = 0; i + 1 < key.length(); i++) {
            std::swap(key[i], key[i + 1]);
        }

        int safe_idx = 34 % Salts.size();
        initial = Salts[safe_idx];

        for (int i = 0; i < Salts.size(); i++) {
            int key_idx = (round + i + 13) % key.length();
            initial ^= Nmgen((unsigned char)key[key_idx]);
            Salts[i] ^= rotate_left(initial ^ i + 1 - round, ((i - round) + round) % 8);
            Salts[i] = abs(Salts[i]) % 256;
        }

        for (int i = 0; i < key.length(); i++) {
            int sbox_idx = ((round - i + Salts.size()) ^ 133) % 256;
            key[i] = ((unsigned char)key[i]) ^ ((0xC6A4A7935BD1E995ULL ^ tempSBox[sbox_idx]) % 256);
        }

        if (round % 2 != 0) initial = ~initial;
        if (round % 2 == 0) initial = rotate_right_64(initial, (round + Salts.size() + 13) % 64);
        if (round % 9 == 0) key = Bytemix(key);
        if (round % 6 == 0)
        {
            for (int i = 0;i + 3 < key.length();i += 4) key[i] = (key[i] ^ key[i]) * 12 * (i + 3 - round) + ((round + key.length() + i) | 1);
        }

        rs = Bytemix(rs);

        if (round % 4 == 0)
        {
            seed_seq mixx(key.begin(), key.end());
            mt19937 mgen(mixx);
            std::shuffle(std::begin(tempSBox), std::end(tempSBox), mgen);
            for (int k = 0;k < key.length();k++) key[k] = tempSBox[static_cast<uint8_t>(key[k])];
            for (int i = 0; i < rs.length(); i++) {
                uint8_t val = static_cast<uint8_t>(rs[i]);
                val ^= (tempSBox[val] ^ tempSBox[rs.length() - 1 - i] ^ (i + 13)) % 256;
                rs[i] = static_cast<char>(val);

                for (int j = 0; j < key.length(); j++)
                    key[j] ^= rotate_left(tempSBox[((ogkey[j % ogkey.length()] ^ SALT[j % SALT.length()]) >> ((j + 1 - round) % 3)) % 256] ^ rs[j % rs.length()], (j + round + 4) % 8);
            }


            if (round % 8 == 0)
            {
                for (int IR = 0;IR < key.length();IR++)
                {
                    for (int k = IR;k + 1 < key.length();k += 2) key[k] ^= key[k + 1];
                }
                if (key.length() % 8 != 0) {
                    size_t pad = 8 - (key.length() % 8);
                    key.append(pad, '0');
                }

                int nBlocks = key.length() / 8;

                std::vector<std::vector<int>> blocks(nBlocks, std::vector<int>(8));

                for (int bl = 0; bl < nBlocks; ++bl) {
                    for (int i = 0; i < 8; ++i) {
                        blocks[bl][i] = static_cast<unsigned char>(key[bl * 8 + i]);
                    }
                }
                string replacekey;
                for(int bl = 0;bl<nBlocks;bl++)
                {
                    for(int i = 0;i<8;i++)
                    {
                        blocks[bl][i] ^= blocks[(i+1) % (bl + 1)][(bl+1) % 8];
                        blocks[bl][i] ^= GRC ^ tempSBox[blocks[bl][(i + 1) % 8] % 256];
                        int targetBlock = (bl + nBlocks - i + (bl * i)) % nBlocks;
                        int targetIndex = (bl + i + nBlocks) % 8;
                        std::swap(blocks[bl][i], blocks[targetBlock][targetIndex]);
                        if (blocks[bl][i] % 2 != 0) blocks[bl][i] += ((nBlocks + i + 2) | 1);
                        if (i % 2 == 0) blocks[bl][i] = rotate_left(blocks[bl][i], (i + bl) % 8);
                        if (i % 2 != 0) blocks[bl][i] = rotate_right(blocks[bl][i], (i + bl) % 8);
                        if (i < 4) swap(blocks[bl][i], blocks[bl][7 - i]);
                        int constAffine = (bl + round - i + 4) | 1;
                        if (i < 4 && bl < nBlocks/2) swap(blocks[bl][i], blocks[nBlocks - 1 - bl][i]);
                        replacekey += (blocks[bl][i] * (GRC + ((7 - i) * i)) / 2 + constAffine) % 256;
                    }
                }
                for (int i = 0;i < key.length();i++) key[i] ^= replacekey[i % replacekey.length()];
                
            }
        }
    }
    post = key;

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

    if (post.length() > targetLength)
        post.resize(targetLength);
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
