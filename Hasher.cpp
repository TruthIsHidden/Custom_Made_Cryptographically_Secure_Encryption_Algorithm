#include "Hasher.h"

bool Hasher::IsPrime(int num) {
    if (num < 2) return false;
    if (num == 2 || num == 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    return true;
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

string Hasher::compressor(const string& input) {
    if (input.empty()) return "";

    ostringstream out;
    int count = 1;
    for (size_t i = 1; i <= input.length(); ++i) {
        if (i < input.length() && input[i] == input[i - 1]) {
            count++;
        }
        else {
            out << input[i - 1] << count;
            count = 1;
        }
    }
    return out.str();
}

string Hasher::decompressor(const string& input) {
    ostringstream out;
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        string num;
        while (i + 1 < input.length() && isdigit(input[i + 1])) {
            num += input[++i];
        }
        if (!num.empty()) {
            int count = stoi(num);
            out << string(count, c);
        }
    }
    return out.str();
}

string Hasher::toHex(const string& input) {
    stringstream ss;
    ss << hex << setfill('0');
    for (unsigned char c : input) {
        ss << setw(2) << (int)c;
    }
    return ss.str();
}

string Hasher::fromHex(const string& hex) {
    string output;
    for (size_t i = 0; i < hex.length(); i += 2) {
        if (i + 1 < hex.length()) {
            string byte = hex.substr(i, 2);
            unsigned char chr = (unsigned char)strtol(byte.c_str(), nullptr, 16);
            output.push_back(chr);
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

    string SCRAPPY = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789!@#$%._+";

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
        SEED[i] ^= SEED[(i + 1) % SEED.length()];  // Better initial mixing
    }
    for (int iter = 1; iter <= 100; iter++) {
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

string Hasher::HASHER(string key, int lenny){
    
    for (size_t i = 0; i < key.length(); i++)
    {
        uint8_t ch = (uint8_t)key[i];
        ch = (uint8_t)(ch * 0x9E);
        ch ^= (uint8_t)((i + 1) * 0xA7);  // Ensure non-zero for first char
        ch ^= (uint8_t)(0x5C ^ (i * 0x2B)); // Additional position mixing
        ch = (uint8_t)((ch << 3) | (ch >> 5));
        key[i] = ch;
    }

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
    string SALT = "!@$%<>?@!$%(#8BRks,1-/*193#<{}|Mzsi31~#$>-:?>$,.";

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

    for (int i = 1; i <= 64; i++) {
        while (Predata.length() < key.length()) Predata += Predata;
        Predata = Predata.substr(0, key.length());

        while (SALT.length() < key.length()) SALT += SALT;
        SALT = SALT.substr(0, key.length());

        string currIntermediate;
        for (size_t y = 0; y < key.length(); y++) {
            currIntermediate.push_back((((unsigned char)key[y] ^ (unsigned char)Predata[y]) >> (i % 4)) ^ ((unsigned char)SALT[y] >> 1));
        }

        if (!prevIntermediate.empty()) {
            for (size_t j = 0; j < currIntermediate.length() && j < prevIntermediate.length(); j++) {
                currIntermediate[j] = (unsigned char)currIntermediate[j] ^ (unsigned char)prevIntermediate[j];
            }
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
            c = c << (i%8);
        }

        AddSalt = RSProducer(binary);
        for (char& c : AddSalt) {
            int shift = 1 + (binary[(i + 1) % binary.length()] - '0'); // 1 or 2
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
        PRESALT = PRESALT.substr(0, min(saltLen, (int)PRESALT.length()));

        for (size_t m = 0; m < PRESALT.length(); m++) {
            PRESALT[m] = ((((((((unsigned char)PRESALT[m] << 1) * i % 251) >> (m % 8)) + (i - (int)m) * 2 % max(1, (int)PRESALT.length())) >> (i % 2)) + 13) >> 1) % 256;
        }

        for (size_t m = 0; m < post.length(); m++) {
            post[m] = (unsigned char)post[m] ^ ((((((unsigned char)intermediate[m % intermediate.size()] << 1) | ((unsigned char)intermediate[m % intermediate.size()] >> 7) + (i * 3) << 1) % 256)));
        }

        if (post.length() % 8 == 0) {
            if (post.length() < 7) {
                post.append("!@#?:*/-+.?}}{|" + char((PRESALT[i % PRESALT.length()])) - 2);
            }
            else {
                int reduction = ndist(s);
                post = post.substr(0, post.length() - min(reduction, (int)post.length()));
            }
        }

        if (post.length() % 16 == 0) {
            post = KDFRSARIPOFF(post, AddSalt);
            int ind = 0;
            for(char &c: post)
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
                uc = (unsigned char)((uc >> 1) | (uc << 7)); // ROTR1
                c = (char)uc;
            }
        }
    }
    while (post.length() < lenny) {
        post += RSProducer(post + post[40 % (post.length() / 2)]);
    }

    return Base64Encode(post.substr(0, lenny));
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

    string stream = KDFProduceEncryptStream(r, (int)content.length(), content);

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
        int remaining = content.length() - SALTV2.length();
        string Additive = HASHER(SALTV2, remaining);
        SALTV2 += Additive;
    }
    SALTV2 = SALTV2.substr(0, content.length());

    for (size_t m = 0; m < content.length(); m++) {
        content[m] = (unsigned char)content[m] ^ (unsigned char)SALTV2[m];
        content[m] = (unsigned char)content[m] >> ((m + 1) % 2);
    }

    return content;

}
