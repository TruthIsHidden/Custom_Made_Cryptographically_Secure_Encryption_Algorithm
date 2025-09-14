// ALL PRNG USE IS INTERNAL, THE SEED USED IS ALSO HAS EXCELLENT ENTROPY, NONE OF THE PRNG OUTPUTS ARE EXPOSED, A.K.A IT IS ONLY A THEORETICAL WEAKNESS
//SALTS ARE RANDOM, MARKERS ARE DETERMINISTIC
//SALT IS ADDED TOO KDF TO FOR MAXIMUM ENTROPY


#include "Hasher.h"
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
#include <limits>
#include <locale>


using namespace std;

class CryptoSystem {
private:
    Hasher h;
    string KEY;
    const string CHARSET = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    const string Extended = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
                         "€‚ƒ„…†‡ˆ‰Š‹ŒŽ''""•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿"
        "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
    const char CONTROL_CHARS[8] = {'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07'};

    const string encoder = "234567890~!@#$%^&*()_+qwertyuiop[]asdfghjkl;'zxcvbnmn<>?|";
    const string Combined = CHARSET + Extended;
    int MainBox[256] = { 1,133,84,252,17,126,159,177,34,59,138,108,24,185,131,174,249,243,171,112,5,
180,241,110,43,181,183,156,132,157,160,90,158,10,217,147,190,54,238,77,152,56,232,145,105,221,
149,37,125,109,55,226,250,220,96,204,22,165,162,196,85,28,134,124,137,142,8,229,52,11,16,114,
170,202,57,51,3,169,224,95,178,123,176,139,188,153,197,21,151,129,70,101,98,215,72,155,66,74,
113,107,150,44,206,89,40,205,172,29,198,20,75,128,223,245,19,167,47,41,182,228,64,26,27,62,179,
140,81,234,100,208,23,25,218,239,222,2,38,236,127,195,93,6,50,184,154,13,97,144,240,242,211,106
,87,39,175,82,148,166,187,32,191,213,122,67,130,244,49,35,46,143,78,76,116,168,141,247,94,103,163,
237,0,121,45,201,63,248,203,9,119,61,104,7,231,33,92,80,233,253,200,186,99,219,79,230,83,193,
146,214,235,14,60,73,209,31,255,18,117,12,227,210,194,4,164,42,192,225,71,69,118,212,135,88,30,
115,53,91,161,207,199,36,254,48,65,68,102,86,111,216,136,120,58,15,189,251,246,173 };
    string RandomSaltMerge = "";
 
public:

    string DeterministicLookUpTable(string original)
    {
        string useKEY = KEY;
        for (int j = 0;j < 2;j++)
        {
            for (int i = 0; i < useKEY.length(); i++)
                useKEY[i] = ((useKEY[i] << 3) | (useKEY[i] >> 5)) ^ (useKEY[(i + 1) % useKEY.size()] + i);
        }
        seed_seq use(useKEY.begin(), useKEY.end());
        mt19937 gen(use);

        int Table[256];
        for (int i = 0; i < 256; i++)
            Table[i] = i;
        for (int i = 255; i > 0; i--) {
            uniform_int_distribution<int> dist(0, i);
            int j = dist(gen);
            swap(Table[i], Table[j]);
        }

        vector<char> combine;
        for (int i = 0; i < original.length(); i++)
            combine.push_back(char(i + 3 % 256));
        for (int i = 0;i < combine.size();i++) combine[i] ^= useKEY[i % useKEY.length()];
        std::shuffle(combine.begin(), combine.end(), gen);

        for (int j = 0; j < 3;j++)
        {
            for (int i = 0; i < original.length(); i++)
            {
                original[i] ^= combine[i];
                original[i] = Table[static_cast<unsigned char>(original[i])];
            }
            original = h.Bytemix(original);
            original = h.DataShuffle(original);
        }
        return original;
    }

    string ReverseDeterministicLookUpTable(string encrypted)
    {
        string useKEY = KEY;
        for(int j = 0;j<2;j++)
        {
            for (int i = 0; i < useKEY.length(); i++)
                useKEY[i] = ((useKEY[i] << 3) | (useKEY[i] >> 5)) ^ (useKEY[(i + 1) % useKEY.size()] + i);
        }
        seed_seq use(useKEY.begin(), useKEY.end());
        mt19937 gen(use);

        int Table[256];
        for (int i = 0; i < 256; i++)
            Table[i] = i;

        for (int i = 255; i > 0; i--) {
            uniform_int_distribution<int> dist(0, i);
            int j = dist(gen);
            swap(Table[i], Table[j]);
        }

        int Inverse[256];
        for (int i = 0; i < 256; i++)
            Inverse[Table[i]] = i;

        // regenerate the same combine vector
        vector<char> combine;
        for (int i = 0; i < encrypted.length(); i++)
            combine.push_back(char(i + 3 % 256));
        for (int i = 0;i < combine.size();i++) combine[i] ^= useKEY[i % useKEY.length()];
        std::shuffle(combine.begin(), combine.end(), gen);

        for (int j = 2; j >= 0; j--)
        {
            encrypted = h.RDataShuffle(encrypted);
            encrypted = h.ReverseByteMix(encrypted);

            for (int i = 0; i < encrypted.length(); i++) {
                encrypted[i] = Inverse[static_cast<unsigned char>(encrypted[i])]; // undo Table
                encrypted[i] ^= combine[i];  // undo XOR
            }
        }

        return encrypted;
    }

    void GenerateRandomFusion()
    {
        random_device r;
        uniform_int_distribution<int> Range(32,126);
        for(int i = 0;i<12;i++)
        {
            int no = Range(r);
            while (no == 58 || no == 32) no = Range(r);
            RandomSaltMerge += char(no);
        }
    }
    string Mix256(string orginal)
    {
        int len = orginal.length();
        int bl;
        string PerXor;
        if (len < KEY.length())
        {
            if (len % 2 != 0) {
                orginal += ":|}";
                len = orginal.length();
            }

            bl = KEY.length() / len;

            for (int i = 0; i < len; i++)
            {
  
                PerXor = ""; 
                for (int j = 0; j < bl; j++)
                {
                    int keyIdx = i * bl + j; 
                    if (keyIdx < KEY.length()) {
                        PerXor += KEY[keyIdx];
                    }
                }

                for (int k = 0; k < PerXor.length(); k++)
                {
                    orginal[i] ^= PerXor[k];
                }
            }
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                orginal[i] ^= KEY[i % KEY.length()];
            }
        }

        return orginal;
    }
    string ReverseMix256(string mixed)
    {
        int len = mixed.length();
        int bl;
        string PerXor;
        if (len < KEY.length())
        {
            bl = KEY.length() / len;

            for (int i = 0; i < len; i++)
            {
                PerXor = ""; 
                for (int j = 0; j < bl; j++)
                {
                    int keyIdx = i * bl + j; 
                    if (keyIdx < KEY.length()) {
                        PerXor += KEY[keyIdx];
                    }
                }

                for (int k = 0; k < PerXor.length(); k++)
                {
                    mixed[i] ^= PerXor[k];
                }
            }

            if (mixed.length() >= 3 && mixed.substr(mixed.length() - 3) == ":|}") {
                mixed = mixed.substr(0, mixed.length() - 3);
            }
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                mixed[i] ^= KEY[i % KEY.length()];
            }
        }

        return mixed;
    }

    string KeyForMac(string Orginal)
    {
        for (char& c : Orginal) c = MainBox[int(c)];
        string final;
        string prekey = KEY;
        if (prekey.length() < Orginal.length()) prekey += h.HASHER(Orginal + prekey.substr(7, 11), Orginal.length() - prekey.length());
        while (Orginal.length() < prekey.length())
            Orginal += prekey[prekey.length() - Orginal.length()];
        string usekey = prekey;
        for (int i = 0; i < 2; i++)
        {
            for (char& c : usekey) c = MainBox[int(c)];
            int len = usekey.length();
            int x = 0;
            for (char& c : usekey) { c = (unsigned char)c ^ ((x * 17 + len) & 0xFF); x++; }
            if (Orginal.length() > 0) {
                int mid = Orginal.length() / 2;
                if (mid < prekey.length()) {
                    Orginal = Orginal.substr(0, mid) + prekey[((prekey.length() + Orginal.length() -4) / 2) % prekey.length()] +
                        (mid < Orginal.length() ? Orginal.substr(mid) : "");
                }
            }
            final = Orginal;
            final = h.DataShuffle(final);
            final = h.BytemixCorrupt(final);
        }
        return final;
    }
    string ImplementMac(string Orginal)
    {
        return h.HASHER(KeyForMac(Orginal), 12) + ":" + Orginal;
    }
    string VerifyMac(string Combined)
    {
        size_t npos = Combined.find(":");
        string Hash = Combined.substr(0, npos);
        Combined = Combined.substr(npos + 1);

        string keyForMacResult = KeyForMac(Combined);
        string computedMAC = h.HASHER(keyForMacResult, 12);

        if (Hash != computedMAC)
        {
            cout << "Tampered";
            RunInteractive();
        }
        return Combined;
    }
    string AddRandomSalt(string orginal)
    {
        string final;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(33, 126);
        string salt = "";
        for (int i = 0;i < 8;i++) salt += dist(gen);
        
        uint64_t seed = KEY[145];
        char org = (h.Nmgen(seed) % 256);
        seed ^= KEY[130];
        char sec = (h.Nmgen(seed) % 256);
        char marker = org ^ sec;
        string postsalt = "";
        for (int i = 0;i < 4;i++) postsalt += dist(gen);
        final = salt + orginal + marker +postsalt;
        return final;
    }

    string RemoveRandomSalt(const string& Salted)
    {
        string Data;
        Data = Salted.substr(8, Salted.length() - 13);
        return Data;
    }

    string ReverseStreamer(string streamerOutput) {
        string D_Key = KEY;
        streamerOutput = h.ReverseByteMix(streamerOutput);
        for (int i = D_Key.length() - 1;i >= 0;i--) D_Key[i] = ((((D_Key[i] + 43) + i) * 5) - 40) % 125;
        D_Key = h.HASHER(D_Key, streamerOutput.length());
        string originalData = h.RDimensionalMix(streamerOutput, D_Key);
        size_t npos = originalData.find("\x1F\x0B\x0E");
        originalData = originalData.substr(0, npos);
        return originalData;
    }
    
    void GenerateKey(string password) {
        uint64_t seed = 0;
        for(char &c: password)
        {
            c = MainBox[(unsigned char)c];
        }
        string seedSource = h.HASHER(password + Combined, password.length() / 2);
        
        int x = 0;
        for (char& c : seedSource) {
            if ((c - x) < 0) seed += int(c) * 1391 + ((x + c) * -(c - x));
            else seed += int(c) * 1391 + ((x + c) * (c - x));
            
            x++;
        }

        mt19937 gen(seed);
        uniform_int_distribution<> dist(0, Combined.length() - 1);
        while (password.length() <= 128) {
            char cha = h.Nmgen(seed);
            password += cha;
            seed ^= cha;
        }

        string hashedKey = h.HASHER(password, password.length());

        for (size_t i = 0; i < password.length(); i++) {
            password[i] ^= hashedKey[i];
        }
        string baseKey = password; 
        string Opkey(baseKey.length(), 0);
        string Touse = h.HASHER(RandomSaltMerge, password.length());
        for (int i = 0; i < baseKey.length(); i++) {
            Opkey[i] = baseKey[i] ^ Combined[i % Combined.length()];
            Opkey[i] = (Opkey[i] << (1 + (i % 3))) | (Opkey[i] >> (8 - (1 + (i % 3))) >> 1);
        }

        for (int i = 0; i < baseKey.length(); i++) {
            password[i] ^= (Opkey[i] << 2);
            password[i] ^= Touse[i];
        }
        KEY = h.HASHER(password, 300);
    }
    
    void ExtendKey(size_t targetLength) {
        if (KEY.length() < targetLength) {
            string extension = h.HASHER(KEY, targetLength - KEY.length());
            KEY += extension;
        }
    }
    string Encrypt(string& plaintext, const string& password) {
        GenerateRandomFusion();
        GenerateKey(password);
        h.GenSBox(KEY);
        plaintext = h.DataShuffle(plaintext);
        plaintext = AddRandomSalt(plaintext);
        plaintext = h.Bytemix(plaintext);
        plaintext = Mix256(plaintext);
        plaintext = h.Graph(plaintext, KEY);
        plaintext = DeterministicLookUpTable(plaintext);
        plaintext = h.DimensionalMix(plaintext, KEY);
        plaintext = h.Bytemix(plaintext);

        string D1;
        for (int i = 0; i < KEY.length(); i++) {
            D1 += KEY[i] ^ (KEY.length() - 1 - i);
        }

        string SPLITKEY(KEY.length(), '0'); 
        for (size_t i = 0; i < KEY.length(); i++) {
            uint8_t ch = (uint8_t)KEY[i];
            ch = (uint8_t)(ch * 0x9E);
            ch ^= (uint8_t)((i + 1) * 0xA7);
            ch ^= (uint8_t)(0x5C ^ (i * 0x2B));
            ch = (uint8_t)((ch << 3) | (ch >> 5));
            SPLITKEY[i] = ch;
        }

        for (int i = 0; i < min(SPLITKEY.length(), D1.length()); i++) {
            SPLITKEY[i] ^= D1[i];
        }

        plaintext = h.REVERSIBLEKDFRSARIPOFF(plaintext, SPLITKEY);
        string use = RandomSaltMerge;
        RandomSaltMerge = "";
        return h.Base64Encode(use + (plaintext));
    }

    string Decrypt(string& ciphertext, const string& password) {
        ciphertext = h.Base64Decode(ciphertext);
        RandomSaltMerge = ciphertext.substr(0, 12);
        ciphertext = ciphertext.substr(12);
        GenerateKey(password);
        h.GenSBox(KEY);

        string decodedCipher = ciphertext;

        string D1;
        for (int i = 0; i < KEY.length(); i++) {
            D1 += KEY[i] ^ (KEY.length() - 1 - i);
        }

        string SPLITKEY(KEY.length(), '0');  
        for (size_t i = 0; i < KEY.length(); i++) {
            uint8_t ch = (uint8_t)KEY[i];
            ch = (uint8_t)(ch * 0x9E);
            ch ^= (uint8_t)((i + 1) * 0xA7);
            ch ^= (uint8_t)(0x5C ^ (i * 0x2B));
            ch = (uint8_t)((ch << 3) | (ch >> 5));
            SPLITKEY[i] = ch;
        }

        for (int i = 0; i < min(SPLITKEY.length(), D1.length()); i++) {
            SPLITKEY[i] ^= D1[i];
        }

        decodedCipher = h.REVERSIBLEKDFRSARIPOFF(decodedCipher, SPLITKEY);
        decodedCipher = h.ReverseByteMix(decodedCipher);
        decodedCipher = h.RDimensionalMix(decodedCipher, KEY);
        decodedCipher = ReverseDeterministicLookUpTable(decodedCipher);
        decodedCipher = h.DecryptGraph(decodedCipher, KEY);

        decodedCipher = ReverseMix256(decodedCipher);
        decodedCipher = h.ReverseByteMix(decodedCipher);
        decodedCipher = RemoveRandomSalt(decodedCipher);
        decodedCipher = h.RDataShuffle(decodedCipher);
        RandomSaltMerge = "";
        return decodedCipher;
    }

   
    int GetIntInput() {
        int choice;
        while (true) {
            if (cin >> choice) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                return choice;
            }
            else {
                
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input!\n Please enter a number: ";
            }
        }
    }


    void ClearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    // Interactive Menu
    void ShowMenu() {
        cout << "\n=== ULTRA SECURE ENCRYPTION SYSTEM ===" << endl;
        cout << "1. Encrypt Message" << endl;
        cout << "2. Decrypt Message" << endl;
        cout << "3. Clear Screen" << endl;
        cout << "4. Exit" << endl;
        cout << "Choice: ";
    }

    void RunInteractive() {
        int choice;
        string message, password, result;

        while (true) {
            ShowMenu();
            choice = GetIntInput(); 
            switch (choice) {
            case 1: 
                cout << "\nEnter message to encrypt: ";
                getline(cin, message);
                cout << "Enter password: ";
                getline(cin, password);

                result = Encrypt(message, password);
                cout << "\nEncrypted: " << result << endl;
                cout << "Length: " << result.length() << " characters" << endl;
                break;

            case 2: 
                cout << "\nEnter encrypted message: ";
                getline(cin, message);
                cout << "Enter password: ";
                getline(cin, password);

                try {
                    result = Decrypt(message, password);
                    cout << "\nDecrypted: " << result << endl;
                }
                catch (const exception& e) {
                    cout << "\nError: Invalid ciphertext or wrong password!" << endl;
                }
                break;

            case 3:
                ClearScreen();
                cout << "🔥 WELCOME TO THE OVERKILL ENCRYPTION SYSTEM 🔥" << endl;
                continue;

            case 4: 
                ClearScreen();
                cout << "\n🔒 Goodbye! Your secrets are safe! 🔒" << endl;
                cout << "Remember: Your data is protected by computational overkill!" << endl;
                return;

            default:
                cout << "\nInvalid choice! Please enter 1-4." << endl;
            }

            cout << "\nPress Enter to continue...";
            cin.get();
        }
    }
};

int main() {
    std::setlocale(LC_ALL, "C");
    CryptoSystem crypto;


    crypto.ClearScreen();
    cout << "🔥 WELCOME TO THE OVERKILL ENCRYPTION SYSTEM 🔥" << endl;
    cout << "\nPress Enter to start...";
    cin.get();

    crypto.RunInteractive();

    return 0;
}
