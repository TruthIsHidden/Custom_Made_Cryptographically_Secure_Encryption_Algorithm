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

using namespace std;

class CryptoSystem {
private:
    Hasher h;
    string KEY;
    const string CHARSET = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

public:

    string AddRandomSalt(string orginal)
    {
        random_device rd; 
        mt19937 gen(rd()); 
        uniform_int_distribution<> dist(0, 99);
        uniform_int_distribution<> zdist(0, 8);
        uniform_int_distribution<> fdist(0, static_cast<int>(CHARSET.length() - 1));
        int SaltLen = 25 + zdist(gen);
        string GenSalt, GenSalt2;
        for(int i = 0;i<SaltLen;i++)
        {
            GenSalt += CHARSET[fdist(gen)];
        }
        SaltLen += zdist(gen);
        for (int i = 0;i < SaltLen;i++)
        {
            GenSalt2 += CHARSET[fdist(gen)];
        }
        int num = dist(gen);
        int num2 = fdist(gen);
        int num3 = fdist(gen);
        return GenSalt +  "?2@m̴a̴r̴k̴e̶r̴{.!" + orginal + CHARSET[num2] + "-̵*̶/̸" + GenSalt2 + CHARSET[num3];

    }

    string RemoveRandomSalt(const string& Salted)
    {
        
        size_t pos = Salted.find("?2@m̴a̴r̴k̴e̶r̴{.!");
        if (pos == string::npos) return ""; 

        string afterMarker = Salted.substr(pos + 18);

        size_t sepPos = afterMarker.find("-̵*̶/̸");
        if (sepPos == string::npos || sepPos == 0) return ""; 

        string Original = afterMarker.substr(0, sepPos - 1); 

        return Original;
    }
    string Streamer(string Data)
    {
        string DerivedKey = KEY;
        while (DerivedKey.length() < Data.length())
        {
            DerivedKey += h.HASHER(DerivedKey, Data.length() - DerivedKey.length());
        }

        // XOR each byte
        for (size_t i = 0; i < Data.length(); i++)
        {
            Data[i] ^= DerivedKey[i];
        }

        return Data;
    }

    string ReverseStreamer(string streamerOutput)
    {
        string DerivedKey = KEY;
        while (DerivedKey.length() < streamerOutput.length())
        {
            DerivedKey += h.HASHER(DerivedKey, streamerOutput.length() - DerivedKey.length());
        }

        string originalData = "";
        for (size_t i = 0; i < streamerOutput.length(); i++)
        {
            originalData += char(streamerOutput[i] ^ DerivedKey[i]);
        }

        return originalData;
    }
    string Bytemix(string Data)
    {
        string ByteBlob;
        for(char &c: Data)
        {
            bitset<8> binary(c);
            ByteBlob += binary.to_string();
        }

        for (int i = 0; i + 3 < ByteBlob.length(); i += 4)  
        {
            swap(ByteBlob[i], ByteBlob[i + 3]);
            swap(ByteBlob[i + 1], ByteBlob[i + 2]);
        }
        for (int i = 0; i + 3 < ByteBlob.length(); i += 4)  
        {
            swap(ByteBlob[i], ByteBlob[i + 3]);
            swap(ByteBlob[i + 1], ByteBlob[i + 2]);
        }
        string RevBlob;
        for (int i = ByteBlob.length() - 1; i >= 0; i--) {
            if (ByteBlob[i] == '0') RevBlob += '1';
            else RevBlob += '0';
        }
        string PackedResult = "";
        for (int i = 0; i < RevBlob.length(); i += 8) {
            if (i + 7 < RevBlob.length()) {
                string byte = RevBlob.substr(i, 8);
                bitset<8> bits(byte);
                PackedResult += char(bits.to_ulong());
            }
        }
        return PackedResult;
    }

    string ReverseByteMix(string PackedData)
    {
        string RevBlob = "";
        for (char& c : PackedData)
        {
            bitset<8> binary(c);
            RevBlob += binary.to_string();
        }

        string ByteBlob;

        for (int i = RevBlob.length() - 1; i >= 0; i--) {
            if (RevBlob[i] == '0') ByteBlob += '1';
            else ByteBlob += '0';
        }

        for (int i = 0; i + 3 < ByteBlob.length(); i += 4) {
            swap(ByteBlob[i], ByteBlob[i + 3]);
            swap(ByteBlob[i + 1], ByteBlob[i + 2]);
        }
        for (int i = 0; i + 3 < ByteBlob.length(); i += 4) {
            swap(ByteBlob[i], ByteBlob[i + 3]);
            swap(ByteBlob[i + 1], ByteBlob[i + 2]);
        }

        string originalData = "";
        for (int i = 0; i < ByteBlob.length(); i += 8) {
            if (i + 7 < ByteBlob.length()) {
                string byte = ByteBlob.substr(i, 8);
                bitset<8> bits(byte);
                originalData += char(bits.to_ulong());
            }
        }

        return originalData;
    }

    string Base64Encode(const string& input) {
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

    // Base64 Decoding
    string Base64Decode(const string& input) {
        static const string base64_chars =
            "s̸͊͌̏̊́͂̌̓͛̆͑͘͘͜͝͝è̶͍̈́̿̿̂͝x̶͆̃̂̎͆ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/b̷̦͓̔́́͑͝t̴͉̙̜̝̃̊͗̅̐͜͝c̸̨̗͉̤̳͂̌̚͠h̶̻̥̗̊̔͝ṉ̵̱̲͕͇͑į̶̩͈̭̐́g̵̲̺̈g̷̛͔̔̑͘a̸̢̮̰̾̂̽͗d̸̘̊͐͋ȉ̵̬̱̮̲͙͍͑̍̐̉͐ç̵̥̱̙̑̋͗̇̌̚͜k̸̞͉̪̠̦̍";
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

    void GenerateKey(string password) {
        int seed = 0;
        string seedSource = h.HASHER(password + CHARSET, password.length() / 2);

        int x = 0;
        for (char& c : seedSource) {
            if ((c - x) < 0) seed += int(c) * 1391 + ((x + c) * (-c - x));
            else seed += int(c) * 1391 + ((x + c) * (c - x));
            
            x++;
        }

        mt19937 gen(seed);
        uniform_int_distribution<> dist(0, CHARSET.length() - 1);
        while (password.length() < 128) {
            password += CHARSET[dist(gen)];
        }

        string hashedKey = h.HASHER(password, password.length());

        for (size_t i = 0; i < password.length(); i++) {
            password[i] ^= hashedKey[i];
        }
        string baseKey = password; 
        string Opkey(baseKey.length(), 0);

        for (int i = 0; i < baseKey.length(); i++) {
            Opkey[i] = baseKey[i] ^ CHARSET[i % CHARSET.length()];
            Opkey[i] = (Opkey[i] << (1 + (i % 3))) | (Opkey[i] >> (8 - (1 + (i % 3))) >> 1);
        }

        for (int i = 0; i < baseKey.length(); i++) {
            password[i] ^= (Opkey[i] << 2);
        }

        KEY = password;
    }


    void ExtendKey(size_t targetLength) {
        if (KEY.length() < targetLength) {
            string extension = h.HASHER(KEY, targetLength - KEY.length());
            KEY += extension;
        }
    }
    string Encrypt(string& plaintext, const string& password) {
        plaintext = AddRandomSalt(plaintext);
        plaintext = Bytemix(plaintext);
        GenerateKey(password);
        ExtendKey(plaintext.length());

        string encrypted = plaintext;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] ^= KEY[i];
        }
        // encrypted = Matrix(encrypted);
        encrypted = Base64Encode(Streamer(encrypted));
        return encrypted;
    }

    string Decrypt(const string& ciphertext, const string& password) {
        string decodedCipher = Base64Decode(ciphertext);
        string afterStreamer = ReverseStreamer(decodedCipher);
        GenerateKey(password);
        ExtendKey(afterStreamer.length());

        // Step 5: XOR to get final plaintext
        string decrypted = afterStreamer;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] ^= KEY[i];
        }
        decrypted = ReverseByteMix(decrypted);
        return RemoveRandomSalt(decrypted);
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
                cout << "Protecting your data with a keyspace over 2^1024(yes) of brute force resistance!" << endl;
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
    CryptoSystem crypto;

    crypto.ClearScreen();
    cout << "🔥 WELCOME TO THE OVERKILL ENCRYPTION SYSTEM 🔥" << endl;
    cout << "Protecting your data with a keyspace of over 2^1024(yes) years of brute force resistance!" << endl;
    cout << "\nPress Enter to start...";
    cin.get();

    crypto.RunInteractive();

    return 0;
}
