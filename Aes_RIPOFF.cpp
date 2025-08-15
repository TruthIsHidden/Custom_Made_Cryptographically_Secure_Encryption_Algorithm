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
string salts[] = {
            "xK9mP#vR8@qL2$nF7!jH4%wY",
            "Z3tB&gE5*uC1+oI6^sA9-dV0",
            "M7fQ!hN2@bW8#lX4$rT6%pJ1",
            "G5yU*cV9+eS3^aO7-iK0&mH2",
            "D4wL@nR6#fB8!kM1$tE5%qA9",
            "P2sJ*gH7+vC4^xZ0-uY3&oI6",
            "T8lN!mQ5@dF1#rW9$eK2%bS7",
            "A6jV*hG3+cX0^pL4-wM8&nR5",
            "E9fT@qB2#sY7!oH6$mU1%gK4",
            "R3zD*lW8+eC5^vN0-jA9&iP2",
            "F7bQ!nM4@xS1#kL6$tH9%rY3",
            "L0gE*cV5+oJ8^fW2-uB7&mP4",
            "H8dK!qR6@sT3#lN9$eA1%vC5",
            "U2pL*mF7+gS4^jH0-xY6&oB9",
            "Y5wN@bK8!cT1#rM3$vE6%qJ2",
            "C7tR*dH4+lW9^sF0-eU5&mA8",
            "B6qJ!gL3@nP7#fS2$oK9%xV1",
            "W4mE*hR8+cB5^tY0-jN6&lF3",
            "I9vD@pQ2#sG7!uL4$kM1%eT8",
            "O3jW*fH6+lC9^rN0-bS4&vE7",
            "Q8kL!mT5@dR2#eY6$fJ3%gP9",
            "S1nH*cU7+vB4^lE0-xM8&oK6",
            "N6pF@jR3#sL9!gW2$mT5%hV1",
            "V0tK*bE8+cQ4^rH7-lS3&uY6",
            "J4wM!nL1@dF5#pB9$vC2%eT8",
            "X7sR*gH3+jK6^mU0-fE4&oQ1",
            "K2cT@lW8#nB5!eR6$pJ9%vH3",
            "Q9mE*fD4+sL1^gK7-uY0&cW5",
            "Z5nR!bH8@tC3#lF6$oM2%pJ9",
            "F8wL*dK1+eT4^sR7-mQ0&gH5",
            "H3vJ@cF6#pL9!nB2$tE8%oY1",
            "T6kM*lR4+gS0^eH7-uC3&fW9",
            "R1jQ!mE8@dL5#sB2$nF6%vK4",
            "L9pH*cT3+eW7^gR0-oJ4&lM6",
            "Y4sE@fK1#nL8!mC5$rH2%pB9",
            "D7nW*jR6+lE3^sK0-cT4&vF8",
            "G2mQ!eH5@dC8#fL1$oR6%nJ9",
            "P5vK*lT2+sE7^mH4-gW0&cB3",
            "E8jF@nR4#lC1!pS6$tK9%eH2",
            "A0cM*dL7+fE3^sJ6-oR1&vW5",
            "U9nH@gK2#mE5!cR8$lF1%pT4",
            "I6fW*sL4+eC7^nH0-jR3&mQ9",
            "O3kE!dT6@lB1#pF4$sH8%vM2",
            "M7jR*cL0+eK5^gS3-nW6&fH9",
            "C4pT@mE8#sF1!oR5$lK2%nJ7",
            "B1vH*gL6+dC9^fE3-mS0&pR4",
            "W8nK!eT2@cF5#lR1$jH6%sM9",
            "V5mL*pE7+gK4^sC0-oF3&nR8",
            "J2fH@dR6#eL9!mT1$cK5%pS4",
            "X0sE*lW3+nF8^gR5-jH2&mC7",
            "K9pT!cL4@eH1#sR6$fM3%nJ8",
            "Q6vF*mE2+gK7^lC0-pR5&sH4",
            "Z3jL@nR8#eT1!cF6$mK9%pS2",
            "F7sH*dE4+lW0^gR3-oM6&nC1",
            "H4nK!pL8@eF2#sR5$jC9%mT6",
            "T1vE*gR7+cL4^nK0-fH3&sM8",
            "R8mF@dC5#pE2!lR6$eK1%nS9",
            "L5jH*sW3+gE7^mC4-pR0&lF6",
            "Y2nK!eT8@fL1#cR5$mH9%pS3",
            "D9vF*lE4+sC7^gR0-nK3&mH6",
            "G6pL@dR2#eH5!cF8$lM1%nS4",
            "P3sE*mK7+gL0^eR4-fH6&cW9",
            "E0jT!nF5@lC8#pR2$sK6%mH1",
            "A7vL*eH4+gC1^nR5-mF8&pS0",
            "U4nE@sK7#lF2!pR6$eH9%mC3",
            "I1mT*gL5+cE8^sR0-nF4&pK7",
            "O8jH@eC3#lR6!mF1$pS4%nK9",
            "M5vE*sL2+gR7^eH0-cF3&mT6",
            "C2nK!pF8@lE1#sR4$mH7%cW0",
            "B9jL*eT5+gC3^nR6-fH2&pS8",
            "W6mE@sK1#lF4!pR7$eH0%nC9",
            "V3sT*gL8+eR5^mC2-nF6&pK1",
            "J0nH!cE7@lR4#pF1$sK8%mT5",
            "X7vL*eC3+gR6^nH0-mF4&pS9",
            "K4jE@sT1#lC8!pR5$eH2%nF7",
            "Q1mL*gE6+cR9^sH3-nK0&pF4",
            "Z8vT!eL2@gC5#nR1$mF6%pH9",
            "F5jK*sE8+lC4^gR7-eH0&nF3",
            "H2nT!mL6@cE9#pR3$sK1%eH5",
            "T9vF*gL1+eC4^nR8-mH5&pS0",
            "R6jE@sK3#lF7!pR2$eH6%nC9",
            "L3mT*gE5+cL8^sR1-nF4&pH7",
            "Y0vK!eH2@lC6#pR9$mF3%nS5",
            "D7jL*sE4+gC1^nR5-eH8&pF0",
            "G4nT@mK7#lE2!pR6$sH9%cF3",
            "P1vL*eC5+gR8^nH4-mF7&pS2",
            "E8jK!sT3@lE6#pR0$eH1%nF9",
            "A5mL*gE2+cR7^sH4-nK1&pF6",
            "U2vT!eH8@lC5#pR3$mF0%nS9",
            "I9jL*sE1+gC6^nR4-eH7&pF2",
            "O6nK@mT3#lE8!pR5$sH2%cF0",
            "M3vL*eC7+gR1^nH6-mF4&pS9",
            "C0jT!sE5@lC2#pR8$eH3%nF6",
            "B7mK*gL4+eR1^sH5-nF8&pC0",
            "W4vT!eL7@gC2#nR6$mH9%pS3",
            "V1jE*sK5+lF8^pR3-eH0&nC6",
            "J8mL@gT2#eC5!nR1$sK4%pH7",
            "X5vF*eH8+lC3^pR6-mK0&nS9",
            "K2jT!gL5@eC1#nR8$sF4%pH7",
            "Q9mE*lK3+gC6^eR2-nH5&pF0",
            "Z6vL!sT8@eC4#pR1$mH7%nF3",
            "F3jK*gE5+lC2^sR8-eH1&pF6",
            "H0mT!vL7@gC4#nR3$sK6%eH9"
};

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
        uniform_int_distribution<> fdist(0, static_cast<int>(CHARSET.length() - 1));
        int num = dist(gen);
        int num2 = fdist(gen);
        int num3 = fdist(gen);
        return salts[num] + orginal + CHARSET[num2] + CHARSET[num3];

    }

    string RemoveRandomSalt(string Salted)
    {
        Salted = Salted.substr(0, Salted.length() - 2);
        int saltCount = sizeof(salts) / sizeof(salts[0]);
        for (int i = 0; i < saltCount; i++) {
            size_t pos = 0;
            while ((pos = Salted.find(salts[i])) != string::npos) {
                Salted.erase(pos, salts[i].length());
            }
        }
        return Salted;
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

    void GenerateKey(string password) {
        int seed = 0;
        string seedSource = h.HASHER(password + CHARSET, password.length() / 2);

        for (char& c : seedSource) {
            seed += int(c);
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
        GenerateKey(password);
        ExtendKey(plaintext.length());

        string encrypted = plaintext;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] ^= KEY[i];
        }
        // encrypted = Matrix(encrypted);
        encrypted = Base64Encode(Streamer(Bytemix(encrypted)));
        return encrypted;
    }

    string Decrypt(const string& ciphertext, const string& password) {
        string decodedCipher = Base64Decode(ciphertext);
        string afterStreamer = ReverseStreamer(decodedCipher);
        string afterBytemix = ReverseByteMix(afterStreamer);
        GenerateKey(password);
        ExtendKey(afterBytemix.length());

        // Step 5: XOR to get final plaintext
        string decrypted = afterBytemix;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] ^= KEY[i];
        }

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