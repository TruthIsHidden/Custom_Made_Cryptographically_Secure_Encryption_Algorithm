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
    const string Combined = CHARSET + Extended;
public:

    string AddRandomSalt(string orginal)
    {
        random_device rd;
        mt19937 gen(rd());
        int Seed = 0;
        int x = 0;
        string PREDATA = KEY + KEY.substr(49, 54);
        string USEKEY = "";
        for (int i = PREDATA.length() - 1;i >= 0;i--) { USEKEY += char((PREDATA[i] + i - x - 1) * 131) % 143;x++; }
        x = 0;
        for (char& c : USEKEY) {
            Seed += (int(c) * 133 + 2 * x) % 109;x++;
        }
        mt19937 fgen(Seed);
        uniform_int_distribution<> dist(0, 99);
        uniform_int_distribution<> zdist(0, 8);
        uniform_int_distribution<> fdist(0, static_cast<int>(CHARSET.length() - 1));
        uniform_int_distribution<> ndist(0, Extended.length() - 1);
        int SaltLen = 25 + zdist(gen);
        string GenSalt, GenSalt2;
        for (int i = 0;i < SaltLen;i++)
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
        string Marker1, Marker2;
        int len = 3 + zdist(fgen);
        for (int i = 0;i < len;i++)
        {
            int pos = ndist(fgen);
            Marker1 += Extended[pos];
        }
        for (int i = 0;i < len;i++)
        {
            int pos = ndist(fgen);
            Marker2 += Extended[pos];
        }
        string final = GenSalt + Marker1 + orginal + CHARSET[num2] + Marker2 + GenSalt2 + CHARSET[num3];
        USEKEY = h.MINIHASHER(USEKEY, final.length());
        string Balls = Bytemix(Combined);
        for(int i = 0;i<Balls.length();i++)
        {
            Balls[i] <<= 2;
        }
        while (Balls.length() < final.length()) Balls += Balls;
        for (int i = final.length() - 1;i >= 0;i--) {
            unsigned char val = final[i];
            val = (val + i + 131) & 0xFF;  // Position-dependent addition
            int rot = (i + 1) % 8;  // Rotation amount
            val = ((val << rot) | (val >> (8 - rot))) & 0xFF;  // Left rotate
            final[i] = val ^ Balls[i] ^ USEKEY[i];
        }
        return final;
    }

    string RemoveRandomSalt(const string& Salted)
    {
        int Seed = 0;
        int x = 0;
        string PREDATA = KEY + KEY.substr(49, 54);
        string USEKEY = "";
        for (int i = PREDATA.length() - 1;i >= 0;i--) { USEKEY += char((PREDATA[i] + i - x - 1) * 131) % 143;x++; }
        x = 0;
        for (char& c : USEKEY) {
            Seed += (int(c) * 133 + 2 * x) % 109;x++;
        }

        // First decrypt the entire string
        string decrypted = Salted;
        USEKEY = h.MINIHASHER(USEKEY, decrypted.length());
        string Balls = Bytemix(Combined);
        for (int i = 0;i < Balls.length();i++)
        {
            Balls[i] <<= 2;
        }
         while (Balls.length() < decrypted.length()) Balls += Balls;
        for (int i = decrypted.length() - 1;i >= 0;i--) {
            unsigned char val = decrypted[i];
            val = val ^ USEKEY[i] ^ Balls[i];  // Reverse XOR with key
            int rot = (i + 1) % 8;  // Same rotation amount
            val = ((val >> rot) | (val << (8 - rot))) & 0xFF;  // Right rotate (reverse of left)
            val = (val - i - 131) & 0xFF;  // Reverse addition - THIS WAS MISSING
            decrypted[i] = val;
        }

        // Now find markers in the decrypted data
        mt19937 fgen(Seed);
        uniform_int_distribution<> zdist(0, 8);
        uniform_int_distribution<> ndist(0, Extended.length() - 1);
        int len = 3 + zdist(fgen);
        string Marker1, Marker2;
        for (int i = 0; i < len; i++) Marker1 += Extended[ndist(fgen)];
        for (int i = 0; i < len; i++) Marker2 += Extended[ndist(fgen)];

        size_t pos = decrypted.find(Marker1);  // Search in decrypted, not Salted
        if (pos == string::npos) return "";
        string afterMarker = decrypted.substr(pos + Marker1.length());  // Use decrypted
        size_t sepPos = afterMarker.find(Marker2);
        if (sepPos == string::npos || sepPos == 0) return "";
        string Original = afterMarker.substr(0, sepPos - 1);
        return Original;
    }
    string Streamer(string Data)
    {
        string DerivedKey = KEY;
        if(DerivedKey.length() < Data.length())
        {
            DerivedKey += h.MINIHASHER(DerivedKey, Data.length() - DerivedKey.length());
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
            DerivedKey += h.MINIHASHER(DerivedKey, streamerOutput.length() - DerivedKey.length());
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
    void GenerateKey(string password) {
        int seed = 0;
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
            password += Combined[dist(gen)];
        }

        string hashedKey = h.HASHER(password, password.length());

        for (size_t i = 0; i < password.length(); i++) {
            password[i] ^= hashedKey[i];
        }
        string baseKey = password; 
        string Opkey(baseKey.length(), 0);

        for (int i = 0; i < baseKey.length(); i++) {
            Opkey[i] = baseKey[i] ^ Combined[i % Combined.length()];
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
        GenerateKey(password);
        plaintext = AddRandomSalt(plaintext);
        plaintext = Bytemix(plaintext);
        GenerateKey(password);
        ExtendKey(plaintext.length());

        string encrypted = plaintext;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] ^= KEY[i];
        }
        // encrypted = Matrix(encrypted);
        encrypted = h.REVERSIBLEKDFRSARIPOFF(encrypted, KEY);
        encrypted = h.Base64Encode(AddRandomSalt(Streamer(encrypted)));
        return encrypted;
    }

    string Decrypt(const string& ciphertext, const string& password) {
        GenerateKey(password);
        string decodedCipher = h.Base64Decode(ciphertext);
        decodedCipher = RemoveRandomSalt(decodedCipher);
        string afterStreamer = ReverseStreamer(decodedCipher);
        afterStreamer = h.REVERSIBLEKDFRSARIPOFF(afterStreamer, KEY);
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
